#pragma once

#include "../ShaderCodeCompiler.h"
#include <string>
#include <unordered_map>

namespace EmbeddedShader
{
    class HardcodeShaders
    {
        friend class ShaderHardcodeManager;
        static std::unordered_map<std::string, std::unordered_map<std::string, std::variant<ShaderCodeModule::ShaderResources, std::variant<std::vector<uint32_t>, std::string>>> *> hardcodeShaders;
        static std::unordered_map<std::string, std::variant<ShaderCodeModule::ShaderResources, std::variant<std::vector<uint32_t>, std::string>>> hardcodeShadersVertexShader;
        static std::unordered_map<std::string, std::variant<ShaderCodeModule::ShaderResources, std::variant<std::vector<uint32_t>, std::string>>> hardcodeShadersFragmentShader;
        static std::unordered_map<std::string, std::variant<ShaderCodeModule::ShaderResources, std::variant<std::vector<uint32_t>, std::string>>> hardcodeShadersComputeShader;
    };
}