#pragma once

#include <nvrhi/core.h>

#include <string>

namespace nvrhi
{
    //////////////////////////////////////////////////////////////////////////
    // Shader
    //////////////////////////////////////////////////////////////////////////

    // Shader type mask. The values match ones used in Vulkan.
    enum class ShaderType : uint16_t
    {
        None            = 0x0000,

        Compute         = 0x0020,

        Vertex          = 0x0001,
        Hull            = 0x0002,
        Domain          = 0x0004,
        Geometry        = 0x0008,
        Pixel           = 0x0010,
        Amplification   = 0x0040,
        Mesh            = 0x0080,
        AllGraphics     = 0x00DF,

        RayGeneration   = 0x0100,
        AnyHit          = 0x0200,
        ClosestHit      = 0x0400,
        Miss            = 0x0800,
        Intersection    = 0x1000,
        Callable        = 0x2000,
        AllRayTracing   = 0x3F00,

        All             = 0x3FFF,
    };

    NVRHI_ENUM_CLASS_FLAG_OPERATORS(ShaderType)

    enum class FastGeometryShaderFlags : uint8_t
    {
        ForceFastGS                      = 0x01,
        UseViewportMask                  = 0x02,
        OffsetTargetIndexByViewportIndex = 0x04,
        StrictApiOrder                   = 0x08
    };

    NVRHI_ENUM_CLASS_FLAG_OPERATORS(FastGeometryShaderFlags)

    struct CustomSemantic
    {
        enum Type
        {
            Undefined = 0,
            XRight = 1,
            ViewportMask = 2
        };

        Type type;
        std::string name;
        
        constexpr CustomSemantic& setType(Type value) { type = value; return *this; }
                  CustomSemantic& setName(const std::string& value) { name = value; return *this; }
    };

    struct ShaderDesc
    {
        ShaderType shaderType = ShaderType::None;
        std::string debugName;
        std::string entryName = "main";

        int hlslExtensionsUAV = -1;

        bool useSpecificShaderExt = false;
        uint32_t numCustomSemantics = 0;
        CustomSemantic* pCustomSemantics = nullptr;

        FastGeometryShaderFlags fastGSFlags = FastGeometryShaderFlags(0);
        uint32_t* pCoordinateSwizzling = nullptr;

        constexpr ShaderDesc& setShaderType(ShaderType value) { shaderType = value; return *this; }
                  ShaderDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
                  ShaderDesc& setEntryName(const std::string& value) { entryName = value; return *this; }
        constexpr ShaderDesc& setHlslExtensionsUAV(int value) { hlslExtensionsUAV = value; return *this; }
        constexpr ShaderDesc& setUseSpecificShaderExt(bool value) { useSpecificShaderExt = value; return *this; }
        constexpr ShaderDesc& setCustomSemantics(uint32_t count, CustomSemantic* data) { numCustomSemantics = count;
            pCustomSemantics = data; return *this; }
        constexpr ShaderDesc& setFastGSFlags(FastGeometryShaderFlags value) { fastGSFlags = value; return *this; }
        constexpr ShaderDesc& setCoordinateSwizzling(uint32_t* value) { pCoordinateSwizzling = value; return *this; }
    };

    struct ShaderSpecialization
    {
        uint32_t constantID = 0;
        union
        {
            uint32_t u = 0;
            int32_t i;
            float f;
        } value;

        static ShaderSpecialization UInt32(uint32_t constantID, uint32_t u)
        {
            ShaderSpecialization s;
            s.constantID = constantID;
            s.value.u = u;
            return s;
        }

        static ShaderSpecialization Int32(uint32_t constantID, int32_t i)
        {
            ShaderSpecialization s;
            s.constantID = constantID;
            s.value.i = i;
            return s;
        }

        static ShaderSpecialization Float(uint32_t constantID, float f)
        {
            ShaderSpecialization s;
            s.constantID = constantID;
            s.value.f = f;
            return s;
        }
    };

    class IShader : public IResource
    {
    public:
        [[nodiscard]] virtual const ShaderDesc& getDesc() const = 0;
        virtual void getBytecode(const void** ppBytecode, size_t* pSize) const = 0;
    };

    typedef RefCountPtr<IShader> ShaderHandle;

    //////////////////////////////////////////////////////////////////////////
    // Shader Library
    //////////////////////////////////////////////////////////////////////////

    class IShaderLibrary : public IResource
    {
    public:
        virtual void getBytecode(const void** ppBytecode, size_t* pSize) const = 0;
        virtual ShaderHandle getShader(const char* entryName, ShaderType shaderType) = 0;
    };

    typedef RefCountPtr<IShaderLibrary> ShaderLibraryHandle;
} // namespace nvrhi
