#pragma once

#include <nvrhi/commandlist.h>

#include <vector>

namespace nvrhi
{
    enum class Feature : uint8_t
    {
        ComputeQueue,
        ConservativeRasterization,
        ConstantBufferRanges,
        CopyQueue,
        DeferredCommandLists,
        FastGeometryShader,
        HeapDirectlyIndexed,
        HlslExtensionUAV,
        LinearSweptSpheres,
        Meshlets,
        RayQuery,
        RayTracingAccelStruct,
        RayTracingClusters,
        RayTracingOpacityMicromap,
        RayTracingPipeline,
        SamplerFeedback,
        ShaderExecutionReordering,
        ShaderSpecializations,
        SinglePassStereo,
        Spheres,
        VariableRateShading,
        VirtualResources,
        WaveLaneCountMinMax,
        CooperativeVectorInferencing,
        CooperativeVectorTraining
    };

    enum class MessageSeverity : uint8_t
    {
        Info,
        Warning,
        Error,
        Fatal
    };

    struct VariableRateShadingFeatureInfo
    {
        uint32_t shadingRateImageTileSize;
    };

    struct WaveLaneCountMinMaxFeatureInfo
    {
        uint32_t minWaveLaneCount;
        uint32_t maxWaveLaneCount;
    };

    // IMessageCallback should be implemented by the application.
    class IMessageCallback
    {
    protected:
        IMessageCallback() = default;
        virtual ~IMessageCallback() = default;

    public:
        // NVRHI will call message(...) whenever it needs to signal something.
        // The application is free to ignore the messages, show message boxes, or terminate.
        virtual void message(MessageSeverity severity, const char* messageText) = 0;

        IMessageCallback(const IMessageCallback&) = delete;
        IMessageCallback(const IMessageCallback&&) = delete;
        IMessageCallback& operator=(const IMessageCallback&) = delete;
        IMessageCallback& operator=(const IMessageCallback&&) = delete;
    };

    //////////////////////////////////////////////////////////////////////////
    // IDevice
    //////////////////////////////////////////////////////////////////////////

    class AftermathCrashDumpHelper;

    class IDevice : public IResource
    {
    public:
        virtual HeapHandle createHeap(const HeapDesc& d) = 0;

        virtual TextureHandle createTexture(const TextureDesc& d) = 0;
        virtual MemoryRequirements getTextureMemoryRequirements(ITexture* texture) = 0;
        virtual bool bindTextureMemory(ITexture* texture, IHeap* heap, uint64_t offset) = 0;

        virtual TextureHandle createHandleForNativeTexture(ObjectType objectType, Object texture, const TextureDesc& desc) = 0;

        virtual StagingTextureHandle createStagingTexture(const TextureDesc& d, CpuAccessMode cpuAccess) = 0;
        virtual void* mapStagingTexture(IStagingTexture* tex, const TextureSlice& slice, CpuAccessMode cpuAccess, size_t* outRowPitch) = 0;
        virtual void unmapStagingTexture(IStagingTexture* tex) = 0;

        virtual void getTextureTiling(ITexture* texture, uint32_t* numTiles, PackedMipDesc* desc, TileShape* tileShape, uint32_t* subresourceTilingsNum, SubresourceTiling* subresourceTilings) = 0;
        virtual void updateTextureTileMappings(ITexture* texture, const TextureTilesMapping* tileMappings, uint32_t numTileMappings, CommandQueue executionQueue = CommandQueue::Graphics) = 0;

        virtual SamplerFeedbackTextureHandle createSamplerFeedbackTexture(ITexture* pairedTexture, const SamplerFeedbackTextureDesc& desc) = 0;
        virtual SamplerFeedbackTextureHandle createSamplerFeedbackForNativeTexture(ObjectType objectType, Object texture, ITexture* pairedTexture) = 0;

        virtual BufferHandle createBuffer(const BufferDesc& d) = 0;
        virtual void* mapBuffer(IBuffer* buffer, CpuAccessMode cpuAccess) = 0;
        virtual void unmapBuffer(IBuffer* buffer) = 0;
        virtual MemoryRequirements getBufferMemoryRequirements(IBuffer* buffer) = 0;
        virtual bool bindBufferMemory(IBuffer* buffer, IHeap* heap, uint64_t offset) = 0;

        virtual BufferHandle createHandleForNativeBuffer(ObjectType objectType, Object buffer, const BufferDesc& desc) = 0;

        virtual ShaderHandle createShader(const ShaderDesc& d, const void* binary, size_t binarySize) = 0;
        virtual ShaderHandle createShaderSpecialization(IShader* baseShader, const ShaderSpecialization* constants, uint32_t numConstants) = 0;
        virtual ShaderLibraryHandle createShaderLibrary(const void* binary, size_t binarySize) = 0;
        
        virtual SamplerHandle createSampler(const SamplerDesc& d) = 0;

        // Note: vertexShader is only necessary on D3D11, otherwise it may be null
        virtual InputLayoutHandle createInputLayout(const VertexAttributeDesc* d, uint32_t attributeCount, IShader* vertexShader) = 0;
        
        // Event queries
        virtual EventQueryHandle createEventQuery() = 0;
        virtual void setEventQuery(IEventQuery* query, CommandQueue queue) = 0;
        virtual bool pollEventQuery(IEventQuery* query) = 0;
        virtual void waitEventQuery(IEventQuery* query) = 0;
        virtual void resetEventQuery(IEventQuery* query) = 0;

        // Timer queries - see also begin/endTimerQuery in ICommandList
        virtual TimerQueryHandle createTimerQuery() = 0;
        virtual bool pollTimerQuery(ITimerQuery* query) = 0;
        // returns time in seconds
        virtual float getTimerQueryTime(ITimerQuery* query) = 0;
        virtual void resetTimerQuery(ITimerQuery* query) = 0;

        // Returns the API kind that the RHI backend is running on top of.
        virtual GraphicsAPI getGraphicsAPI() = 0;
        
        virtual FramebufferHandle createFramebuffer(const FramebufferDesc& desc) = 0;
        
        virtual GraphicsPipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc, FramebufferInfo const& fbinfo) = 0;

        [[deprecated("Use createGraphicsPipeline with FramebufferInfo instead")]]
        virtual GraphicsPipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc, IFramebuffer* fb) = 0;
        
        virtual ComputePipelineHandle createComputePipeline(const ComputePipelineDesc& desc) = 0;

        virtual MeshletPipelineHandle createMeshletPipeline(const MeshletPipelineDesc& desc, FramebufferInfo const& fbinfo) = 0;

        [[deprecated("Use createMeshletPipeline with FramebufferInfo instead")]]
        virtual MeshletPipelineHandle createMeshletPipeline(const MeshletPipelineDesc& desc, IFramebuffer* fb) = 0;

        virtual rt::PipelineHandle createRayTracingPipeline(const rt::PipelineDesc& desc) = 0;
        
        virtual BindingLayoutHandle createBindingLayout(const BindingLayoutDesc& desc) = 0;
        virtual BindingLayoutHandle createBindlessLayout(const BindlessLayoutDesc& desc) = 0;

        virtual BindingSetHandle createBindingSet(const BindingSetDesc& desc, IBindingLayout* layout) = 0;
        virtual DescriptorTableHandle createDescriptorTable(IBindingLayout* layout) = 0;

        virtual void resizeDescriptorTable(IDescriptorTable* descriptorTable, uint32_t newSize, bool keepContents = true) = 0;
        virtual bool writeDescriptorTable(IDescriptorTable* descriptorTable, const BindingSetItem& item) = 0;

        virtual rt::OpacityMicromapHandle createOpacityMicromap(const rt::OpacityMicromapDesc& desc) = 0;
        virtual rt::AccelStructHandle createAccelStruct(const rt::AccelStructDesc& desc) = 0;
        virtual MemoryRequirements getAccelStructMemoryRequirements(rt::IAccelStruct* as) = 0;
        virtual rt::cluster::OperationSizeInfo getClusterOperationSizeInfo(const rt::cluster::OperationParams& params) = 0;
        virtual bool bindAccelStructMemory(rt::IAccelStruct* as, IHeap* heap, uint64_t offset) = 0;
        
        virtual CommandListHandle createCommandList(const CommandListParameters& params = CommandListParameters()) = 0;
        virtual uint64_t executeCommandLists(ICommandList* const* pCommandLists, size_t numCommandLists, CommandQueue executionQueue = CommandQueue::Graphics) = 0;
        virtual void queueWaitForCommandList(CommandQueue waitQueue, CommandQueue executionQueue, uint64_t instance) = 0;
        // returns true if the wait completes successfully, false if detecting a problem (e.g. device removal)
        virtual bool waitForIdle() = 0;

        // Releases the resources that were referenced in the command lists that have finished executing.
        // IMPORTANT: Call this method at least once per frame.
        virtual void runGarbageCollection() = 0;

        virtual bool queryFeatureSupport(Feature feature, void* pInfo = nullptr, size_t infoSize = 0) = 0;

        virtual FormatSupport queryFormatSupport(Format format) = 0;

        // Returns a list of supported CoopVec matrix multiplication formats and accumulation capabilities.
        virtual coopvec::DeviceFeatures queryCoopVecFeatures() = 0;

        // Calculates and returns the on-device size for a CoopVec matrix of the given dimensions, type and layout.
        virtual size_t getCoopVecMatrixSize(coopvec::DataType type, coopvec::MatrixLayout layout, int rows, int columns) = 0;

        virtual Object getNativeQueue(ObjectType objectType, CommandQueue queue) = 0;

        virtual IMessageCallback* getMessageCallback() = 0;

        virtual bool isAftermathEnabled() = 0;
        virtual AftermathCrashDumpHelper& getAftermathCrashDumpHelper() = 0;

        // Front-end for executeCommandLists(..., 1) for compatibility and convenience
        uint64_t executeCommandList(ICommandList* commandList, CommandQueue executionQueue = CommandQueue::Graphics)
        {
            return executeCommandLists(&commandList, 1, executionQueue);
        }
    };

    typedef RefCountPtr<IDevice> DeviceHandle;
} // namespace nvrhi
