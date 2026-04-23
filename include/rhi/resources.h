/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <rhi/core.h>

namespace helicon
{
    //////////////////////////////////////////////////////////////////////////
    // Heap
    //////////////////////////////////////////////////////////////////////////

    enum class HeapType : uint8_t
    {
        DeviceLocal,
        Upload,
        Readback
    };

    struct HeapDesc
    {
        uint64_t capacity = 0;
        HeapType type;
        std::string debugName;

        constexpr HeapDesc& setCapacity(uint64_t value) { capacity = value; return *this; }
        constexpr HeapDesc& setType(HeapType value) { type = value; return *this; }
        HeapDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
    };

    class IHeap : public IResource
    {
    public:
        virtual const HeapDesc& getDesc() = 0;
    };

    typedef RefCountPtr<IHeap> HeapHandle;

    struct MemoryRequirements
    {
        uint64_t size = 0;
        uint64_t alignment = 0;
    };

    //////////////////////////////////////////////////////////////////////////
    // Texture
    //////////////////////////////////////////////////////////////////////////

    enum class TextureDimension : uint8_t
    {
        Unknown,
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        TextureCube,
        TextureCubeArray,
        Texture2DMS,
        Texture2DMSArray,
        Texture3D
    };

    enum class CpuAccessMode : uint8_t
    {
        None,
        Read,
        Write
    };
    
    enum class ResourceStates : uint32_t
    {
        Unknown                     = 0,
        Common                      = 0x00000001,
        ConstantBuffer              = 0x00000002,
        VertexBuffer                = 0x00000004,
        IndexBuffer                 = 0x00000008,
        IndirectArgument            = 0x00000010,
        ShaderResource              = 0x00000020,
        UnorderedAccess             = 0x00000040,
        RenderTarget                = 0x00000080,
        DepthWrite                  = 0x00000100,
        DepthRead                   = 0x00000200,
        StreamOut                   = 0x00000400,
        CopyDest                    = 0x00000800,
        CopySource                  = 0x00001000,
        ResolveDest                 = 0x00002000,
        ResolveSource               = 0x00004000,
        Present                     = 0x00008000,
        AccelStructRead             = 0x00010000,
        AccelStructWrite            = 0x00020000,
        AccelStructBuildInput       = 0x00040000,
        AccelStructBuildBlas        = 0x00080000,
        ShadingRateSurface          = 0x00100000,
        OpacityMicromapWrite        = 0x00200000,
        OpacityMicromapBuildInput   = 0x00400000,
        ConvertCoopVecMatrixInput   = 0x00800000,
        ConvertCoopVecMatrixOutput  = 0x01000000,
    };

    NVRHI_ENUM_CLASS_FLAG_OPERATORS(ResourceStates)

    typedef uint32_t MipLevel;
    typedef uint32_t ArraySlice;

    // Flags for resources that need to be shared with other graphics APIs or other GPU devices.
    enum class SharedResourceFlags : uint32_t
    {
        None                = 0,

        // D3D11: adds D3D11_RESOURCE_MISC_SHARED
        // D3D12: adds D3D12_HEAP_FLAG_SHARED
        // Vulkan: adds vk::ExternalMemoryImageCreateInfo and vk::ExportMemoryAllocateInfo/vk::ExternalMemoryBufferCreateInfo
        Shared              = 0x01,

        // D3D11: adds (D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE)
        // D3D12, Vulkan: ignored
        Shared_NTHandle     = 0x02,

        // D3D12: adds D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER and D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER
        // D3D11, Vulkan: ignored
        Shared_CrossAdapter = 0x04,
    };

    NVRHI_ENUM_CLASS_FLAG_OPERATORS(SharedResourceFlags)

    struct TextureDesc
    {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t arraySize = 1;
        uint32_t mipLevels = 1;
        uint32_t sampleCount = 1;
        uint32_t sampleQuality = 0;
        Format format = Format::UNKNOWN;
        TextureDimension dimension = TextureDimension::Texture2D;
        std::string debugName;

        bool isShaderResource = true; // Note: isShaderResource is initialized to 'true' for backward compatibility
        bool isRenderTarget = false;
        bool isUAV = false;
        bool isTypeless = false;
        bool isShadingRateSurface = false;

        SharedResourceFlags sharedResourceFlags = SharedResourceFlags::None;

        // Indicates that the texture is created with no backing memory,
        // and memory is bound to the texture later using bindTextureMemory.
        // On DX12, the texture resource is created at the time of memory binding.
        bool isVirtual = false;
        bool isTiled = false;

        Color clearValue;
        bool useClearValue = false;

        ResourceStates initialState = ResourceStates::Unknown;

        // If keepInitialState is true, command lists that use the texture will automatically
        // begin tracking the texture from the initial state and transition it to the initial state 
        // on command list close.
        bool keepInitialState = false;

        constexpr TextureDesc& setWidth(uint32_t value) { width = value; return *this; }
        constexpr TextureDesc& setHeight(uint32_t value) { height = value; return *this; }
        constexpr TextureDesc& setDepth(uint32_t value) { depth = value; return *this; }
        constexpr TextureDesc& setArraySize(uint32_t value) { arraySize = value; return *this; }
        constexpr TextureDesc& setMipLevels(uint32_t value) { mipLevels = value; return *this; }
        constexpr TextureDesc& setSampleCount(uint32_t value) { sampleCount = value; return *this; }
        constexpr TextureDesc& setSampleQuality(uint32_t value) { sampleQuality = value; return *this; }
        constexpr TextureDesc& setFormat(Format value) { format = value; return *this; }
        constexpr TextureDesc& setDimension(TextureDimension value) { dimension = value; return *this; }
                  TextureDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
        constexpr TextureDesc& setIsRenderTarget(bool value) { isRenderTarget = value; return *this; }
        constexpr TextureDesc& setIsUAV(bool value) { isUAV = value; return *this; }
        constexpr TextureDesc& setIsTypeless(bool value) { isTypeless = value; return *this; }
        constexpr TextureDesc& setIsVirtual(bool value) { isVirtual = value; return *this; }
        constexpr TextureDesc& setClearValue(const Color& value) { clearValue = value; useClearValue = true; return *this; }
        constexpr TextureDesc& setUseClearValue(bool value) { useClearValue = value; return *this; }
        constexpr TextureDesc& setInitialState(ResourceStates value) { initialState = value; return *this; }
        constexpr TextureDesc& setKeepInitialState(bool value) { keepInitialState = value; return *this; }
        constexpr TextureDesc& setSharedResourceFlags(SharedResourceFlags value) { sharedResourceFlags = value; return *this; }
        
        // Equivalent to .setInitialState(_initialState).setKeepInitialState(true)
        constexpr TextureDesc& enableAutomaticStateTracking(ResourceStates _initialState)
        {
            initialState = _initialState;
            keepInitialState = true;
            return *this;
        }
    };

    // Describes a 2D or 3D section of a single mip level, single array slice of a texture.
    struct TextureSlice
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
        // -1 means the entire dimension is part of the region
        // resolve() below will translate these values into actual dimensions
        uint32_t width = uint32_t(-1);
        uint32_t height = uint32_t(-1);
        uint32_t depth = uint32_t(-1);

        MipLevel mipLevel = 0;
        ArraySlice arraySlice = 0;

        [[nodiscard]] NVRHI_API TextureSlice resolve(const TextureDesc& desc) const;

        constexpr TextureSlice& setOrigin(uint32_t vx = 0, uint32_t vy = 0, uint32_t vz = 0) { x = vx; y = vy; z = vz; return *this; }
        constexpr TextureSlice& setWidth(uint32_t value) { width = value; return *this; }
        constexpr TextureSlice& setHeight(uint32_t value) { height = value; return *this; }
        constexpr TextureSlice& setDepth(uint32_t value) { depth = value; return *this; }
        constexpr TextureSlice& setSize(uint32_t vx = uint32_t(-1), uint32_t vy = uint32_t(-1), uint32_t vz = uint32_t(-1)) { width = vx; height = vy; depth = vz; return *this; }
        constexpr TextureSlice& setMipLevel(MipLevel level) { mipLevel = level; return *this; }
        constexpr TextureSlice& setArraySlice(ArraySlice slice) { arraySlice = slice; return *this; }
    };

    struct TextureSubresourceSet
    {
        static constexpr MipLevel AllMipLevels = MipLevel(-1);
        static constexpr ArraySlice AllArraySlices = ArraySlice(-1);
        
        MipLevel baseMipLevel = 0;
        MipLevel numMipLevels = 1;
        ArraySlice baseArraySlice = 0;
        ArraySlice numArraySlices = 1;

        TextureSubresourceSet() = default;

        TextureSubresourceSet(MipLevel _baseMipLevel, MipLevel _numMipLevels, ArraySlice _baseArraySlice, ArraySlice _numArraySlices)
            : baseMipLevel(_baseMipLevel)
            , numMipLevels(_numMipLevels)
            , baseArraySlice(_baseArraySlice)
            , numArraySlices(_numArraySlices)
        {
        }

        [[nodiscard]] NVRHI_API TextureSubresourceSet resolve(const TextureDesc& desc, bool singleMipLevel) const;
        [[nodiscard]] NVRHI_API bool isEntireTexture(const TextureDesc& desc) const;

        bool operator ==(const TextureSubresourceSet& other) const
        {
            return baseMipLevel == other.baseMipLevel &&
                numMipLevels == other.numMipLevels &&
                baseArraySlice == other.baseArraySlice &&
                numArraySlices == other.numArraySlices;
        }
        bool operator !=(const TextureSubresourceSet& other) const { return !(*this == other); }

        constexpr TextureSubresourceSet& setBaseMipLevel(MipLevel value) { baseMipLevel = value; return *this; }
        constexpr TextureSubresourceSet& setNumMipLevels(MipLevel value) { numMipLevels = value; return *this; }
        constexpr TextureSubresourceSet& setMipLevels(MipLevel base, MipLevel num) { baseMipLevel = base; numMipLevels = num; return *this; }
        constexpr TextureSubresourceSet& setBaseArraySlice(ArraySlice value) { baseArraySlice = value; return *this; }
        constexpr TextureSubresourceSet& setNumArraySlices(ArraySlice value) { numArraySlices = value; return *this; }
        constexpr TextureSubresourceSet& setArraySlices(ArraySlice base, ArraySlice num) { baseArraySlice = base; numArraySlices = num; return *this; }

        // see the bottom of this file for a specialization of std::hash<TextureSubresourceSet>
    };

    static const TextureSubresourceSet AllSubresources = TextureSubresourceSet(0, TextureSubresourceSet::AllMipLevels, 0, TextureSubresourceSet::AllArraySlices);

    class ITexture : public IResource
    {
    public:
        [[nodiscard]] virtual const TextureDesc& getDesc() const = 0;

        // Similar to getNativeObject, returns a native view for a specified set of subresources. Returns nullptr if unavailable.
        // TODO: on D3D12, the views might become invalid later if the view heap is grown/reallocated, we should do something about that.
        virtual Object getNativeView(ObjectType objectType, Format format = Format::UNKNOWN, TextureSubresourceSet subresources = AllSubresources, TextureDimension dimension = TextureDimension::Unknown, bool isReadOnlyDSV = false) = 0;
    };
    typedef RefCountPtr<ITexture> TextureHandle;

    class IStagingTexture : public IResource
    {
    public:
        [[nodiscard]] virtual const TextureDesc& getDesc() const = 0;
    };
    typedef RefCountPtr<IStagingTexture> StagingTextureHandle;

    struct TiledTextureCoordinate
    {
        uint16_t mipLevel = 0;
        uint16_t arrayLevel = 0;
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
    };

    struct TiledTextureRegion
    {
        uint32_t tilesNum = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 0;
    };

    struct TextureTilesMapping
    {
        TiledTextureCoordinate* tiledTextureCoordinates = nullptr;
        TiledTextureRegion* tiledTextureRegions = nullptr;
        uint64_t* byteOffsets = nullptr;
        uint32_t numTextureRegions = 0;
        IHeap* heap = nullptr;
    };

    struct PackedMipDesc
    {
        uint32_t numStandardMips = 0;
        uint32_t numPackedMips = 0;
        uint32_t numTilesForPackedMips = 0;
        uint32_t startTileIndexInOverallResource = 0;
    };
    
    struct TileShape
    {
        uint32_t widthInTexels = 0;
        uint32_t heightInTexels = 0;
        uint32_t depthInTexels = 0;
    };

    struct SubresourceTiling
    {
        uint32_t widthInTiles = 0;
        uint32_t heightInTiles = 0;
        uint32_t depthInTiles = 0;
        uint32_t startTileIndexInOverallResource = 0;
    };

    enum SamplerFeedbackFormat : uint8_t
    {
        MinMipOpaque = 0x0,
        MipRegionUsedOpaque = 0x1,
    };

    struct SamplerFeedbackTextureDesc
    {
        SamplerFeedbackFormat samplerFeedbackFormat = SamplerFeedbackFormat::MinMipOpaque;
        uint32_t samplerFeedbackMipRegionX = 0;
        uint32_t samplerFeedbackMipRegionY = 0;
        uint32_t samplerFeedbackMipRegionZ = 0;
        ResourceStates initialState = ResourceStates::Unknown;
        bool keepInitialState = false;
    };

    class ISamplerFeedbackTexture : public IResource
    {
    public:
        [[nodiscard]] virtual const SamplerFeedbackTextureDesc& getDesc() const = 0;
        virtual TextureHandle getPairedTexture() = 0;
    };
    typedef RefCountPtr<ISamplerFeedbackTexture> SamplerFeedbackTextureHandle;

    //////////////////////////////////////////////////////////////////////////
    // Buffer
    //////////////////////////////////////////////////////////////////////////

    struct BufferDesc
    {
        uint64_t byteSize = 0;
        uint32_t structStride = 0; // if non-zero it's structured
        uint32_t maxVersions = 0; // only valid and required to be nonzero for volatile buffers on Vulkan
        std::string debugName;
        Format format = Format::UNKNOWN; // for typed buffer views
        bool canHaveUAVs = false;
        bool canHaveTypedViews = false;
        bool canHaveRawViews = false;
        bool isVertexBuffer = false;
        bool isIndexBuffer = false;
        bool isConstantBuffer = false;
        bool isDrawIndirectArgs = false;
        bool isAccelStructBuildInput = false;
        bool isAccelStructStorage = false;
        bool isShaderBindingTable = false;

        // A dynamic/upload buffer whose contents only live in the current command list
        bool isVolatile = false;

        // Indicates that the buffer is created with no backing memory,
        // and memory is bound to the buffer later using bindBufferMemory.
        // On DX12, the buffer resource is created at the time of memory binding.
        bool isVirtual = false;

        ResourceStates initialState = ResourceStates::Common;

        // see TextureDesc::keepInitialState
        bool keepInitialState = false;

        CpuAccessMode cpuAccess = CpuAccessMode::None;

        SharedResourceFlags sharedResourceFlags = SharedResourceFlags::None;

        constexpr BufferDesc& setByteSize(uint64_t value) { byteSize = value; return *this; }
        constexpr BufferDesc& setStructStride(uint32_t value) { structStride = value; return *this; }
        constexpr BufferDesc& setMaxVersions(uint32_t value) { maxVersions = value; return *this; }
                  BufferDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
        constexpr BufferDesc& setFormat(Format value) { format = value; return *this; }
        constexpr BufferDesc& setCanHaveUAVs(bool value) { canHaveUAVs = value; return *this; }
        constexpr BufferDesc& setCanHaveTypedViews(bool value) { canHaveTypedViews = value; return *this; }
        constexpr BufferDesc& setCanHaveRawViews(bool value) { canHaveRawViews = value; return *this; }
        constexpr BufferDesc& setIsVertexBuffer(bool value) { isVertexBuffer = value; return *this; }
        constexpr BufferDesc& setIsIndexBuffer(bool value) { isIndexBuffer = value; return *this; }
        constexpr BufferDesc& setIsConstantBuffer(bool value) { isConstantBuffer = value; return *this; }
        constexpr BufferDesc& setIsDrawIndirectArgs(bool value) { isDrawIndirectArgs = value; return *this; }
        constexpr BufferDesc& setIsAccelStructBuildInput(bool value) { isAccelStructBuildInput = value; return *this; }
        constexpr BufferDesc& setIsAccelStructStorage(bool value) { isAccelStructStorage = value; return *this; }
        constexpr BufferDesc& setIsShaderBindingTable(bool value) { isShaderBindingTable = value; return *this; }
        constexpr BufferDesc& setIsVolatile(bool value) { isVolatile = value; return *this; }
        constexpr BufferDesc& setIsVirtual(bool value) { isVirtual = value; return *this; }
        constexpr BufferDesc& setInitialState(ResourceStates value) { initialState = value; return *this; }
        constexpr BufferDesc& setKeepInitialState(bool value) { keepInitialState = value; return *this; }
        constexpr BufferDesc& setCpuAccess(CpuAccessMode value) { cpuAccess = value; return *this; }

        // Equivalent to .setInitialState(_initialState).setKeepInitialState(true)
        constexpr BufferDesc& enableAutomaticStateTracking(ResourceStates _initialState)
        {
            initialState = _initialState;
            keepInitialState = true;
            return *this;
        }
    };

    struct BufferRange
    {
        uint64_t byteOffset = 0;
        uint64_t byteSize = 0;
        
        BufferRange() = default;

        BufferRange(uint64_t _byteOffset, uint64_t _byteSize)
            : byteOffset(_byteOffset)
            , byteSize(_byteSize)
        { }

        [[nodiscard]] NVRHI_API BufferRange resolve(const BufferDesc& desc) const;
        [[nodiscard]] constexpr bool isEntireBuffer(const BufferDesc& desc) const { return (byteOffset == 0) && (byteSize == ~0ull || byteSize == desc.byteSize); }
        constexpr bool operator== (const BufferRange& other) const { return byteOffset == other.byteOffset && byteSize == other.byteSize; }

        constexpr BufferRange& setByteOffset(uint64_t value) { byteOffset = value; return *this; }
        constexpr BufferRange& setByteSize(uint64_t value) { byteSize = value; return *this; }
    };

    static const BufferRange EntireBuffer = BufferRange(0, ~0ull);

    class IBuffer : public IResource
    {
    public:
        [[nodiscard]] virtual const BufferDesc& getDesc() const = 0;
        [[nodiscard]] virtual GpuVirtualAddress getGpuVirtualAddress() const = 0;
    };

    typedef RefCountPtr<IBuffer> BufferHandle;

    //////////////////////////////////////////////////////////////////////////
}
