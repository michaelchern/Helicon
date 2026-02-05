#pragma once

#include <Codegen/Generator/SlangGenerator.hpp>
#include <Codegen/VariateProxy.h>

namespace EmbeddedShader::BuiltinVariate
{
    inline VariateProxy<ktm::fvec4> position()
    {
        return VariateProxy<ktm::fvec4>{Ast::AST::getPositionOutput()};
    }

    inline VariateProxy<ktm::uvec3> dispatchThreadID()
    {
        return VariateProxy<ktm::uvec3>{Ast::AST::getDispatchThreadIDInput()};
    }
}

namespace EmbeddedShader
{
    using namespace BuiltinVariate;
}