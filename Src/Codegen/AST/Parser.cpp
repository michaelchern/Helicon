#include "Parser.hpp"
#include <Codegen/Generator/SlangGenerator.hpp>
#include <utility>

// 这行代码是 C++ 中用于定义和初始化一个线程局部（Thread-local）的静态成员变量。
// 它的核心含义是：“为每一个运行的线程，创建一个独立的、专属的 Parser 对象实例，并用智能指针管理它的生命周期。”
// 目的：实现无锁的线程安全。这样设计允许引擎在多个线程上并发编译 Shader，而不需要使用互斥锁（Mutex）来保护解析器的状态（如 localStatementStack 或 structure），因为不同线程之间的数据完全隔离，互不干扰。

// 总结：这种设计模式叫什么？
//这是一种线程局部单例模式(Thread - local Singleton)。
//对用户来说：Parser 类提供了一组静态方法（如 beginShaderParse, setBindless），看起来像是一个全局工具类。
//在内部实现：每个线程都有一个隐式的上下文（Context），即 currentParser。
//这解决了全局状态（Global State）在多线程环境下的竞争问题。
//同时避免了在每个函数调用中显式传递 ParserInstance 参数的麻烦，保持了 API 的简洁。

thread_local std::unique_ptr<EmbeddedShader::Ast::Parser> EmbeddedShader::Ast::Parser::currentParser = std::unique_ptr<Parser>(new Parser);

//这段代码是 EmbeddedShader 系统中用于批量解析一组相关 Shader（通常用于图形管线，如同时解析顶点着色器和像素着色器）的核心函数。
//它的主要作用是：“分别录制多个 Shader 的逻辑，然后把它们共享的全局定义（结构体、Uniforms）提取出来，拼接到每个 Shader 的头部，形成完整的代码。”

//总结
//这个函数完美展示了嵌入式 DSL 的优势：你可以在 C++ 里写两个独立的 Lambda（一个 VS，一个 PS），它们自然地共享外部定义的 C++ 结构体。这个 parse 函数负责把这种 C++层面的“共享”关系，翻译成 Shader 代码层面的“共享代码（Header）”，让开发者感觉像是在写单一程序一样自然。

std::vector<EmbeddedShader::Ast::ParseOutput> EmbeddedShader::Ast::Parser::parse(const std::vector<ParseParameter>& parameters)
{
    std::vector<ParseOutput> outputs;
    for (const auto& parameter: parameters)
    {
        //输入：parameters 是一个列表，里面可能包含{ VS_Lambda, VertexStage } 和{ PS_Lambda, PixelStage }。
        //动作：循环调用私有的 parse 重载版本（之前分析过的那个会执行 Lambda 的函数）。
        //结果：此时 outputs 里存的代码是不完整的。它只有函数体（例如 void main() { ... }），但缺少结构体定义和全局变量，因此还不能编译。
        //关键点：在循环执行所有 Lambda 的过程中，所有用到的结构体和全局变量都会被注册到同一个 currentParser->structure 中。
        
        outputs.emplace_back(parse(parameter.shaderCode,parameter.stage),parameter.stage);
    }

    //时机：必须在上面的循环结束之后调用。
    //原因：只有执行完所有的 Shader Lambda，解析器才知道一共用到了哪些结构体、哪些全局变量（Uniform Buffer）。
    //动作：getGlobalOutput 会把这些收集到的元数据转换成 Slang / HLSL 代码字符串。
    //例如生成： struct MyData { float4 pos; }; ConstantBuffer<MyData> gData;

    auto globalOutput = Generator::SlangGenerator::getGlobalOutput(currentParser->structure);

    //动作：将第 2 步生成的“头”（全局定义）拼接到第 1 步生成的“身子”（函数体）前面。
    //结果：现在每个 output 都是一份独立、完整的 Shader 代码文件，可以发给编译器了。
    //拼接前：void main() { ... gData ... } (报错：gData 未定义)
    //拼接后：struct ...; ConstantBuffer ...; void main() { ... } (正确)

    for (auto& output: outputs)
        output.output = globalOutput + output.output;

    //作用：清理全局变量的状态。
    //场景：在资源绑定（Bindless 或一般绑定）中，可能会记录哪些资源被读写了。解析完成后重置这些标记，防止影响下一次解析任务。

    for (const auto& global: currentParser->structure.globalStatements)
        global->resetAccessPermissions();

    return outputs;
}

// 总结：完整的工作流
//结合之前的 ComputePipelineObject.h，整个流程是这样的：
//ComputePipelineObject::compile 被调用。
//beginShaderParse(Compute) :
//    设置状态为 Compute Shader。
//    推入栈，开始监听 C++ 操作。
//    执行用户 Lambda(parseHelper::callLambda) :
//    用户写 var = var + 1。
//    因为栈是推入状态，这个操作被转换成 AST 节点，存入 localStatements。
//    endPipelineParse() :
//    发现还在解析状态，于是把 localStatements 里的 AST 转换成字符串（例如 void main() { ... }）。
//    生成全局定义（例如 struct Output { ... }; ）。
//    拼接：struct Output{ ... }; +void main() { ... }。
//    返回完整的代码字符串供编译器使用。

void EmbeddedShader::Ast::Parser::beginShaderParse(ShaderStage stage)
{
    if (currentParser->isInShaderParse)
    {
        // 如果当前正在解析另一个 Shader，先把它保存下来
        //防御性编程：正常情况下，用户应该先结束一个 Shader 再开始下一个。但如果连续调用了两次 begin，这段代码保证前一个 Shader 的内容不会丢失，而是被强制保存并结束。
        //它生成当前 AST 的 Shader 代码字符串，存入 parseOutputs 列表，然后清理栈和状态。

        currentParser->parseOutputs.emplace_back(Generator::SlangGenerator::getShaderOutput(currentParser->structure), currentParser->structure.stage);
        currentParser->reset();
        currentParser->localStatementStack.pop();
        currentParser->isInShaderParse = false;
    }

    currentParser->structure.stage = stage;

    //核心机制：localStatementStack 是 AST 节点生成的关键。
    //当你在 C++ Lambda 里写 a + b 时，重载的操作符会查找这个栈的栈顶，把“加法操作”作为一个 Statement 塞进去。
    //这里把 structure.localStatements 压入栈顶，意味着接下来的所有 C++ 操作都会被记录到当前这个 Shader 的函数体中。

    currentParser->localStatementStack.push(&currentParser->structure.localStatements);
    currentParser->isInShaderParse = true;
}

// 这个函数的作用是结束整个管线的解析，生成最终代码。它不仅处理当前的 Shader，还负责把全局定义（结构体、全局变量）和 Shader 函数体拼装在一起。
std::vector<EmbeddedShader::Ast::ParseOutput> EmbeddedShader::Ast::Parser::endPipelineParse()
{
    if (currentParser->isInShaderParse)
    {
        currentParser->parseOutputs.emplace_back(Generator::SlangGenerator::getShaderOutput(currentParser->structure), currentParser->structure.stage);
        currentParser->reset();
        currentParser->localStatementStack.pop();
        currentParser->isInShaderParse = false;
    }

    // Shader 不仅仅有函数体（Local），还有结构体定义、常量缓冲区定义（Uniforms）等。
    //这些定义通常是在 Shader 函数体之外生成的。getGlobalOutput 负责把这些东西生成为字符串（例如 struct MyData { float x; }; ）。

    auto globalOutput = Generator::SlangGenerator::getGlobalOutput(currentParser->structure);

    //关键步骤：它遍历之前保存的所有 Shader 代码片段（比如这里可能同时有一个 Vertex Shader 和一个 Pixel Shader）。
    //它把 globalOutput（头） 拼接到 output.output（身子） 的前面。
    //为什么？ 因为 Shader 编译器需要先看到结构体定义，才能编译使用该结构体的函数。

    for (auto& output: currentParser->parseOutputs)
        output.output = globalOutput + output.output;

    //重置全局变量的访问权限标记（可能是为了下一次解析做准备，或者处理资源绑定的元数据）。
    for (const auto& global: currentParser->structure.globalStatements)
    {
        global->resetAccessPermissions();
    }

    //使用 swap 将内部成员变量 parseOutputs 的内容转移给局部变量 result，然后清空成员变量。
    //返回最终的 result，其中包含了完整的、可编译的 Shader 源代码字符串。
    std::vector<ParseOutput> result;
    currentParser->parseOutputs.swap(result);
    return result;
}

void EmbeddedShader::Ast::Parser::setBindless(bool bindless)
{
    currentParser->bindless = bindless;
}

bool EmbeddedShader::Ast::Parser::getBindless()
{
    return currentParser->bindless;
}

std::string EmbeddedShader::Ast::Parser::parse(const std::function<void()>& shaderCode, ShaderStage stage)
{
    currentParser->structure.stage = stage;
    currentParser->localStatementStack.push(&currentParser->structure.localStatements);
    shaderCode();
    std::string output = Generator::SlangGenerator::getShaderOutput(currentParser->structure);
    currentParser->reset();
    return output;
}

void EmbeddedShader::Ast::Parser::reset()
{
    structure.localStatements.clear();
    structure.inputStatements.clear();
    structure.outputStatements.clear();
    currentVariateIndex = 0;
    positionOutput.reset();
    dispatchThreadIDInput.reset();
}

std::string EmbeddedShader::Ast::Parser::getUniqueVariateName()
{
    return "var_" + std::to_string(currentParser->currentVariateIndex++);
}

std::string EmbeddedShader::Ast::Parser::getUniqueAggregateTypeName()
{
    return "aggregate_type_" + std::to_string(currentParser->currentAggregateTypeIndex++);
}

std::string EmbeddedShader::Ast::Parser::getUniqueGlobalVariateName()
{
    return "global_var_" + std::to_string(currentParser->currentGlobalVariateIndex++);
}
