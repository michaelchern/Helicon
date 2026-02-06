#pragma once

#include <Codegen/AST/AST.hpp>
#include <Codegen/AST/Parser.hpp>
#include <Codegen/ParseHelper.h>
#include <Compiler/ShaderCodeCompiler.h>
#include <functional>

namespace EmbeddedShader
{
    class ComputePipelineObject
    {
    public:
        static ComputePipelineObject compile(auto&& computeShaderCode, ktm::uvec3 numthreads = ktm::uvec3(1), CompilerOption compilerOption = {}, std::source_location sourceLocation = std::source_location::current());
        // 这是一个指向 ShaderCodeCompiler 的智能指针。它保存了实际编译后的着色器信息。这意味着 ComputePipelineObject 拥有编译结果的所有权。
        std::unique_ptr<ShaderCodeCompiler> compute;
    private:
        static std::vector<Ast::ParseOutput> parse(auto&& computeShaderCode);
    };

    // auto&& computeShaderCode: 用户的 C++ Lambda 表达式，包含了着色器的实际逻辑。
    // ktm::uvec3 numthreads: 定义计算着色器的线程组大小（Thread Group Size），默认为 (1, 1, 1)。
    // CompilerOption compilerOption: 编译选项。
    // std::source_location: 用于调试，记录调用编译的位置。
    ComputePipelineObject ComputePipelineObject::compile(auto&& computeShaderCode, ktm::uvec3 numthreads, CompilerOption compilerOption, std::source_location sourceLocation)
    {
        // 设置全局状态: Generator::SlangGenerator::numthreads = numthreads; 将线程组大小传递给代码生成器。
        Generator::SlangGenerator::numthreads = numthreads;
        Ast::Parser::setBindless(false);

        // 初始解析: 调用 parse(computeShaderCode) 将 C++ Lambda 解析为中间表示（AST 或代码字符串）。
        auto outputs = parse(computeShaderCode);

        // 创建编译器对象: result.compute = std::make_unique<ShaderCodeCompiler>(...)。这里指定了 ShaderStage::ComputeShader 和 ShaderLanguage::Slang，说明后端通过 Slang 语言进行编译。
        ComputePipelineObject result;
        result.compute = std::make_unique<ShaderCodeCompiler>(outputs[0].output, ShaderStage::ComputeShader, ShaderLanguage::Slang, compilerOption, sourceLocation);

        if (compilerOption.enableBindless)
        {
            Ast::Parser::setBindless(true);
            outputs = parse(std::forward<decltype(computeShaderCode)>(computeShaderCode));
            result.compute->compile(outputs[0].output, ShaderStage::ComputeShader, ShaderLanguage::Slang, compilerOption);
        }
        return result;
    }

    // 这个函数负责“理解”用户的 C++ 代码。它使用了一种常见的 C++ 嵌入式 DSL（领域特定语言） 技术——模拟执行（Tracing/Mocking）。
    std::vector<Ast::ParseOutput> ComputePipelineObject::parse(auto&& computeShaderCode)
    {
        // 包装 Lambda: 将传入的 computeShaderCode 转换为 std::function。
        // “我创建了一个通用的函数包装器 csFunc。通过自动推导知道了它的参数和返回值类型。同时，我利用‘完美转发’技术，极其吝啬地拿到了用户传入的那个 Lambda——如果是新的就‘抢’过来（移动），如果是旧的就‘抄’一份（拷贝）。”
        auto csFunc = std::function(std::forward<decltype(computeShaderCode)>(computeShaderCode));

        // 开始解析: 调用 Ast::Parser::beginShaderParse(Ast::ShaderStage::Compute)，通知解析器开始记录。
        Ast::Parser::beginShaderParse(Ast::ShaderStage::Compute);

        // 检查返回值: 计算着色器通常没有返回值（void）。代码检查 ParseHelper::hasReturnValue(csFunc)，如果有返回值则抛出逻辑错误。
        if (ParseHelper::hasReturnValue(csFunc))
            throw std::logic_error("Compute shader function doesn't have return value");

        // 参数构造与模拟调用

        // ParseHelper::createParamTuple(csFunc): 创建一组“虚拟”参数。这些参数类型通常是特殊的代理对象（Proxy Objects），它们重载了操作符。
        auto csParams = ParseHelper::createParamTuple(csFunc);

        // ParseHelper::callLambda(csFunc, ...): 实际执行用户的 Lambda 函数。
        ParseHelper::callLambda(csFunc,std::move(csParams));


        // 原理: 当 Lambda 执行时，由于参数是代理对象，Lambda 内部的操作（如 a + b）不会进行 CPU 计算，而是会调用代理对象的操作符重载，从而在后台构建抽象语法树（AST）。
        // 结束解析: Ast::Parser::endPipelineParse() 返回解析生成的 AST 数据（std::vector<Ast::ParseOutput>）。

        return Ast::Parser::endPipelineParse();
    }
}
