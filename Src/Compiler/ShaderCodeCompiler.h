#pragma once

#include <cstdint>
#include <mutex>
#include <source_location>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace EmbeddedShader
{
    enum class ShaderLanguage : uint16_t
    {
        GLSL,
        HLSL,
        DXIL,
        DXBC,
        SpirV,
        Slang,
    };

    std::string enumToString(ShaderLanguage language);

    enum class ShaderStage : uint16_t
    {
        VertexShader = 0,
        FragmentShader = 1,
        ComputeShader = 2,
        // RayGenShader = 3,
        // IntersectShader = 4,
        // AnyHitShader = 5,
        // ClosestHitShader = 6,
        // MissShader = 7,
    };

    std::string enumToString(ShaderStage stage);

    // Helicon 通过全面的反射系统自动提取和管理着色器资源绑定。ShaderCodeModule 中的 ShaderResources 结构捕获有关着色器输入、输出、uniform 缓冲区、纹理、采样器、存储缓冲区和 push constants 的详细信息。
    // 资源绑定按 BindType 分类，包括用于接口匹配的阶段输入和输出、用于参数传递的 uniform 缓冲区和 push constants，以及用于数据访问的各种图像和缓冲区类型。每个绑定都包含元数据，例如描述集编号、绑定位置、类型信息、元素计数和字节偏移量。此反射数据能够在着色器阶段之间自动生成和验证资源布局。

    struct ShaderCodeModule
    {
        struct ShaderResources
        {
            enum BindType
            {
                none = -1,
                pushConstantMembers = 0,
                stageInputs = 1,
                stageOutputs = 2,
                uniformBuffers = 3,
                sampledImages = 4,

                texture,
                sampler,
                rawBuffer,
                storageTexture,
                storageBuffer,
            };
            struct ShaderBindInfo
            {
                uint32_t set = 0;
                uint32_t binding = 0;
                uint32_t location = 0;
                std::string semantic;

                std::string variateName;
                std::string typeName;
                uint64_t elementCount = 0;
                uint32_t typeSize = 0;
                uint64_t byteOffset = 0;

                BindType bindType;
            };

            uint32_t pushConstantSize = 0;
            std::string pushConstantName;

            std::unordered_map<std::string, ShaderBindInfo> bindInfoPool;

            ShaderBindInfo *findShaderBindInfo(const std::string &resourceName)
            {
                auto it = bindInfoPool.find(resourceName);
                if (it != bindInfoPool.end())
                {
                    return &it->second;
                }
                return nullptr;
            }
        } shaderResources;

        ShaderCodeModule() = default;
        ~ShaderCodeModule() = default;

        ShaderCodeModule(std::string shaderCode)
            : shaderCode(std::move(shaderCode))
        {
        }
        ShaderCodeModule(std::vector<uint32_t> shaderCode)
            : shaderCode(std::move(shaderCode))
        {
        }

        ShaderCodeModule(std::string shaderCode,ShaderResources shaderResources)
            : shaderResources(std::move(shaderResources)),shaderCode(std::move(shaderCode))
        {
        }

        ShaderCodeModule(std::vector<uint32_t> shaderCode,ShaderResources shaderResources)
            : shaderResources(std::move(shaderResources)),shaderCode(std::move(shaderCode))
        {
        }

        operator std::string()
        {
            return std::get<std::string>(shaderCode);
        }

        operator std::vector<uint32_t>()
        {
            return std::get<std::vector<uint32_t>>(shaderCode);
        }

        std::variant<std::vector<uint32_t>, std::string> shaderCode;
    };

    struct CompilerOption
    {
        bool compileGLSL = true;
        bool compileHLSL = true;
        bool compileDXIL = true;
        bool compileDXBC = true;
        bool compileSpirV = true;
        bool enableBindless = true;
    };

    struct ShaderCodeCompiler
    {
    public:
        // ShaderCodeCompiler(const std::string &shaderCode, ShaderStage inputStage, ShaderLanguage language = ShaderLanguage::GLSL, const std::source_location &sourceLocation = std::source_location::current());
        // ShaderCodeCompiler(const std::vector<uint32_t> &shaderCode, ShaderStage inputStage, ShaderLanguage language = ShaderLanguage::GLSL, const std::source_location &sourceLocation = std::source_location::current());

        ShaderCodeCompiler(const std::string &shaderCode, ShaderStage inputStage, ShaderLanguage language = ShaderLanguage::GLSL, CompilerOption option = {}, const std::source_location &sourceLocation = std::source_location::current());
        ~ShaderCodeCompiler() = default;

        [[nodiscard]] ShaderCodeModule getShaderCode(ShaderLanguage language, bool bindless = false) const;
        void compile(const std::string &shaderCode, ShaderStage inputStage, ShaderLanguage language = ShaderLanguage::GLSL, CompilerOption option = {}) const;
    private:
        std::string sourceLocationStr;
        std::string stage;
    };
}