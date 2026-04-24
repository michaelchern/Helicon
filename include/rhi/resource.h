// ============================================================================
// @file        ResourceTypes.h
// @brief       Core resource descriptors and interfaces for the Helicon RHI (Helicon RHI 核心资源描述符与接口)
// @project     Helicon
// @author      Michael Chern <1216866818@qq.com>
// @date        2026-04-24
//
// Copyright (c) 2025-2026 Michael Chern. All rights reserved.
// ============================================================================

#pragma once

#include <nvrhi/abi.h>
#include <nvrhi/format.h>

#include <string>
#include <vector>

namespace helicon
{
    //////////////////////////////////////////////////////////////////////////
    // Heap
    //////////////////////////////////////////////////////////////////////////

    /**
     * @enum HeapType
     * @brief Type of GPU memory heap.
     *
     * @par 中文说明：
     *      GPU 内存堆的类型：DeviceLocal 表示显存，Upload 用于 CPU 到 GPU 数据传输，
     *      Readback 用于 GPU 到 CPU 的回读。
     */
    enum class HeapType : uint8_t
    {
        DeviceLocal,
        Upload,
        Readback
    };

    /**
     * @struct HeapDesc
     * @brief Description for creating a heap.
     *
     * @par 中文说明：
     *      创建堆的描述结构，包含容量、类型和调试名称。
     */
    struct HeapDesc
    {
        uint64_t capacity = 0;           ///< Capacity in bytes.
        HeapType type;                   ///< Heap type.
        std::string debugName;           ///< Debug name for the heap.

        constexpr HeapDesc& setCapacity(uint64_t value) { capacity = value; return *this; }
        constexpr HeapDesc& setType(HeapType value) { type = value; return *this; }
                  HeapDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
    };

    /**
     * @class IHeap
     * @brief Interface for a GPU heap resource.
     *
     * @par 中文说明：
     *      GPU 堆的接口，继承自 IResource，可获取堆描述信息。
     */
    class IHeap : public IResource
    {
    public:
        virtual const HeapDesc& getDesc() = 0;
    };

    typedef RefCountPtr<IHeap> HeapHandle;

    /**
     * @struct MemoryRequirements
     * @brief Memory size and alignment requirements for a resource.
     *
     * @par 中文说明：
     *      资源的内存大小和对齐要求。
     */
    struct MemoryRequirements
    {
        uint64_t size = 0;      ///< Required size in bytes.
        uint64_t alignment = 0; ///< Required alignment in bytes.
    };

    //////////////////////////////////////////////////////////////////////////
    // Texture
    //////////////////////////////////////////////////////////////////////////

    /**
     * @enum TextureDimension
     * @brief Dimensionality of a texture resource.
     *
     * @par 中文说明：
     *      纹理的维度类型，如 1D、2D、3D、立方体等。
     */
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

    /**
     * @enum CpuAccessMode
     * @brief Specifies CPU access level for a resource.
     *
     * @par 中文说明：
     *      资源允许的 CPU 访问方式：无、读、写。
     */
    enum class CpuAccessMode : uint8_t
    {
        None,
        Read,
        Write
    };

    /**
     * @enum ResourceStates
     * @brief Resource state flags used for pipeline state tracking.
     *
     * @par 中文说明：
     *      资源状态标志，用于管线状态跟踪，如着色器资源、渲染目标、深度读写等。
     *      支持按位组合。
     */
    enum class ResourceStates : uint32_t
    {
        Unknown                    = 0,
        Common                     = 0x00000001,
        ConstantBuffer             = 0x00000002,
        VertexBuffer               = 0x00000004,
        IndexBuffer                = 0x00000008,
        IndirectArgument           = 0x00000010,
        ShaderResource             = 0x00000020,
        UnorderedAccess            = 0x00000040,
        RenderTarget               = 0x00000080,
        DepthWrite                 = 0x00000100,
        DepthRead                  = 0x00000200,
        StreamOut                  = 0x00000400,
        CopyDest                   = 0x00000800,
        CopySource                 = 0x00001000,
        ResolveDest                = 0x00002000,
        ResolveSource              = 0x00004000,
        Present                    = 0x00008000,
        AccelStructRead            = 0x00010000,
        AccelStructWrite           = 0x00020000,
        AccelStructBuildInput      = 0x00040000,
        AccelStructBuildBlas       = 0x00080000,
        ShadingRateSurface         = 0x00100000,
        OpacityMicromapWrite       = 0x00200000,
        OpacityMicromapBuildInput  = 0x00400000,
        ConvertCoopVecMatrixInput  = 0x00800000,
        ConvertCoopVecMatrixOutput = 0x01000000,
    };

    NVRHI_ENUM_CLASS_FLAG_OPERATORS(ResourceStates)

    typedef uint32_t MipLevel;
    typedef uint32_t ArraySlice;

    /**
     * @enum SharedResourceFlags
     * @brief Flags for resources that need to be shared with other graphics APIs or other GPU devices.
     *
     * @par 中文说明：
     *      资源共享标志，用于跨 API 或多 GPU 设备间的共享。
     *      - Shared: 基本共享。
     *      - Shared_NTHandle: 使用 NT 句柄共享（D3D11 特有）。
     *      - Shared_CrossAdapter: 跨适配器共享（D3D12 特有）。
     */
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

    /**
     * @struct TextureDesc
     * @brief Descriptor for creating a texture resource.
     *
     * Provides detailed configuration for textures, including dimensions, format,
     * usage flags, initial state, and automatic state tracking.
     *
     * @par 中文说明：
     *      纹理创建描述结构，包含尺寸、格式、用途标志、初始状态等。
     *      提供 setter 链式调用和自动状态跟踪开关。
     */
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

        bool isShaderResource = true; ///< True if usable as a shader resource (default true for backward compatibility).
        bool isRenderTarget = false;
        bool isUAV = false;
        bool isTypeless = false;
        bool isShadingRateSurface = false;

        SharedResourceFlags sharedResourceFlags = SharedResourceFlags::None;

        /**
         * @brief If true, the texture is created without backing memory; memory is bound later via bindTextureMemory.
         *        On DX12, the texture resource is created only when memory is bound.
         * @par 中文说明：虚拟纹理标志，内存稍后绑定。
         */
        bool isVirtual = false;
        bool isTiled = false;

        Color clearValue;
        bool useClearValue = false;

        ResourceStates initialState = ResourceStates::Unknown;

        /**
         * @brief If true, command lists automatically track initial state and transition back to it on close.
         * @par 中文说明：
         *      若为 true，命令列表会自动从 initial 状态开始追踪纹理状态，
         *      并在命令列表关闭时转换回该状态。
         */
        bool keepInitialState = false;

        // Setter methods (fluent interface)
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

        /**
         * @brief Convenience function to set initial state and enable automatic state tracking.
         * @param _initialState The initial resource state.
         * @return Reference to this descriptor.
         *
         * @par 中文说明：
         *      等价于调用 .setInitialState(_initialState).setKeepInitialState(true)。
         */
        constexpr TextureDesc& enableAutomaticStateTracking(ResourceStates _initialState)
        {
            initialState = _initialState;
            keepInitialState = true;
            return *this;
        }
    };

    /**
     * @struct TextureSlice
     * @brief Describes a 2D or 3D sub-region within a single mip level and array slice.
     *
     * @par 中文说明：
     *      描述单个 mip 级别、单个数组切片内的一个 2D 或 3D 子区域。
     *      宽度、高度、深度默认值为 -1，表示使用纹理的整个维度（由 resolve() 解析为实际大小）。
     */
    struct TextureSlice
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
        /**
         * @brief Width of the slice; -1 means the entire dimension of the texture.
         *        resolve() will translate -1 into actual dimensions.
         */
        uint32_t width = uint32_t(-1);
        /**
         * @brief Height of the slice; -1 means the entire dimension of the texture.
         */
        uint32_t height = uint32_t(-1);
        /**
         * @brief Depth of the slice; -1 means the entire dimension of the texture.
         */
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

    /**
     * @struct TextureSubresourceSet
     * @brief Specifies a range of mip levels and array slices for texture views.
     *
     * Supports sentinel values AllMipLevels and AllArraySlices to denote the entire resource.
     *
     * @par 中文说明：
     *      纹理子资源范围，用于视图创建。提供 AllMipLevels 和 AllArraySlices 哨兵值，
     *      表示包含所有 mip 级别或所有数组切片。resolve() 方法将哨兵值转换为实际范围。
     */
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

        /**
         * @brief Resolves sentinel values to actual subresource counts based on the descriptor.
         * @param desc Texture descriptor.
         * @param singleMipLevel If true, treat the mip range as a single mip for views that require it.
         * @return Resolved TextureSubresourceSet.
         */
        [[nodiscard]] NVRHI_API TextureSubresourceSet resolve(const TextureDesc& desc, bool singleMipLevel) const;

        /**
         * @brief Checks if this subresource set covers the entire texture.
         */
        [[nodiscard]] NVRHI_API bool isEntireTexture(const TextureDesc& desc) const;

        bool operator ==(const TextureSubresourceSet& other) const
        {
            return baseMipLevel   == other.baseMipLevel &&
                   numMipLevels   == other.numMipLevels &&
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
    };

    /**
     * @brief Predefined sentinel for all subresources.
     */
    static const TextureSubresourceSet AllSubresources = TextureSubresourceSet(0, TextureSubresourceSet::AllMipLevels, 0, TextureSubresourceSet::AllArraySlices);

    /**
     * @class ITexture
     * @brief Interface for a texture resource.
     *
     * @par 中文说明：
     *      纹理资源接口，提供描述信息查询以及获取原生视图的能力。
     */
    class ITexture : public IResource
    {
    public:
        [[nodiscard]] virtual const TextureDesc& getDesc() const = 0;

        /**
         * @brief Returns a native view for the specified subresource range and format.
         * @param objectType Type of the native object requested.
         * @param format Optional override format (UNKNOWN = use texture's format).
         * @param subresources Subresource range (default AllSubresources).
         * @param dimension Optional override dimension (Unknown = use texture's dimension).
         * @param isReadOnlyDSV If true, request a read-only depth-stencil view.
         * @return Native object handle, or nullptr if unavailable.
         *
         * @par 中文说明：
         *      获取指定子资源范围、格式、维度的原生视图对象。如果无法提供则返回 nullptr。
         *      注意：在 D3D12 中，若视图堆增长或重分配，返回的视图可能失效。
         */
        virtual Object getNativeView(ObjectType objectType, Format format = Format::UNKNOWN, TextureSubresourceSet subresources = AllSubresources, TextureDimension dimension = TextureDimension::Unknown, bool isReadOnlyDSV = false) = 0;
    };
    typedef RefCountPtr<ITexture> TextureHandle;

    /**
     * @class IStagingTexture
     * @brief Interface for a staging texture used for CPU read/write.
     *
     * @par 中文说明：
     *      用于 CPU 读写操作的临时纹理，仅作为数据传输中介。
     */
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

    /**
     * @enum SamplerFeedbackFormat
     * @brief Format for sampler feedback data (used in texture space shading).
     *
     * @par 中文说明：
     *      采样器反馈数据的格式，用于纹理空间着色等特性。
     */
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

    /**
     * @struct BufferDesc
     * @brief Descriptor for creating a buffer resource.
     *
     * @par 中文说明：
     *      缓冲区创建描述结构，包含字节大小、步幅、用途标志、初始状态等。
     *      支持易失性缓冲区、虚拟缓冲区、自动状态跟踪等特性。
     */
    struct BufferDesc
    {
        uint64_t byteSize = 0;
        uint32_t structStride = 0;        ///< If non-zero, the buffer is a structured buffer.
        uint32_t maxVersions = 0;         ///< Non-zero only for volatile buffers on Vulkan.
        std::string debugName;
        Format format = Format::UNKNOWN;  ///< Format for typed buffer views.
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

        bool isVolatile = false;          ///< Dynamic/upload buffer whose contents only live in the current command list.
        bool isVirtual = false;           ///< Created without backing memory; memory is bound later.

        ResourceStates initialState = ResourceStates::Common;
        bool keepInitialState = false;    ///< See TextureDesc::keepInitialState.

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

        /**
         * @brief Equivalent to .setInitialState(_initialState).setKeepInitialState(true)
         */
        constexpr BufferDesc& enableAutomaticStateTracking(ResourceStates _initialState)
        {
            initialState = _initialState;
            keepInitialState = true;
            return *this;
        }
    };

    /**
     * @struct BufferRange
     * @brief Specifies a byte range within a buffer.
     *
     * @par 中文说明：
     *      缓冲区的字节范围，用于视图或拷贝操作。
     */
    struct BufferRange
    {
        uint64_t byteOffset = 0;
        uint64_t byteSize = 0;

        BufferRange() = default;

        BufferRange(uint64_t _byteOffset, uint64_t _byteSize)
            : byteOffset(_byteOffset)
            , byteSize(_byteSize)
        {
        }

        [[nodiscard]] NVRHI_API BufferRange resolve(const BufferDesc& desc) const;
        [[nodiscard]] constexpr bool isEntireBuffer(const BufferDesc& desc) const { return (byteOffset == 0) && (byteSize == ~0ull || byteSize == desc.byteSize); }
        constexpr bool operator== (const BufferRange& other) const { return byteOffset == other.byteOffset && byteSize == other.byteSize; }

        constexpr BufferRange& setByteOffset(uint64_t value) { byteOffset = value; return *this; }
        constexpr BufferRange& setByteSize(uint64_t value) { byteSize = value; return *this; }
    };

    /**
     * @brief Sentinel BufferRange representing the entire buffer.
     */
    static const BufferRange EntireBuffer = BufferRange(0, ~0ull);

    /**
     * @class IBuffer
     * @brief Interface for a GPU buffer resource.
     *
     * @par 中文说明：
     *      GPU 缓冲区资源接口，可获取描述信息和 GPU 虚拟地址。
     */
    class IBuffer : public IResource
    {
    public:
        [[nodiscard]] virtual const BufferDesc& getDesc() const = 0;
        [[nodiscard]] virtual GpuVirtualAddress getGpuVirtualAddress() const = 0;
    };

    typedef RefCountPtr<IBuffer> BufferHandle;

    //////////////////////////////////////////////////////////////////////////
    // Sampler
    //////////////////////////////////////////////////////////////////////////

    /**
     * @enum SamplerAddressMode
     * @brief Texture addressing mode.
     *
     * Provides both D3D and Vulkan naming conventions.
     *
     * @par 中文说明：
     *      纹理寻址模式，如钳位、重复、镜像等。同时提供 D3D 和 Vulkan 命名。
     */
    enum class SamplerAddressMode : uint8_t
    {
        // D3D names
        Clamp,
        Wrap,
        Border,
        Mirror,
        MirrorOnce,

        // Vulkan names
        ClampToEdge = Clamp,
        Repeat = Wrap,
        ClampToBorder = Border,
        MirroredRepeat = Mirror,
        MirrorClampToEdge = MirrorOnce
    };

    /**
     * @enum SamplerReductionType
     * @brief Sampler reduction mode (e.g., min/max filtering).
     *
     * @par 中文说明：
     *      采样器缩减类型，如标准、比较、最小值、最大值。
     */
    enum class SamplerReductionType : uint8_t
    {
        Standard,
        Comparison,
        Minimum,
        Maximum
    };

    /**
     * @struct SamplerDesc
     * @brief Description for creating a sampler resource.
     *
     * @par 中文说明：
     *      采样器创建描述结构，包含过滤方式、寻址模式、各向异性等参数。
     */
    struct SamplerDesc
    {
        Color borderColor = 1.f;
        float maxAnisotropy = 1.f;
        float mipBias = 0.f;

        bool minFilter = true;
        bool magFilter = true;
        bool mipFilter = true;
        SamplerAddressMode addressU = SamplerAddressMode::Clamp;
        SamplerAddressMode addressV = SamplerAddressMode::Clamp;
        SamplerAddressMode addressW = SamplerAddressMode::Clamp;
        SamplerReductionType reductionType = SamplerReductionType::Standard;

        SamplerDesc& setBorderColor(const Color& color) { borderColor = color; return *this; }
        SamplerDesc& setMaxAnisotropy(float value) { maxAnisotropy = value; return *this; }
        SamplerDesc& setMipBias(float value) { mipBias = value; return *this; }
        SamplerDesc& setMinFilter(bool enable) { minFilter = enable; return *this; }
        SamplerDesc& setMagFilter(bool enable) { magFilter = enable; return *this; }
        SamplerDesc& setMipFilter(bool enable) { mipFilter = enable; return *this; }
        SamplerDesc& setAllFilters(bool enable) { minFilter = magFilter = mipFilter = enable; return *this; }
        SamplerDesc& setAddressU(SamplerAddressMode mode) { addressU = mode; return *this; }
        SamplerDesc& setAddressV(SamplerAddressMode mode) { addressV = mode; return *this; }
        SamplerDesc& setAddressW(SamplerAddressMode mode) { addressW = mode; return *this; }
        SamplerDesc& setAllAddressModes(SamplerAddressMode mode) { addressU = addressV = addressW = mode; return *this; }
        SamplerDesc& setReductionType(SamplerReductionType type) { reductionType = type; return *this; }
    };

    /**
     * @class ISampler
     * @brief Interface for a sampler resource.
     */
    class ISampler : public IResource
    {
    public:
        [[nodiscard]] virtual const SamplerDesc& getDesc() const = 0;
    };

    typedef RefCountPtr<ISampler> SamplerHandle;

    //////////////////////////////////////////////////////////////////////////
    // Framebuffer
    //////////////////////////////////////////////////////////////////////////

    /**
     * @struct FramebufferAttachment
     * @brief Describes a texture attachment in a framebuffer.
     *
     * @par 中文说明：
     *      帧缓冲附件的描述，包含纹理指针、子资源范围、格式、只读标志等。
     */
    struct FramebufferAttachment
    {
        ITexture* texture = nullptr;
        TextureSubresourceSet subresources = TextureSubresourceSet(0, 1, 0, 1);
        Format format = Format::UNKNOWN;
        bool isReadOnly = false;

        constexpr FramebufferAttachment& setTexture(ITexture* t) { texture = t; return *this; }
        constexpr FramebufferAttachment& setSubresources(TextureSubresourceSet value) { subresources = value; return *this; }
        constexpr FramebufferAttachment& setArraySlice(ArraySlice index) { subresources.baseArraySlice = index; subresources.numArraySlices = 1; return *this; }
        constexpr FramebufferAttachment& setArraySliceRange(ArraySlice index, ArraySlice count) { subresources.baseArraySlice = index; subresources.numArraySlices = count; return *this; }
        constexpr FramebufferAttachment& setMipLevel(MipLevel level) { subresources.baseMipLevel = level; subresources.numMipLevels = 1; return *this; }
        constexpr FramebufferAttachment& setFormat(Format f) { format = f; return *this; }
        constexpr FramebufferAttachment& setReadOnly(bool ro) { isReadOnly = ro; return *this; }

        [[nodiscard]] bool valid() const { return texture != nullptr; }
    };

    /**
     * @struct FramebufferDesc
     * @brief Descriptor for creating a framebuffer.
     *
     * @par 中文说明：
     *      帧缓冲创建描述结构，包含颜色附件数组、深度附件、着色率附件。
     */
    struct FramebufferDesc
    {
        static_vector<FramebufferAttachment, c_MaxRenderTargets> colorAttachments;
        FramebufferAttachment depthAttachment;
        FramebufferAttachment shadingRateAttachment;

        FramebufferDesc& addColorAttachment(const FramebufferAttachment& a) { colorAttachments.push_back(a); return *this; }
        FramebufferDesc& addColorAttachment(ITexture* texture) { colorAttachments.push_back(FramebufferAttachment().setTexture(texture)); return *this; }
        FramebufferDesc& addColorAttachment(ITexture* texture, TextureSubresourceSet subresources) { colorAttachments.push_back(FramebufferAttachment().setTexture(texture).setSubresources(subresources)); return *this; }
        FramebufferDesc& setDepthAttachment(const FramebufferAttachment& d) { depthAttachment = d; return *this; }
        FramebufferDesc& setDepthAttachment(ITexture* texture) { depthAttachment = FramebufferAttachment().setTexture(texture); return *this; }
        FramebufferDesc& setDepthAttachment(ITexture* texture, TextureSubresourceSet subresources) { depthAttachment = FramebufferAttachment().setTexture(texture).setSubresources(subresources); return *this; }
        FramebufferDesc& setShadingRateAttachment(const FramebufferAttachment& d) { shadingRateAttachment = d; return *this; }
        FramebufferDesc& setShadingRateAttachment(ITexture* texture) { shadingRateAttachment = FramebufferAttachment().setTexture(texture); return *this; }
        FramebufferDesc& setShadingRateAttachment(ITexture* texture, TextureSubresourceSet subresources) { shadingRateAttachment = FramebufferAttachment().setTexture(texture).setSubresources(subresources); return *this; }
    };

    /**
     * @struct FramebufferInfo
     * @brief Compact description of a framebuffer for pipeline compatibility checks.
     *
     * Must match between the framebuffer and the pipeline object.
     *
     * @par 中文说明：
     *      帧缓冲的紧凑描述，用于管线兼容性验证。颜色格式列表、深度格式、采样参数等
     *      必须与管线对象一致。
     */
    struct FramebufferInfo
    {
        static_vector<Format, c_MaxRenderTargets> colorFormats;
        Format depthFormat = Format::UNKNOWN;
        uint32_t sampleCount = 1;
        uint32_t sampleQuality = 0;

        FramebufferInfo() = default;
        NVRHI_API FramebufferInfo(const FramebufferDesc& desc);

        bool operator==(const FramebufferInfo& other) const
        {
            return formatsEqual(colorFormats, other.colorFormats)
                && depthFormat == other.depthFormat
                && sampleCount == other.sampleCount
                && sampleQuality == other.sampleQuality;
        }
        bool operator!=(const FramebufferInfo& other) const { return !(*this == other); }

        FramebufferInfo& addColorFormat(Format format) { colorFormats.push_back(format); return *this; }
        FramebufferInfo& setDepthFormat(Format format) { depthFormat = format; return *this; }
        FramebufferInfo& setSampleCount(uint32_t count) { sampleCount = count; return *this; }
        FramebufferInfo& setSampleQuality(uint32_t quality) { sampleQuality = quality; return *this; }

    private:
        static bool formatsEqual(const static_vector<Format, c_MaxRenderTargets>& a, const static_vector<Format, c_MaxRenderTargets>& b)
        {
            if (a.size() != b.size()) return false;
            for (size_t i = 0; i < a.size(); i++) if (a[i] != b[i]) return false;
            return true;
        }
    };

    /**
     * @struct FramebufferInfoEx
     * @brief Extended FramebufferInfo including dimensions and array size.
     *
     * Provides a helper to construct a Viewport from its dimensions.
     *
     * @par 中文说明：
     *      扩展帧缓冲信息，包含宽高及数组大小。可据此生成默认视口。
     */
    struct FramebufferInfoEx : FramebufferInfo
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t arraySize = 1;

        FramebufferInfoEx() = default;
        NVRHI_API FramebufferInfoEx(const FramebufferDesc& desc);

        FramebufferInfoEx& setWidth(uint32_t value) { width = value; return *this; }
        FramebufferInfoEx& setHeight(uint32_t value) { height = value; return *this; }
        FramebufferInfoEx& setArraySize(uint32_t value) { arraySize = value; return *this; }

        [[nodiscard]] Viewport getViewport(float minZ = 0.f, float maxZ = 1.f) const
        {
            return Viewport(0.f, float(width), 0.f, float(height), minZ, maxZ);
        }
    };

    /**
     * @class IFramebuffer
     * @brief Interface for a framebuffer resource.
     *
     * @par 中文说明：
     *      帧缓冲资源接口，可获取描述信息和扩展帧缓冲信息。
     */
    class IFramebuffer : public IResource
    {
    public:
        [[nodiscard]] virtual const FramebufferDesc& getDesc() const = 0;
        [[nodiscard]] virtual const FramebufferInfoEx& getFramebufferInfo() const = 0;
    };

    typedef RefCountPtr<IFramebuffer> FramebufferHandle;
} // namespace helicon

namespace std
{
    template<> struct hash<helicon::TextureSubresourceSet>
    {
        std::size_t operator()(helicon::TextureSubresourceSet const& s) const noexcept
        {
            size_t hash = 0;
            helicon::hash_combine(hash, s.baseMipLevel);
            helicon::hash_combine(hash, s.numMipLevels);
            helicon::hash_combine(hash, s.baseArraySlice);
            helicon::hash_combine(hash, s.numArraySlices);
            return hash;
        }
    };

    template<> struct hash<helicon::BufferRange>
    {
        std::size_t operator()(helicon::BufferRange const& s) const noexcept
        {
            size_t hash = 0;
            helicon::hash_combine(hash, s.byteOffset);
            helicon::hash_combine(hash, s.byteSize);
            return hash;
        }
    };

    template<> struct hash<helicon::FramebufferInfo>
    {
        std::size_t operator()(helicon::FramebufferInfo const& s) const noexcept
        {
            size_t hash = 0;
            for (auto format : s.colorFormats)
                helicon::hash_combine(hash, format);
            helicon::hash_combine(hash, s.depthFormat);
            helicon::hash_combine(hash, s.sampleCount);
            helicon::hash_combine(hash, s.sampleQuality);
            return hash;
        }
    };
}