#pragma once

#include <nvrhi/resource.h>
#include <nvrhi/shader.h>

#include <vector>

namespace nvrhi
{
namespace rt { class IAccelStruct; }

    //////////////////////////////////////////////////////////////////////////
    // Binding Layouts
    //////////////////////////////////////////////////////////////////////////

    // identifies the underlying resource type in a binding
    enum class ResourceType : uint8_t
    {
        None,
        Texture_SRV,
        Texture_UAV,
        TypedBuffer_SRV,
        TypedBuffer_UAV,
        StructuredBuffer_SRV,
        StructuredBuffer_UAV,
        RawBuffer_SRV,
        RawBuffer_UAV,
        ConstantBuffer,
        VolatileConstantBuffer,
        Sampler,
        RayTracingAccelStruct,
        PushConstants,
        SamplerFeedbackTexture_UAV,

        Count
    };

    
    struct BindingLayoutItem
    {
        uint32_t slot;

        ResourceType type : 8;
        uint8_t unused : 8;
        // Push constant byte size when (type == PushConstants)
        // Descriptor array size (1 or more) for all other resource types
        // Must be 1 for VolatileConstantBuffer
        uint16_t size : 16;

        bool operator ==(const BindingLayoutItem& b) const
        {
            return slot == b.slot
                && type == b.type
                && size == b.size;
        }
        bool operator !=(const BindingLayoutItem& b) const { return !(*this == b); }

        constexpr BindingLayoutItem& setSlot(uint32_t value) { slot = value; return *this; }
        constexpr BindingLayoutItem& setType(ResourceType value) { type = value; return *this; }
        constexpr BindingLayoutItem& setSize(uint32_t value) { size = uint16_t(value); return *this; }

        uint32_t getArraySize() const { return (type == ResourceType::PushConstants) ? 1 : size; }

        // Helper functions for strongly typed initialization
#define NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(TYPE) /* NOLINT(cppcoreguidelines-macro-usage) */ \
        static BindingLayoutItem TYPE(const uint32_t slot) { \
            BindingLayoutItem result{}; \
            result.slot = slot; \
            result.type = ResourceType::TYPE; \
            result.size = 1; \
            return result; }

        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(Texture_SRV)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(Texture_UAV)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(TypedBuffer_SRV)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(TypedBuffer_UAV)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(StructuredBuffer_SRV)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(StructuredBuffer_UAV)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(RawBuffer_SRV)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(RawBuffer_UAV)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(ConstantBuffer)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(VolatileConstantBuffer)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(Sampler)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(RayTracingAccelStruct)
        NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER(SamplerFeedbackTexture_UAV)

        static BindingLayoutItem PushConstants(const uint32_t slot, const size_t size)
        {
            BindingLayoutItem result{};
            result.slot = slot;
            result.type = ResourceType::PushConstants;
            result.size = uint16_t(size);
            return result;
        }
#undef NVRHI_BINDING_LAYOUT_ITEM_INITIALIZER
    };

    // verify the packing of BindingLayoutItem for good alignment
    static_assert(sizeof(BindingLayoutItem) == 8, "sizeof(BindingLayoutItem) is supposed to be 8 bytes");

    // Describes compile-time settings for HLSL -> SPIR-V register allocation.
    struct VulkanBindingOffsets
    {
        uint32_t shaderResource = 0;
        uint32_t sampler = 128;
        uint32_t constantBuffer = 256;
        uint32_t unorderedAccess = 384;

        constexpr VulkanBindingOffsets& setShaderResourceOffset(uint32_t value) { shaderResource = value; return *this; }
        constexpr VulkanBindingOffsets& setSamplerOffset(uint32_t value) { sampler = value; return *this; }
        constexpr VulkanBindingOffsets& setConstantBufferOffset(uint32_t value) { constantBuffer = value; return *this; }
        constexpr VulkanBindingOffsets& setUnorderedAccessViewOffset(uint32_t value) { unorderedAccess = value; return *this; }
    };

    struct BindingLayoutDesc
    {
        ShaderType visibility = ShaderType::None;

        // On DX11, the registerSpace is ignored, and all bindings are placed in the same space.
        // On DX12, it controls the register space of the bindings.
        // On Vulkan, DXC maps register spaces to descriptor sets by default, so this can be used to
        // determine the descriptor set index for the binding layout.
        // In order to use this behavior, you must set `registerSpaceIsDescriptorSet` to true. See below.
        uint32_t registerSpace = 0;

        // This flag controls the behavior for pipelines that use multiple binding layouts.
        // It must be set to the same value for _all_ of the binding layouts in a pipeline.
        // - When it's set to `false`, the `registerSpace` parameter only affects the DX12 implementation,
        //   and the validation layer will report an error when non-zero `registerSpace` is used with other APIs.
        // - When it's set to `true` the parameter also affects the Vulkan implementation, allowing any
        //   layout to occupy any register space or descriptor set, regardless of their order in the pipeline.
        //   However, a consequence of DXC mapping the descriptor set index to register space is that you may
        //   not have more than one `BindingLayout` using the same `registerSpace` value in the same pipeline.
        // - When it's set to different values for the layouts in a pipeline, the validation layer will report
        //   an error.
        bool registerSpaceIsDescriptorSet = false;

        std::vector<BindingLayoutItem> bindings;
        VulkanBindingOffsets bindingOffsets;

        BindingLayoutDesc& setVisibility(ShaderType value) { visibility = value; return *this; }
        BindingLayoutDesc& setRegisterSpace(uint32_t value) { registerSpace = value; return *this; }
        BindingLayoutDesc& setRegisterSpaceIsDescriptorSet(bool value) { registerSpaceIsDescriptorSet = value; return *this; }
        // Shortcut for .setRegisterSpace(value).setRegisterSpaceIsDescriptorSet(true)
        BindingLayoutDesc& setRegisterSpaceAndDescriptorSet(uint32_t value) { registerSpace = value; registerSpaceIsDescriptorSet = true; return *this; }
        BindingLayoutDesc& addItem(const BindingLayoutItem& value) { bindings.push_back(value); return *this; }
        BindingLayoutDesc& setBindingOffsets(const VulkanBindingOffsets& value) { bindingOffsets = value; return *this; }
    };

    // Bindless layouts allow applications to attach a descriptor table to an unbounded
    // resource array in the shader. The size of the array is not known ahead of time.
    // The same table can be bound to multiple register spaces on DX12, in order to 
    // access different types of resources stored in the table through different arrays.
    // The `registerSpaces` vector specifies which spaces will the table be bound to,
    // with the table type (SRV or UAV) derived from the resource type assigned to each space.
    struct BindlessLayoutDesc
    {

        // BindlessDescriptorType bridges the DX12 and Vulkan in supporting HLSL ResourceDescriptorHeap and SamplerDescriptorHeap
        // For DX12: 
        // - MutableSrvUavCbv, MutableCounters will enable D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED for the Root Signature
        // - MutableSampler will enable D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED for the Root Signature
        // - The BindingLayout will be ignored in terms of setting a descriptor set. DescriptorIndexing should use GetDescriptorIndexInHeap()
        // For Vulkan:
        // - The type corresponds to the SPIR-V bindings which map to ResourceDescriptorHeap and SamplerDescriptorHeap
        // - The shader needs to be compiled with the same descriptor set index as is passed into setState
        // https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#resourcedescriptorheaps-samplerdescriptorheaps
        enum class LayoutType
        {
            Immutable = 0,      // Must use registerSpaces to define a fixed descriptor type

            MutableSrvUavCbv,   // Corresponds to SPIR-V binding -fvk-bind-resource-heap (Counter resources ResourceDescriptorHeap)
                                // Valid descriptor types: Texture_SRV, Texture_UAV, TypedBuffer_SRV, TypedBuffer_UAV,
                                // StructuredBuffer_SRV, StructuredBuffer_UAV, RawBuffer_SRV, RawBuffer_UAV, ConstantBuffer

            MutableCounters,    // Corresponds to SPIR-V binding -fvk-bind-counter-heap (Counter resources accessed via ResourceDescriptorHeap)
                                // Valid descriptor types: StructuredBuffer_UAV

            MutableSampler,     // Corresponds to SPIR-V binding -fvk-bind-sampler-heap (SamplerDescriptorHeap)
                                // Valid descriptor types: Sampler
        };

        ShaderType visibility = ShaderType::None;
        uint32_t firstSlot = 0;
        uint32_t maxCapacity = 0;
        static_vector<BindingLayoutItem, c_MaxBindlessRegisterSpaces> registerSpaces;

        LayoutType layoutType = LayoutType::Immutable;

        BindlessLayoutDesc& setVisibility(ShaderType value) { visibility = value; return *this; }
        BindlessLayoutDesc& setFirstSlot(uint32_t value) { firstSlot = value; return *this; }
        BindlessLayoutDesc& setMaxCapacity(uint32_t value) { maxCapacity = value; return *this; }
        BindlessLayoutDesc& addRegisterSpace(const BindingLayoutItem& value) { registerSpaces.push_back(value); return *this; }
        BindlessLayoutDesc& setLayoutType(LayoutType value) { layoutType = value; return *this; }
    };

    class IBindingLayout : public IResource
    {
    public:
        [[nodiscard]] virtual const BindingLayoutDesc* getDesc() const = 0;           // returns nullptr for bindless layouts
        [[nodiscard]] virtual const BindlessLayoutDesc* getBindlessDesc() const = 0;  // returns nullptr for regular layouts
    };

    typedef RefCountPtr<IBindingLayout> BindingLayoutHandle;

    //////////////////////////////////////////////////////////////////////////
    // Binding Sets
    //////////////////////////////////////////////////////////////////////////

    struct BindingSetItem
    {
        IResource* resourceHandle;

        uint32_t slot;

        // Specifies the index in a binding array.
        // Must be less than the 'size' property of the matching BindingLayoutItem.
        // - DX11/12: Effective binding slot index is calculated as (slot + arrayElement), i.e. arrays are flattened
        // - Vulkan: Descriptor arrays are used.
        // This behavior matches the behavior of HLSL resource array declarations when compiled with DXC.
        uint32_t arrayElement;

        ResourceType type          : 8;
        TextureDimension dimension : 8; // valid for Texture_SRV, Texture_UAV
        Format format              : 8; // valid for Texture_SRV, Texture_UAV, Buffer_SRV, Buffer_UAV
        uint8_t unused             : 8;

        uint32_t unused2; // padding

        union 
        {
            TextureSubresourceSet subresources; // valid for Texture_SRV, Texture_UAV
            BufferRange range; // valid for Buffer_SRV, Buffer_UAV, ConstantBuffer
            uint64_t rawData[2];
        };

        // verify that the `subresources` and `range` have the same size and are covered by `rawData`
        static_assert(sizeof(TextureSubresourceSet) == 16, "sizeof(TextureSubresourceSet) is supposed to be 16 bytes");
        static_assert(sizeof(BufferRange) == 16, "sizeof(BufferRange) is supposed to be 16 bytes");

        bool operator ==(const BindingSetItem& b) const
        {
            return resourceHandle == b.resourceHandle
                && slot == b.slot
                && type == b.type
                && dimension == b.dimension
                && format == b.format
                && rawData[0] == b.rawData[0]
                && rawData[1] == b.rawData[1];
        }

        bool operator !=(const BindingSetItem& b) const
        {
            return !(*this == b);
        }

        // Default constructor that doesn't initialize anything for performance:
        // BindingSetItem's are stored in large statically sized arrays.
        BindingSetItem() { }  // NOLINT(cppcoreguidelines-pro-type-member-init, modernize-use-equals-default)

        // Helper functions for strongly typed initialization

        static BindingSetItem None(uint32_t slot = 0)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::None;
            result.resourceHandle = nullptr;
            result.format = Format::UNKNOWN;
            result.dimension = TextureDimension::Unknown;
            result.rawData[0] = 0;
            result.rawData[1] = 0;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem Texture_SRV(uint32_t slot, ITexture* texture, Format format = Format::UNKNOWN,
            TextureSubresourceSet subresources = AllSubresources, TextureDimension dimension = TextureDimension::Unknown)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::Texture_SRV;
            result.resourceHandle = texture;
            result.format = format;
            result.dimension = dimension;
            result.subresources = subresources;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem Texture_UAV(uint32_t slot, ITexture* texture, Format format = Format::UNKNOWN,
            TextureSubresourceSet subresources = TextureSubresourceSet(0, 1, 0, TextureSubresourceSet::AllArraySlices),
            TextureDimension dimension = TextureDimension::Unknown)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::Texture_UAV;
            result.resourceHandle = texture;
            result.format = format;
            result.dimension = dimension;
            result.subresources = subresources;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem TypedBuffer_SRV(uint32_t slot, IBuffer* buffer, Format format = Format::UNKNOWN, BufferRange range = EntireBuffer)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::TypedBuffer_SRV;
            result.resourceHandle = buffer;
            result.format = format;
            result.dimension = TextureDimension::Unknown;
            result.range = range;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem TypedBuffer_UAV(uint32_t slot, IBuffer* buffer, Format format = Format::UNKNOWN, BufferRange range = EntireBuffer)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::TypedBuffer_UAV;
            result.resourceHandle = buffer;
            result.format = format;
            result.dimension = TextureDimension::Unknown;
            result.range = range;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem ConstantBuffer(uint32_t slot, IBuffer* buffer, BufferRange range = EntireBuffer)
        {
            bool isVolatile = buffer && buffer->getDesc().isVolatile;

            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = isVolatile ? ResourceType::VolatileConstantBuffer : ResourceType::ConstantBuffer;
            result.resourceHandle = buffer;
            result.format = Format::UNKNOWN;
            result.dimension = TextureDimension::Unknown;
            result.range = range;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem Sampler(uint32_t slot, ISampler* sampler)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::Sampler;
            result.resourceHandle = sampler;
            result.format = Format::UNKNOWN;
            result.dimension = TextureDimension::Unknown;
            result.rawData[0] = 0;
            result.rawData[1] = 0;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem RayTracingAccelStruct(uint32_t slot, rt::IAccelStruct* as)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::RayTracingAccelStruct;
            result.resourceHandle = reinterpret_cast<IResource*>(as);
            result.format = Format::UNKNOWN;
            result.dimension = TextureDimension::Unknown;
            result.rawData[0] = 0;
            result.rawData[1] = 0;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem StructuredBuffer_SRV(uint32_t slot, IBuffer* buffer, Format format = Format::UNKNOWN, BufferRange range = EntireBuffer)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::StructuredBuffer_SRV;
            result.resourceHandle = buffer;
            result.format = format;
            result.dimension = TextureDimension::Unknown;
            result.range = range;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem StructuredBuffer_UAV(uint32_t slot, IBuffer* buffer, Format format = Format::UNKNOWN, BufferRange range = EntireBuffer)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::StructuredBuffer_UAV;
            result.resourceHandle = buffer;
            result.format = format;
            result.dimension = TextureDimension::Unknown;
            result.range = range;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem RawBuffer_SRV(uint32_t slot, IBuffer* buffer, BufferRange range = EntireBuffer)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::RawBuffer_SRV;
            result.resourceHandle = buffer;
            result.format = Format::UNKNOWN;
            result.dimension = TextureDimension::Unknown;
            result.range = range;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem RawBuffer_UAV(uint32_t slot, IBuffer* buffer, BufferRange range = EntireBuffer)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::RawBuffer_UAV;
            result.resourceHandle = buffer;
            result.format = Format::UNKNOWN;
            result.dimension = TextureDimension::Unknown;
            result.range = range;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem PushConstants(uint32_t slot, uint32_t byteSize)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::PushConstants;
            result.resourceHandle = nullptr;
            result.format = Format::UNKNOWN;
            result.dimension = TextureDimension::Unknown;
            result.range.byteOffset = 0;
            result.range.byteSize = byteSize;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        static BindingSetItem SamplerFeedbackTexture_UAV(uint32_t slot, ISamplerFeedbackTexture* texture)
        {
            BindingSetItem result;
            result.slot = slot;
            result.arrayElement = 0;
            result.type = ResourceType::SamplerFeedbackTexture_UAV;
            result.resourceHandle = texture;
            result.format = Format::UNKNOWN;
            result.dimension = TextureDimension::Unknown;
            result.subresources = AllSubresources;
            result.unused = 0;
            result.unused2 = 0;
            return result;
        }

        BindingSetItem& setArrayElement(uint32_t value) { arrayElement = value; return *this; }
        BindingSetItem& setFormat(Format value) { format = value; return *this; }
        BindingSetItem& setDimension(TextureDimension value) { dimension = value; return *this; }
        BindingSetItem& setSubresources(TextureSubresourceSet value) { subresources = value; return *this; }
        BindingSetItem& setRange(BufferRange value) { range = value; return *this; }
    };

    // verify the packing of BindingSetItem for good alignment
    static_assert(sizeof(BindingSetItem) == 40, "sizeof(BindingSetItem) is supposed to be 40 bytes");

    // Describes a set of bindings corresponding to one binding layout
    struct BindingSetDesc
    {
        std::vector<BindingSetItem> bindings;
       
        // Enables automatic liveness tracking of this binding set by nvrhi command lists.
        // By setting trackLiveness to false, you take the responsibility of not releasing it 
        // until all rendering commands using the binding set are finished.
        bool trackLiveness = true;

        bool operator ==(const BindingSetDesc& b) const
        {
            if (bindings.size() != b.bindings.size())
                return false;

            for (size_t i = 0; i < bindings.size(); ++i)
            {
                if (bindings[i] != b.bindings[i])
                    return false;
            }

            return true;
        }

        bool operator !=(const BindingSetDesc& b) const
        {
            return !(*this == b);
        }

        BindingSetDesc& addItem(const BindingSetItem& value) { bindings.push_back(value); return *this; }
        BindingSetDesc& setTrackLiveness(bool value) { trackLiveness = value; return *this; }
    };

    class IBindingSet : public IResource
    {
    public:
        [[nodiscard]] virtual const BindingSetDesc* getDesc() const = 0;  // returns nullptr for descriptor tables
        [[nodiscard]] virtual IBindingLayout* getLayout() const = 0;
    };

    typedef RefCountPtr<IBindingSet> BindingSetHandle;

    // Descriptor tables are bare, without extra mappings, state, or liveness tracking.
    // Unlike binding sets, descriptor tables are mutable - moreover, modification is the only way to populate them.
    // They can be grown or shrunk, and they are not tied to any binding layout.
    // All tracking is off, so applications should use descriptor tables with great care.
    // IDescriptorTable is derived from IBindingSet to allow mixing them in the binding arrays.
    class IDescriptorTable : public IBindingSet
    {
    public:
        [[nodiscard]] virtual uint32_t getCapacity() const = 0;
        [[nodiscard]] virtual uint32_t getFirstDescriptorIndexInHeap() const = 0;
    };

    typedef RefCountPtr<IDescriptorTable> DescriptorTableHandle;
} // namespace nvrhi

namespace std
{
    template<> struct hash<nvrhi::BindingSetItem>
    {
        std::size_t operator()(nvrhi::BindingSetItem const& s) const noexcept
        {
            size_t value = 0;
            nvrhi::hash_combine(value, s.resourceHandle);
            nvrhi::hash_combine(value, s.slot);
            nvrhi::hash_combine(value, s.type);
            nvrhi::hash_combine(value, s.dimension);
            nvrhi::hash_combine(value, s.format);
            nvrhi::hash_combine(value, s.rawData[0]);
            nvrhi::hash_combine(value, s.rawData[1]);
            return value;
        }
    };

    template<> struct hash<nvrhi::BindingSetDesc>
    {
        std::size_t operator()(nvrhi::BindingSetDesc const& s) const noexcept
        {
            size_t value = 0;
            for (const auto& item : s.bindings)
                nvrhi::hash_combine(value, item);
            return value;
        }
    };
}
