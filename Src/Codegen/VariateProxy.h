#pragma once

#include "boost/pfr.hpp"
#include <sstream>
#include <regex>
#include <type_traits>
#include <cassert>
#include <slang.h>
#include <unordered_map>
#include <string>
#include <tuple>
#include <ktm/ktm.h>
#include <ktm/type/vec.h>
#include <Codegen/AST/AST.hpp>
#include <Codegen/ParseHelper.h>
#include <Codegen/MathProxy.h>

namespace EmbeddedShader
{
    // 定义了一个概念，要求类型 T 必须支持自加和自减运算。用于限制模板只接受数学类型。
    template<typename T>
    concept is_mathematical = requires(T a)
    {
        a = a + a;
        a = a - a;
    };

    // 第一个：接受普通类型 T，调用 Ast::AST::createValue(value) 将其包装成 AST 的常量节点。
    template<typename T>
    std::shared_ptr<Ast::Value> proxy_wrap(T value)
    {
        return Ast::AST::createValue(value);
    }

    // 接受 VariateProxy<T>，返回其内部持有的 AST 节点。这两个函数用于统一处理“原始值”和“代理对象”。
    template<typename T>
    std::shared_ptr<Ast::Value> proxy_wrap(const VariateProxy<T>& proxy);

    // 类型萃取器。无论传入的是 T 还是 VariateProxy<T>，都提取出底层的类型 T。
    template<typename T>
    struct base_type
    {
        using type = T;
    };
    template<typename T>
    struct base_type<VariateProxy<T>>
    {
        using type = T;
    };
    template<typename T>
    using base_type_t = typename base_type<std::remove_cvref_t<T>>::type;

    // Concept: 约束两个类型底层的 base_type 必须相同。
    template<typename T1,typename T2>
    concept proxy_wrapper = std::same_as<base_type_t<T1>, base_type_t<T2>>;

    // 这是该文件的核心类。它充当类型 Type（如 float, vec3）的代理。当你操作这个类的实例时，它会生成 AST 节点。
    template<typename Type>
    struct VariateProxy
    {
    private:
        // 这是一个辅助结构体，用于处理向量构造函数的参数。
        // 如果参数是普通值，直接返回。
        template<typename T>
        struct VecArgProcessor
        {
            static T valueOrNode(T value)
            {
                return value;
            }
        };

        // 如果参数是 VariateProxy，则取出其内部的 AST node 并设置为只读权限。
        template<typename T>
        struct VecArgProcessor<VariateProxy<T>>
        {
            static std::shared_ptr<Ast::Value> valueOrNode(const VariateProxy<T>& proxy)
            {
                proxy.node->access(Ast::AccessPermissions::ReadOnly);
                return proxy.node;
            }
        };

        // 辅助函数，利用上面的 Processor 统一参数处理。
        auto valueOrNode(auto&& arg)
        {
            return VecArgProcessor<std::remove_cvref_t<decltype(arg)>>::valueOrNode(std::forward<decltype(arg)>(arg));
        }
    public:
        // 声明数组代理、纹理代理、分支语句（if/else）、生成器等为友元，允许它们访问私有成员。
        template<typename T>
        friend struct ArrayProxy;
        template<typename T>
        friend struct Texture2DProxy;
        template<typename OtherType>
        friend struct VariateProxy;
        friend struct GPU_IF_BRANCH;
        friend struct GPU_ELSEIF_BRANCH;
        friend struct GPU_ELSE_BRANCH;

        friend VariateProxy<ktm::fvec4> position();
        friend class RasterizedPipelineObject;
        friend class Generator::SlangGenerator;

        // 导出内部包装的类型。
        using value_type = Type;

        // 构造函数非常复杂，因为它利用 ParseHelper（解析助手）来判断当前变量处于什么上下文中，从而生成不同类型的 AST 节点。
        VariateProxy()
        {
            // 在 CPU 侧分配内存（可能用于模拟或布局计算）。
            value = std::make_unique<Type>();

            // 如果标记为不初始化节点，直接返回。
            if (ParseHelper::notInitNode())
                return;

            // 如果当前是在初始化一个向量的某个分量，创建 access 节点（访问父级向量的分量）。
            if (auto parent = ParseHelper::getVecParent())
            {
                auto name = ParseHelper::getVecComponentName();
                node = Ast::AST::access(parent,name,Ast::AST::createType<Type>());
                return;
            }

            // 如果当前是在初始化一个结构体（Aggregate）的成员，创建 access 节点。
            if (auto parent = ParseHelper::getAggregateParent())
            {
                auto index = ParseHelper::getAggregateMemberIndex();
                auto aggregateType = reinterpret_cast<Ast::AggregateType*>(parent->type.get());
                auto member = aggregateType->members[index];
                node = Ast::AST::access(parent,member->name, member->type);
                return;
            }

            //Uniform,Input,Local Variate
            // 如果是在着色器函数的参数列表中，定义为输入变量 (Input Variate)。
            if (ParseHelper::isInInputParameter())
            {
                node = Ast::AST::defineInputVariate<Type>(ParseHelper::getCurrentInputIndex());
                return;
            }

            // 如果是在着色器函数体内，定义为局部变量 (Local Variate)。
            if (ParseHelper::isInShaderCodeLambda())
            {
                node = Ast::AST::defineLocalVariate<Type>({});
                return;
            }

            // 定义为全局 Uniform 变量。
            node = Ast::AST::defineUniformVariate<Type>();
        }

        // VariateProxy() requires std::is_aggregate_v (结构体专用):

//这是针对结构体类型的特化构造。

        //逻辑与上面类似，但增加了 ParseHelper::beginAggregateParent(node) 和 endAggregateParent()。这告诉系统：“接下来的成员变量构造，都是属于我这个结构体的”。
        VariateProxy() requires std::is_aggregate_v<Type>
        {
            if (ParseHelper::notInitNode())
            {
                value = std::make_unique<Type>();
                return;
            }

            //Uniform,Input,Local Variate
            ParseHelper::beginNotInitNode();
            if (auto parent = ParseHelper::getAggregateParent())
            {
                auto index = ParseHelper::getAggregateMemberIndex();
                auto aggregateType = reinterpret_cast<Ast::AggregateType*>(parent->type.get());
                auto member = aggregateType->members[index];
                node = Ast::AST::access(parent,member->name, member->type);
            }
            else if (ParseHelper::isInInputParameter())
            {
                node = Ast::AST::defineInputVariate<Type>(ParseHelper::getCurrentInputIndex());
            }
            else if (ParseHelper::isInShaderCodeLambda())
            {
                node = Ast::AST::defineLocalVariate<Type>({});
            }
            else
            {
                node = Ast::AST::defineUniformVariate<Type>();
            }
            ParseHelper::endNotInitNode();

            ParseHelper::beginAggregateParent(node);
            value = std::make_unique<Type>();
            ParseHelper::endAggregateParent();
        }

        // VariateProxy(Arg0&& arg0...) (向量构造):

//用于 vec3(x, y, z) 这种形式。

            //调用 Ast::AST::createVecValue，将参数（可能是其他 Proxy 或数值）组合成一个向量构造的 AST 节点。
        template<typename Arg0,typename Arg1,typename... Args>
        explicit VariateProxy(Arg0&& arg0,Arg1&& arg1,Args&&... args) requires ktm::is_vector_v<Type>
        {
            value = std::make_unique<Type>();
            if (ParseHelper::notInitNode())
                return;
            node = Ast::AST::createVecValue<Type>(valueOrNode(std::forward<Arg0>(arg0)),valueOrNode(std::forward<Arg1>(arg1)),valueOrNode(std::forward<Args>(args))...);
        }

        // VariateProxy(const Type& value) (值拷贝构造):

//如果是在 Shader Lambda 内部，定义为带初始值的局部变量。

            //否则定义为 Uniform。
        VariateProxy(const Type& value) requires (!std::is_aggregate_v<Type>)
        {
            if (ParseHelper::notInitNode())
            {
                this->value = std::make_unique<Type>(value);
                return;
            }

            //Local Variate
            if (ParseHelper::isInShaderCodeLambda())
            {
                node = Ast::AST::defineLocalVariate(value);
                return;
            }

            node = Ast::AST::defineUniformVariate<Type>();
        }

        // VariateProxy(const VariateProxy& value) (拷贝构造):

//通常用于 auto a = b;
        //。这会定义一个新的局部变量，并用 b 的表达式初始化它。
        VariateProxy(const VariateProxy& value)
        {
            //Local Variate
            node = Ast::AST::defineLocalVariate(value.node->type, value.node);
        }

        VariateProxy(VariateProxy&& value) = default;

        // ~VariateProxy():

//Ast::AST::addLocalUniversalStatement(node) : 如果这个对象是一个局部变量表达式且引用计数为1（即将销毁），将其作为一个“通用语句”添加到 AST 中。这确保了像 float a = 1.0;
        //这样的声明语句被写入代码，而不仅仅是作为一个悬空的表达式。
        ~VariateProxy()
        {
            if (isNeedUniversalStatementCheck && node && node.use_count() == 1)
                Ast::AST::addLocalUniversalStatement(node);
        }

        //operator->:

//针对结构体：返回 CPU 侧的指针，允许访问成员（从而触发成员的 VariateProxy 构造）。

 //           针对向量：进入“向量分量初始化”模式，允许访问如.x,
 //           .xy 等 swizzle 操作。
        Type* operator->() requires (std::is_aggregate_v<Type> && !ktm::is_vector_v<Type>)
        {
            return value.get();
        }

        auto operator->() requires (ktm::is_vector_v<Type>)
        {
            ParseHelper::beginVecComponentInit(node);
            auto vecComponents = std::make_unique<typename VecProxyMap<Type>::proxy_type>();
            vecComponents->parent = node;
            ParseHelper::endVecComponentInit();
            return std::move(vecComponents);
        }

        //operator= (赋值):

//不进行 CPU 赋值，而是生成 Ast::AST::assign 节点（在着色器中生成 a = b;
        //）。
        VariateProxy& operator=(const VariateProxy& rhs)
        {
            if (this == &rhs)
                return *this;
            if (!std::dynamic_pointer_cast<Ast::Variate>(node))
            {
                node = Ast::AST::defineLocalVariate(node->type, rhs.node);
                return *this;
            }
            Ast::AST::assign(node,rhs.node);
            return *this;
        }

        VariateProxy& operator=(const Type& rhs)
        {
            Ast::AST::assign(node,Ast::AST::createValue(rhs));
            return *this;
        }

        VariateProxy& operator=(Type&& rhs)
        {
            Ast::AST::assign(node,Ast::AST::createValue(std::forward<Type>(rhs)));
            return *this;
        }

        // 自增/自减 (++, --): 生成一元操作符节点。
        const VariateProxy operator++()
        {
            VariateProxy proxy(Ast::AST::unaryOperator(node,"++",true,Ast::AccessPermissions::ReadAndWrite));
            proxy.isNeedUniversalStatementCheck = true;
            return proxy;
        }

        const VariateProxy operator--()
        {
            VariateProxy proxy(Ast::AST::unaryOperator(node,"--",true,Ast::AccessPermissions::ReadAndWrite));
            proxy.isNeedUniversalStatementCheck = true;
            return proxy;
        }

        const VariateProxy operator++(int)
        {
            VariateProxy proxy(Ast::AST::unaryOperator(node,"++",false,Ast::AccessPermissions::ReadAndWrite));
            proxy.isNeedUniversalStatementCheck = true;
            return proxy;
        }

        const VariateProxy operator--(int)
        {
            VariateProxy proxy(Ast::AST::unaryOperator(node,"--",false,Ast::AccessPermissions::ReadAndWrite));
            proxy.isNeedUniversalStatementCheck = true;
            return proxy;
        }

        //算术运算 (+, -, *, /, %): 生成二元操作符节点（Binary Operator）。例如 a + b 生成一个代表加法的 AST 节点。
        VariateProxy operator+(const VariateProxy& rhs) const
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"+"));
        }
        VariateProxy operator-(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"-"));
        }
        VariateProxy operator*(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"*"));
        }
        VariateProxy operator/(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"/"));
        }
        VariateProxy operator%(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"%"));
        }

        // 逻辑与位运算 (!, ||, &&, &, |, ^, <<, >>): 同上，生成对应的逻辑或位运算节点。
        VariateProxy operator!()
        {
            return VariateProxy(Ast::AST::unaryOperator(node,"!"));
        }
        VariateProxy operator||(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"||"));
        }
        VariateProxy operator&&(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"&&"));
        }
        VariateProxy operator~()
        {
            return VariateProxy(Ast::AST::unaryOperator(node,"~"));
        }
        VariateProxy operator&(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"&"));
        }
        VariateProxy operator|(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"|"));
        }
        VariateProxy operator^(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"^"));
        }
        VariateProxy operator<<(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,"<<"));
        }
        VariateProxy operator>>(const VariateProxy& rhs)
        {
            return VariateProxy(Ast::AST::binaryOperator(node,rhs.node,">>"));
        }

        //复合赋值 (+=, -= 等): 复用上面的算术运算重载。
        VariateProxy& operator+=(const VariateProxy& rhs)
        {
            return *this = *this + rhs;
        }
        VariateProxy& operator-=(const VariateProxy& rhs)
        {
            return *this = *this - rhs;
        }

        VariateProxy& operator*=(const VariateProxy& rhs)
        {
            return *this = *this * rhs;
        }

        VariateProxy& operator/=(const VariateProxy& rhs)
        {
            return *this = *this / rhs;
        }

        VariateProxy& operator%=(const VariateProxy& rhs)
        {
            return *this = *this % rhs;
        }

        VariateProxy& operator&=(const VariateProxy& rhs)
        {
            return *this = *this & rhs;
        }

        VariateProxy& operator|=(const VariateProxy& rhs)
        {
            return *this = *this | rhs;
        }

        VariateProxy& operator^=(const VariateProxy& rhs)
        {
            return *this = *this ^ rhs;
        }

        VariateProxy& operator>>=(const VariateProxy& rhs)
        {
            return *this = *this >> rhs;
        }

        VariateProxy& operator<<=(const VariateProxy& rhs)
        {
            return *this = *this << rhs;
        }
        

        //比较运算(>, <, == 等) :

                                //生成二元操作符节点。

                                //注意 : 返回类型是 VariateProxy<bool>，表示比较的结果是一个布尔类型的 AST 表达式。
        VariateProxy<bool> operator>(const VariateProxy& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,rhs.node,">",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator>(const Type& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,Ast::AST::createValue(rhs),">",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator>=(const VariateProxy& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,rhs.node,">=",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator>=(const Type& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,Ast::AST::createValue(rhs),">=",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator<(const VariateProxy& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,rhs.node,"<",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator<(const Type& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,Ast::AST::createValue(rhs),"<",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator<=(const VariateProxy& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,rhs.node,"<=",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator<=(const Type& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,Ast::AST::createValue(rhs),"<=",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator!=(const VariateProxy& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,rhs.node,"!=",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator!=(const Type& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,Ast::AST::createValue(rhs),"!=",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator==(const VariateProxy& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,rhs.node,"==",Ast::AST::createType<bool>()));
        }

        VariateProxy<bool> operator==(const Type& rhs)
        {
            return VariateProxy<bool>(Ast::AST::binaryOperator(node,Ast::AST::createValue(rhs),"==",Ast::AST::createType<bool>()));
        }

        VariateProxy operator+() { return VariateProxy(Ast::AST::unaryOperator(node,"+")); }
        VariateProxy operator-() { return VariateProxy(Ast::AST::unaryOperator(node,"-")); }
        //VariateProxy& operator!() { return *(new VariateProxy()); }
        //VariateProxy& operator~() { return *(new VariateProxy()); }

        //VariateProxy& operator+(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator-(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator*(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator/(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator%(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator||(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator&&(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator&(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator|(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator^(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator<<(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator>>(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator+=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator-=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator*=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator/=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator%=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator&=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator|=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator^=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator>>=(const VariateProxy& rhs) { return *(new VariateProxy()); }
        //VariateProxy& operator<<=(const VariateProxy& rhs) { return *(new VariateProxy()); }

        //VariateProxy<bool>& operator>(const VariateProxy& rhs) { return *(new VariateProxy<bool>(true)); }
        //VariateProxy<bool>& operator>=(const VariateProxy& rhs) { return *(new VariateProxy<bool>(true)); }
        //VariateProxy<bool>& operator<(const VariateProxy& rhs) { return *(new VariateProxy<bool>(true)); }
        //VariateProxy<bool>& operator<=(const VariateProxy& rhs) { return *(new VariateProxy<bool>(true)); }
        //VariateProxy<bool>& operator!=(const VariateProxy& rhs) { return *(new VariateProxy<bool>(true)); }
        //VariateProxy<bool>& operator==(const VariateProxy& rhs) { return *(new VariateProxy<bool>(true)); }

        VariateProxy(std::shared_ptr<Ast::Value> node) : node(std::move(node))//,value(std::make_unique<Type>())
        {
        }

        // CPU 侧的影子值。内部持有的 CPU 侧值（仅当需要时分配）。
        std::unique_ptr<Type> value{};

        // 指向 AST 中对应节点的智能指针。
        std::shared_ptr<Ast::Value> node;
        bool isNeedUniversalStatementCheck = false;
    };

    template<typename Type>
    VariateProxy<Type> operator+(Type a, const VariateProxy<Type>& b)
    {
        return VariateProxy<Type>(a) + b;
    }

    template<typename Type>
    VariateProxy<Type> operator-(Type a, const VariateProxy<Type>& b)
    {
        return VariateProxy<Type>(a) - b;
    }

    template<typename Type>
    VariateProxy<Type> operator*(Type a, const VariateProxy<Type>& b)
    {
        return VariateProxy<Type>(a) * b;
    }

    template<size_t N,typename Type> requires std::is_arithmetic_v<Type>
    VariateProxy<ktm::vec<N,Type>> operator+(const VariateProxy<Type>& a,const VariateProxy<ktm::vec<N,Type>>& b)
    {
        return VariateProxy<ktm::vec<N,Type>>(Ast::AST::binaryOperator(a.node,b.node,"+"));
    }

    template<size_t N,typename Type> requires std::is_arithmetic_v<Type>
    VariateProxy<ktm::vec<N,Type>> operator+(const VariateProxy<ktm::vec<N,Type>>& a,const VariateProxy<Type>& b)
    {
        return b + a;
    }

    template<size_t N,typename Type> requires std::is_arithmetic_v<Type>
    VariateProxy<ktm::vec<N,Type>> operator-(const VariateProxy<Type>& a,const VariateProxy<ktm::vec<N,Type>>& b)
    {
        return VariateProxy<ktm::vec<N,Type>>(Ast::AST::binaryOperator(a.node,b.node,"-"));
    }

    template<size_t N,typename Type> requires std::is_arithmetic_v<Type>
    VariateProxy<ktm::vec<N,Type>> operator-(const VariateProxy<ktm::vec<N,Type>>& a,const VariateProxy<Type>& b)
    {
        return b - a;
    }

    template<size_t N,typename Type> requires std::is_arithmetic_v<Type>
    VariateProxy<ktm::vec<N,Type>> operator*(const VariateProxy<Type>& a,const VariateProxy<ktm::vec<N,Type>>& b)
    {
        return VariateProxy<ktm::vec<N,Type>>(Ast::AST::binaryOperator(a.node,b.node,"*"));
    }

    template<size_t N,typename Type> requires std::is_arithmetic_v<Type>
    VariateProxy<ktm::vec<N,Type>> operator*(const VariateProxy<ktm::vec<N,Type>>& a,const VariateProxy<Type>& b)
    {
        return VariateProxy<ktm::vec<N,Type>>(Ast::AST::binaryOperator(a.node,b.node,"*"));
    }

    template<size_t N,typename Type> requires std::is_arithmetic_v<Type>
    VariateProxy<ktm::vec<N,Type>> operator*=(const VariateProxy<ktm::vec<N,Type>>& a,const VariateProxy<Type>& b)
    {
        return VariateProxy<ktm::vec<N,Type>>(Ast::AST::binaryOperator(a.node,b.node,"*="));
    }

    template<size_t N,typename Type> requires std::is_arithmetic_v<Type>
    VariateProxy<ktm::vec<N,Type>> operator/(const VariateProxy<ktm::vec<N,Type>>& a,const VariateProxy<Type>& b)
    {
        return VariateProxy<ktm::vec<N,Type>>(Ast::AST::binaryOperator(a.node,b.node,"/"));
    }

    template<typename Type>
    struct Texture2DProxyTraits
    {
        static constexpr bool value = false;
    };

    template<typename Type>
    struct Texture2DProxyTraits<Texture2DProxy<Type>>
    {
        static constexpr bool value = true;
    };

    template<typename Type>
    struct ArrayProxy
    {
        using value_type = Type;
        ArrayProxy()
        {
            if (ParseHelper::notInitNode())
                return;

            if (auto parent = ParseHelper::getAggregateParent())
            {
                auto index = ParseHelper::getAggregateMemberIndex();
                auto aggregateType = reinterpret_cast<Ast::AggregateType*>(parent->type.get());
                auto member = aggregateType->members[index];
                node = Ast::AST::access(parent,member->name, member->type);
                return;
            }

            if constexpr (Texture2DProxyTraits<Type>::value)
            {
                auto textureType = std::make_shared<Ast::Texture2DType>();
                textureType->texelType = Ast::AST::createType<Type::value_type>();
                node = Ast::AST::defineUniversalArray(std::move(textureType));
            }
            else node = Ast::AST::defineUniversalArray<Type>();
        }

        template<std::integral IndexType>
        VariateProxy<Type> operator[](IndexType index)
        {
            if constexpr (std::is_aggregate_v<Type>)
            {
                VariateProxy<Type> proxy{Ast::AST::at(node, index)};
                ParseHelper::beginAggregateParent(proxy.node);
                proxy->value = std::make_unique<Type>();
                ParseHelper::endAggregateParent(proxy.node);
                return proxy;
            }
            else return {Ast::AST::at(node, index)};
        }

        template<std::integral IndexType>
        VariateProxy<Type> operator[](const VariateProxy<IndexType>& index)
        {
            if constexpr (std::is_aggregate_v<Type>)
            {
                VariateProxy<Type> proxy{Ast::AST::at(node, index.node)};
                ParseHelper::beginAggregateParent(proxy.node);
                proxy->value = std::make_unique<Type>();
                ParseHelper::endAggregateParent(proxy.node);
                return proxy;
            }
            return {Ast::AST::at(node, index.node)};
        }

        template<std::integral IndexType>
        Type operator[](IndexType index) requires Texture2DProxyTraits<Type>::value
        {
            return {Ast::AST::at(node, index)};
        }

        template<std::integral IndexType>
        Type operator[](const VariateProxy<IndexType>& index) requires Texture2DProxyTraits<Type>::value
        {
            return {Ast::AST::at(node, index.node)};
        }
    private:
        std::shared_ptr<Ast::Value> node;
    };

    struct SamplerProxy
    {
        SamplerProxy()
        {
            if (auto parent = ParseHelper::getAggregateParent())
            {
                auto index = ParseHelper::getAggregateMemberIndex();
                auto aggregateType = reinterpret_cast<Ast::AggregateType*>(parent->type.get());
                auto member = aggregateType->members[index];
                node = Ast::AST::access(parent,member->name, member->type);
                return;
            }
        }

        void init(std::string name)
        {
            if (node)
                return;
            auto type = std::make_shared<Ast::SamplerType>();
            type->name = std::move(name);
            node = Ast::AST::defineUniformVariate(std::move(type));
        }
        std::shared_ptr<Ast::Value> node;
    };

    template<typename Type>
    struct Texture2DProxy
    {
        using value_type = Type;
        Texture2DProxy()
        {
            if (ParseHelper::notInitNode())
                return;

            if (auto parent = ParseHelper::getAggregateParent())
            {
                auto index = ParseHelper::getAggregateMemberIndex();
                auto aggregateType = reinterpret_cast<Ast::AggregateType*>(parent->type.get());
                auto member = aggregateType->members[index];
                node = Ast::AST::access(parent,member->name, member->type);
                return;
            }

            node = Ast::AST::defineUniversalTexture2D<Type>();
        }

        // Texture2DProxy(SamplerProxy&& sampler)
        // {
        // 	isHybrid = true;
        // 	if (ParseHelper::notInitNode())
        // 		return;
        //
        // 	if (auto parent = ParseHelper::getAggregateParent())
        // 	{
        // 		auto index = ParseHelper::getAggregateMemberIndex();
        // 		auto aggregateType = reinterpret_cast<Ast::AggregateType*>(parent->type.get());
        // 		auto member = aggregateType->members[index];
        // 		std::reinterpret_pointer_cast<Ast::Texture2DType>(member->type)->name = "Sampler2D";
        // 		node = Ast::AST::access(parent,member->name, member->type);
        // 		return;
        // 	}
        //
        // 	node = Ast::AST::defineUniversalTexture2D<Type>();
        // 	std::reinterpret_pointer_cast<Ast::Texture2DType>(node->type)->name = "Sampler2D";
        // }

        template<std::integral IndexType>
        VariateProxy<Type> operator[](ktm::vec<2,IndexType> index)
        {
            if constexpr (std::is_aggregate_v<Type>)
            {
                VariateProxy<Type> proxy{Ast::AST::at(node, index)};
                ParseHelper::beginAggregateParent(proxy.node);
                proxy->value = std::make_unique<Type>();
                ParseHelper::endAggregateParent(proxy.node);
                return proxy;
            }
            else return {Ast::AST::at(node, index)};
        }

        template<std::integral IndexType>
        VariateProxy<Type> operator[](VariateProxy<ktm::vec<2,IndexType>> index)
        {
            if constexpr (std::is_aggregate_v<Type>)
            {
                VariateProxy<Type> proxy{Ast::AST::at(node, index.node)};
                ParseHelper::beginAggregateParent(proxy.node);
                proxy->value = std::make_unique<Type>();
                ParseHelper::endAggregateParent(proxy.node);
                return proxy;
            }
            return {Ast::AST::at(node, index.node)};
        }

        VariateProxy<Type> sample(SamplerProxy& sampler,const VariateProxy<ktm::fvec2>& location)
        {
            //Cannot sample texture with sampler in hybrid mode, use sample(location) instead.
            assert(!isHybrid);
            sampler.init("SamplerState");
            sampler.node->access(Ast::AccessPermissions::ReadOnly);
            node->access(Ast::AccessPermissions::ReadOnly);
            auto func = Ast::AST::callFunc("Sample",Ast::AST::createType<Type>(),{sampler.node,location.node});
            return {Ast::AST::access(node,func->parse(), func->type)};
        }

        // VariateProxy<Type> sample(const VariateProxy<ktm::fvec2>& location)
        // {
        // 	//Cannot sample texture without sampler in non-hybrid mode, use sample(sampler, location) instead.
        // 	assert(isHybrid);
        // 	node->access(Ast::AccessPermissions::ReadOnly);
        // 	auto func = Ast::AST::callFunc("Sample",Ast::AST::createType<Type>(),{location.node});
        // 	return {Ast::AST::access(node,func->parse(), func->type)};
        // }

        Texture2DProxy(std::shared_ptr<Ast::Value> node) : node(std::move(node)) {}

        std::shared_ptr<Ast::Value> node;
        bool isHybrid = false;
    };

    template<typename T>
    std::shared_ptr<Ast::Value> proxy_wrap(const VariateProxy<T>& proxy)
    {
        return proxy.node;
    }
}
