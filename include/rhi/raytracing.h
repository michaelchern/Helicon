#pragma once

#include <nvrhi/pipeline.h>

#include <string>
#include <vector>

namespace nvrhi
{
    namespace rt
    {
        //////////////////////////////////////////////////////////////////////////
        // rt::OpacityMicromap
        //////////////////////////////////////////////////////////////////////////

        enum class OpacityMicromapFormat
        {
            OC1_2_State = 1,
            OC1_4_State = 2,
        };

        enum class OpacityMicromapBuildFlags : uint8_t
        {
            None = 0,
            FastTrace = 1,
            FastBuild = 2,
            AllowCompaction = 4
        };

        NVRHI_ENUM_CLASS_FLAG_OPERATORS(OpacityMicromapBuildFlags)

        struct OpacityMicromapUsageCount
        {
            // Number of OMMs with the specified subdivision level and format.
            uint32_t count;
            // Micro triangle count is 4^N, where N is the subdivision level.
            uint32_t subdivisionLevel;
            // OMM input sub format.
            OpacityMicromapFormat format;
        };

        struct OpacityMicromapDesc
        {
            std::string debugName;
            bool trackLiveness = true;

            // OMM flags. Applies to all OMMs in array.
            OpacityMicromapBuildFlags flags;
            // OMM counts for each subdivision level and format combination in the inputs.
            std::vector<OpacityMicromapUsageCount> counts;

            // Base pointer for raw OMM input data.
            // Individual OMMs must be 1B aligned, though natural alignment is recommended.
            // It's also recommended to try to organize OMMs together that are expected to be used spatially close together.
            IBuffer* inputBuffer = nullptr;
            uint64_t inputBufferOffset = 0;

            // One NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_DESC entry per OMM.
            IBuffer* perOmmDescs = nullptr;
            uint64_t perOmmDescsOffset = 0;

            OpacityMicromapDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
            OpacityMicromapDesc& setTrackLiveness(bool value) { trackLiveness = value; return *this; }
            OpacityMicromapDesc& setFlags(OpacityMicromapBuildFlags value) { flags = value; return *this; }
            OpacityMicromapDesc& setCounts(const std::vector<OpacityMicromapUsageCount>& value) { counts = value; return *this; }
            OpacityMicromapDesc& setInputBuffer(IBuffer* value) { inputBuffer = value; return *this; }
            OpacityMicromapDesc& setInputBufferOffset(uint64_t value) { inputBufferOffset = value; return *this; }
            OpacityMicromapDesc& setPerOmmDescs(IBuffer* value) { perOmmDescs = value; return *this; }
            OpacityMicromapDesc& setPerOmmDescsOffset(uint64_t value) { perOmmDescsOffset = value; return *this; }
        };

        class IOpacityMicromap : public IResource
        {
        public:
            [[nodiscard]] virtual const OpacityMicromapDesc& getDesc() const = 0;
            [[nodiscard]] virtual bool isCompacted() const = 0;
            [[nodiscard]] virtual uint64_t getDeviceAddress() const = 0;
        };

        typedef RefCountPtr<IOpacityMicromap> OpacityMicromapHandle;

        //////////////////////////////////////////////////////////////////////////
        // rt::AccelStruct
        //////////////////////////////////////////////////////////////////////////

        class IAccelStruct;

        typedef float AffineTransform[12];

        constexpr AffineTransform c_IdentityTransform = {
        //  +----+----+---------  rotation and scaling
        //  v    v    v
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f
        //                 ^
        //                 +----  translation
        };

        enum class GeometryFlags : uint8_t
        {
            None = 0,
            Opaque = 1,
            NoDuplicateAnyHitInvocation = 2
        };

        NVRHI_ENUM_CLASS_FLAG_OPERATORS(GeometryFlags)

        enum class GeometryType : uint8_t
        {
            Triangles = 0,
            AABBs = 1,
            Spheres = 2,
            Lss = 3
        };

        struct GeometryAABB
        {
            float minX;
            float minY;
            float minZ;
            float maxX;
            float maxY;
            float maxZ;
        };

        struct GeometryTriangles
        {
            IBuffer* indexBuffer = nullptr;   // make sure the first 2 fields in all Geometry 
            IBuffer* vertexBuffer = nullptr;  // structs are IBuffer* for easier debugging
            Format indexFormat = Format::UNKNOWN;
            Format vertexFormat = Format::UNKNOWN; // See D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC for accepted formats and how they are interpreted
            uint64_t indexOffset = 0;
            uint64_t vertexOffset = 0;
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            uint32_t vertexStride = 0;

            IOpacityMicromap* opacityMicromap = nullptr;
            IBuffer* ommIndexBuffer = nullptr;
            uint64_t ommIndexBufferOffset = 0;
            Format ommIndexFormat = Format::UNKNOWN;
            const OpacityMicromapUsageCount* pOmmUsageCounts = nullptr;
            uint32_t numOmmUsageCounts = 0;

            GeometryTriangles& setIndexBuffer(IBuffer* value) { indexBuffer = value; return *this; }
            GeometryTriangles& setVertexBuffer(IBuffer* value) { vertexBuffer = value; return *this; }
            GeometryTriangles& setIndexFormat(Format value) { indexFormat = value; return *this; }
            GeometryTriangles& setVertexFormat(Format value) { vertexFormat = value; return *this; }
            GeometryTriangles& setIndexOffset(uint64_t value) { indexOffset = value; return *this; }
            GeometryTriangles& setVertexOffset(uint64_t value) { vertexOffset = value; return *this; }
            GeometryTriangles& setIndexCount(uint32_t value) { indexCount = value; return *this; }
            GeometryTriangles& setVertexCount(uint32_t value) { vertexCount = value; return *this; }
            GeometryTriangles& setVertexStride(uint32_t value) { vertexStride = value; return *this; }
            GeometryTriangles& setOpacityMicromap(IOpacityMicromap* value) { opacityMicromap = value; return *this; }
            GeometryTriangles& setOmmIndexBuffer(IBuffer* value) { ommIndexBuffer = value; return *this; }
            GeometryTriangles& setOmmIndexBufferOffset(uint64_t value) { ommIndexBufferOffset = value; return *this; }
            GeometryTriangles& setOmmIndexFormat(Format value) { ommIndexFormat = value; return *this; }
            GeometryTriangles& setPOmmUsageCounts(const OpacityMicromapUsageCount* value) { pOmmUsageCounts = value; return *this; }
            GeometryTriangles& setNumOmmUsageCounts(uint32_t value) { numOmmUsageCounts = value; return *this; }
        };

        struct GeometryAABBs
        {
            IBuffer* buffer = nullptr;
            IBuffer* unused = nullptr;
            uint64_t offset = 0;
            uint32_t count = 0;
            uint32_t stride = 0;

            GeometryAABBs& setBuffer(IBuffer* value) { buffer = value; return *this; }
            GeometryAABBs& setOffset(uint64_t value) { offset = value; return *this; }
            GeometryAABBs& setCount(uint32_t value) { count = value; return *this; }
            GeometryAABBs& setStride(uint32_t value) { stride = value; return *this; }
        };

        struct GeometrySpheres
        {
            IBuffer* indexBuffer = nullptr;
            IBuffer* vertexBuffer = nullptr;
            Format indexFormat = Format::UNKNOWN;
            Format vertexPositionFormat = Format::UNKNOWN;
            Format vertexRadiusFormat = Format::UNKNOWN;
            uint64_t indexOffset = 0;
            uint64_t vertexPositionOffset = 0;
            uint64_t vertexRadiusOffset = 0;
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            uint32_t indexStride = 0;
            uint32_t vertexPositionStride = 0;
            uint32_t vertexRadiusStride = 0;

            GeometrySpheres& setIndexBuffer(IBuffer* value) { indexBuffer = value; return *this; }
            GeometrySpheres& setVertexBuffer(IBuffer* value) { vertexBuffer = value; return *this; }
            GeometrySpheres& setIndexFormat(Format value) { indexFormat = value; return *this; }
            GeometrySpheres& setVertexPositionFormat(Format value) { vertexPositionFormat = value; return *this; }
            GeometrySpheres& setVertexRadiusFormat(Format value) { vertexRadiusFormat = value; return *this; }
            GeometrySpheres& setIndexOffset(uint64_t value) { indexOffset = value; return *this; }
            GeometrySpheres& setVertexPositionOffset(uint64_t value) { vertexPositionOffset = value; return *this; }
            GeometrySpheres& setVertexRadiusOffset(uint64_t value) { vertexRadiusOffset = value; return *this; }
            GeometrySpheres& setIndexCount(uint32_t value) { indexCount = value; return *this; }
            GeometrySpheres& setVertexCount(uint32_t value) { vertexCount = value; return *this; }
            GeometrySpheres& setIndexStride(uint32_t value) { indexStride = value; return *this; }
            GeometrySpheres& setVertexPositionStride(uint32_t value) { vertexPositionStride = value; return *this; }
            GeometrySpheres& setVertexRadiusStride(uint32_t value) { vertexRadiusStride = value; return *this; }
        };

        enum class GeometryLssPrimitiveFormat : uint8_t
        {
            List = 0,
            SuccessiveImplicit = 1
        };

        enum class GeometryLssEndcapMode : uint8_t
        {
            None = 0,
            Chained = 1
        };

        struct GeometryLss
        {
            IBuffer* indexBuffer = nullptr;
            IBuffer* vertexBuffer = nullptr;
            Format indexFormat = Format::UNKNOWN;
            Format vertexPositionFormat = Format::UNKNOWN;
            Format vertexRadiusFormat = Format::UNKNOWN;
            uint64_t indexOffset = 0;
            uint64_t vertexPositionOffset = 0;
            uint64_t vertexRadiusOffset = 0;
            uint32_t indexCount = 0;
            uint32_t primitiveCount = 0;
            uint32_t vertexCount = 0;
            uint32_t indexStride = 0;
            uint32_t vertexPositionStride = 0;
            uint32_t vertexRadiusStride = 0;
            GeometryLssPrimitiveFormat primitiveFormat = GeometryLssPrimitiveFormat::List;
            GeometryLssEndcapMode endcapMode = GeometryLssEndcapMode::None;

            GeometryLss& setIndexBuffer(IBuffer* value) { indexBuffer = value; return *this; }
            GeometryLss& setVertexBuffer(IBuffer* value) { vertexBuffer = value; return *this; }
            GeometryLss& setIndexFormat(Format value) { indexFormat = value; return *this; }
            GeometryLss& setVertexPositionFormat(Format value) { vertexPositionFormat = value; return *this; }
            GeometryLss& setVertexRadiusFormat(Format value) { vertexRadiusFormat = value; return *this; }
            GeometryLss& setIndexOffset(uint64_t value) { indexOffset = value; return *this; }
            GeometryLss& setVertexPositionOffset(uint64_t value) { vertexPositionOffset = value; return *this; }
            GeometryLss& setVertexRadiusOffset(uint64_t value) { vertexRadiusOffset = value; return *this; }
            GeometryLss& setIndexCount(uint32_t value) { indexCount = value; return *this; }
            GeometryLss& setPrimitiveCount(uint32_t value) { primitiveCount = value; return *this; }
            GeometryLss& setVertexCount(uint32_t value) { vertexCount = value; return *this; }
            GeometryLss& setIndexStride(uint32_t value) { indexStride = value; return *this; }
            GeometryLss& setVertexPositionStride(uint32_t value) { vertexPositionStride = value; return *this; }
            GeometryLss& setVertexRadiusStride(uint32_t value) { vertexRadiusStride = value; return *this; }
            GeometryLss& setPrimitiveFormat(GeometryLssPrimitiveFormat value) { primitiveFormat = value; return *this; }
            GeometryLss& setEndcapMode(GeometryLssEndcapMode value) { endcapMode = value; return *this; }
        };

        struct GeometryDesc
        {
            union GeomTypeUnion
            {
                GeometryTriangles triangles;
                GeometryAABBs aabbs;
                GeometrySpheres spheres;
                GeometryLss lss;
            } geometryData;

            bool useTransform = false;
            AffineTransform transform{};
            GeometryFlags flags = GeometryFlags::None;
            GeometryType geometryType = GeometryType::Triangles;

            GeometryDesc() : geometryData{} { }

            GeometryDesc& setTransform(const AffineTransform& value) { memcpy(&transform, &value, sizeof(AffineTransform)); useTransform = true; return *this; }
            GeometryDesc& setFlags(GeometryFlags value) { flags = value; return *this; }
            GeometryDesc& setTriangles(const GeometryTriangles& value) { geometryData.triangles = value; geometryType = GeometryType::Triangles; return *this; }
            GeometryDesc& setAABBs(const GeometryAABBs& value) { geometryData.aabbs = value; geometryType = GeometryType::AABBs; return *this; }
            GeometryDesc& setSpheres(const GeometrySpheres& value) { geometryData.spheres = value; geometryType = GeometryType::Spheres; return *this; }
            GeometryDesc& setLss(const GeometryLss& value) { geometryData.lss = value; geometryType = GeometryType::Lss; return *this; }
        };
        
        enum class InstanceFlags : unsigned
        {
            None = 0,
            TriangleCullDisable = 1,
            TriangleFrontCounterclockwise = 2,
            ForceOpaque = 4,
            ForceNonOpaque = 8,
            ForceOMM2State = 16,
            DisableOMMs = 32,
        };

        NVRHI_ENUM_CLASS_FLAG_OPERATORS(InstanceFlags)

        struct InstanceDesc
        {
            AffineTransform transform;
            unsigned instanceID : 24;
            unsigned instanceMask : 8;
            unsigned instanceContributionToHitGroupIndex : 24;
            InstanceFlags flags : 8;
            union
            {
                IAccelStruct* bottomLevelAS; // for buildTopLevelAccelStruct
                uint64_t blasDeviceAddress;  // for buildTopLevelAccelStructFromBuffer - use IAccelStruct::getDeviceAddress()
            };

            InstanceDesc()  // NOLINT(cppcoreguidelines-pro-type-member-init)
                : instanceID(0)
                , instanceMask(0)
                , instanceContributionToHitGroupIndex(0)
                , flags(InstanceFlags::None)
                , bottomLevelAS(nullptr)
            {
                setTransform(c_IdentityTransform);
            }

            InstanceDesc& setInstanceID(uint32_t value) { instanceID = value; return *this; }
            InstanceDesc& setInstanceContributionToHitGroupIndex(uint32_t value) { instanceContributionToHitGroupIndex = value; return *this; }
            InstanceDesc& setInstanceMask(uint32_t value) { instanceMask = value; return *this; }
            InstanceDesc& setTransform(const AffineTransform& value) { memcpy(&transform, &value, sizeof(AffineTransform)); return *this; }
            InstanceDesc& setFlags(InstanceFlags value) { flags = value; return *this; }
            InstanceDesc& setBLAS(IAccelStruct* value) { bottomLevelAS = value; return *this; }
        };

        static_assert(sizeof(InstanceDesc) == 64, "sizeof(InstanceDesc) is supposed to be 64 bytes");
        static_assert(sizeof(IndirectInstanceDesc) == sizeof(InstanceDesc));

        enum class AccelStructBuildFlags : uint8_t
        {
            None = 0,
            AllowUpdate = 1,
            AllowCompaction = 2,
            PreferFastTrace = 4,
            PreferFastBuild = 8,
            MinimizeMemory = 0x10,
            PerformUpdate = 0x20,

            // Removes the errors or warnings that NVRHI validation layer issues when a TLAS
            // includes an instance that points at a NULL BLAS or has a zero instance mask.
            // Only affects the validation layer, doesn't translate to Vk/DX12 AS build flags.
            AllowEmptyInstances = 0x80
        };

        NVRHI_ENUM_CLASS_FLAG_OPERATORS(AccelStructBuildFlags)

        struct AccelStructDesc
        {
            size_t topLevelMaxInstances = 0; // only applies when isTopLevel = true
            std::vector<GeometryDesc> bottomLevelGeometries; // only applies when isTopLevel = false
            AccelStructBuildFlags buildFlags = AccelStructBuildFlags::None;
            std::string debugName;
            bool trackLiveness = true;
            bool isTopLevel = false;
            bool isVirtual = false;

            AccelStructDesc& setTopLevelMaxInstances(size_t value) { topLevelMaxInstances = value; isTopLevel = true; return *this; }
            AccelStructDesc& addBottomLevelGeometry(const GeometryDesc& value) { bottomLevelGeometries.push_back(value); isTopLevel = false; return *this; }
            AccelStructDesc& setBuildFlags(AccelStructBuildFlags value) { buildFlags = value; return *this; }
            AccelStructDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
            AccelStructDesc& setTrackLiveness(bool value) { trackLiveness = value; return *this; }
            AccelStructDesc& setIsTopLevel(bool value) { isTopLevel = value; return *this; }
            AccelStructDesc& setIsVirtual(bool value) { isVirtual = value; return *this; }
        };

        //////////////////////////////////////////////////////////////////////////
        // rt::AccelStruct
        //////////////////////////////////////////////////////////////////////////

        class IAccelStruct : public IResource
        {
        public:
            [[nodiscard]] virtual const AccelStructDesc& getDesc() const = 0;
            [[nodiscard]] virtual bool isCompacted() const = 0;
            [[nodiscard]] virtual uint64_t getDeviceAddress() const = 0;
        };

        typedef RefCountPtr<IAccelStruct> AccelStructHandle;


        //////////////////////////////////////////////////////////////////////////
        // Clusters
        //////////////////////////////////////////////////////////////////////////
        namespace cluster
        {
            enum class OperationType : uint8_t
            {
                Move,                       // Moves CLAS, CLAS Templates, or Cluster BLAS
                ClasBuild,                  // Builds CLAS from clusters of triangles
                ClasBuildTemplates,         // Builds CLAS templates from triangles
                ClasInstantiateTemplates,   // Instantiates CLAS templates
                BlasBuild                   // Builds Cluster BLAS from CLAS
            };

            enum class OperationMoveType : uint8_t
            {
                BottomLevel,                // Moved objects are Clustered BLAS
                ClusterLevel,               // Moved objects are CLAS
                Template                    // Moved objects are Cluster Templates
            };

            enum class OperationMode : uint8_t
            {
                ImplicitDestinations,       // Provide total buffer space, driver places results within, returns VAs and actual sizes
                ExplicitDestinations,       // Provide individual target VAs, driver places them there, returns actual sizes
                GetSizes                    // Get minimum size per element
            };

            enum class OperationFlags : uint8_t
            {
                None = 0x0,
                FastTrace = 0x1,
                FastBuild = 0x2,
                NoOverlap = 0x4,
                AllowOMM = 0x8
            };
            NVRHI_ENUM_CLASS_FLAG_OPERATORS(OperationFlags);

            enum class OperationIndexFormat : uint8_t
            {
                IndexFormat8bit = 1,
                IndexFormat16bit = 2,
                IndexFormat32bit = 4
            };

            struct OperationSizeInfo
            {
                uint64_t resultMaxSizeInBytes = 0;
                uint64_t scratchSizeInBytes = 0;
            };

            struct OperationMoveParams
            {
                OperationMoveType type;
                uint32_t maxBytes = 0;
            };

            struct OperationClasBuildParams
            {
                // See D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC for accepted formats and how they are interpreted
                Format vertexFormat = Format::RGB32_FLOAT;

                // Index of the last geometry in a single CLAS
                uint32_t maxGeometryIndex = 0;

                // Maximum number of unique geometries in a single CLAS
                uint32_t maxUniqueGeometryCount = 1;

                // Maximum number of triangles in a single CLAS
                uint32_t maxTriangleCount = 0;

                // Maximum number of vertices in a single CLAS
                uint32_t maxVertexCount = 0;

                // Maximum number of triangles summed over all CLAS (in the current cluster operation)
                uint32_t maxTotalTriangleCount = 0;

                // Maximum number of vertices summed over all CLAS (in the current cluster operation)
                uint32_t maxTotalVertexCount = 0;

                // Minimum number of bits to be truncated in vertex positions across all CLAS (in the current cluster operation)
                uint32_t minPositionTruncateBitCount = 0;
            };

            struct OperationBlasBuildParams
            {
                // Maximum number of CLAS references in a single BLAS
                uint32_t maxClasPerBlasCount = 0;

                // Maximum number of CLAS references summed over all BLAS (in the current cluster operation)
                uint32_t maxTotalClasCount = 0;
            };

            // Params that can be used to getClusterOperationSizeInfo on this shared struct before passing to executeMultiIndirectClusterOperation
            struct OperationParams
            {
                // Maximum number of acceleration structures (or templates) to build/instantiate/move
                uint32_t maxArgCount = 0;

                OperationType type;
                OperationMode mode;
                OperationFlags flags;

                OperationMoveParams move;
                OperationClasBuildParams clas;
                OperationBlasBuildParams blas;
            };

            struct OperationDesc
            {
                OperationParams params;

                uint64_t scratchSizeInBytes = 0;                        // Size of scratch resource returned by getClusterOperationSizeInfo() scratchSizeInBytes 

                // Input Resources
                IBuffer* inIndirectArgCountBuffer = nullptr;            // Buffer containing the number of AS to build, instantiate, or move
                uint64_t inIndirectArgCountOffsetInBytes = 0;           // Offset (in bytes) to where the count is in the inIndirectArgCountBuffer 
                IBuffer* inIndirectArgsBuffer = nullptr;                // Buffer of descriptor array of format IndirectTriangleClasArgs, IndirectTriangleTemplateArgs, IndirectInstantiateTemplateArgs
                uint64_t inIndirectArgsOffsetInBytes = 0;               // Offset (in bytes) to where the descriptor array starts inIndirectArgsBuffer

                // In/Out Resources
                IBuffer* inOutAddressesBuffer = nullptr;                // Array of addresses of CLAS, CLAS Templates, or BLAS
                uint64_t inOutAddressesOffsetInBytes = 0;               // Offset (in bytes) to where the addresses array starts in inOutAddressesBuffer

                // Output Resources
                IBuffer* outSizesBuffer = nullptr;                      // Sizes (in bytes) of CLAS, CLAS Templates, or BLAS
                uint64_t outSizesOffsetInBytes = 0;                     // Offset (in bytes) to where the output sizes array starts in outSizesBuffer
                IBuffer* outAccelerationStructuresBuffer = nullptr;     // Destination buffer for CLAS, CLAS Template, or BLAS data. Size must be calculated with getOperationSizeInfo or with the outSizesBuffer result of OperationMode::GetSizes
                uint64_t outAccelerationStructuresOffsetInBytes = 0;    // Offset (in bytes) to where the output acceleration structures starts in outAccelerationStructuresBuffer
            };
        } // namespace cluster
    }

    //////////////////////////////////////////////////////////////////////////
    // Ray Tracing
    //////////////////////////////////////////////////////////////////////////

    namespace rt
    {
        struct PipelineShaderDesc
        {
            std::string exportName;
            ShaderHandle shader;
            BindingLayoutHandle bindingLayout;

            PipelineShaderDesc& setExportName(const std::string& value) { exportName = value; return *this; }
            PipelineShaderDesc& setShader(IShader* value) { shader = value; return *this; }
            PipelineShaderDesc& setBindingLayout(IBindingLayout* value) { bindingLayout = value; return *this; }
        };

        struct PipelineHitGroupDesc
        {
            std::string exportName;
            ShaderHandle closestHitShader;
            ShaderHandle anyHitShader;
            ShaderHandle intersectionShader;
            BindingLayoutHandle bindingLayout;
            bool isProceduralPrimitive = false;

            PipelineHitGroupDesc& setExportName(const std::string& value) { exportName = value; return *this; }
            PipelineHitGroupDesc& setClosestHitShader(IShader* value) { closestHitShader = value; return *this; }
            PipelineHitGroupDesc& setAnyHitShader(IShader* value) { anyHitShader = value; return *this; }
            PipelineHitGroupDesc& setIntersectionShader(IShader* value) { intersectionShader = value; return *this; }
            PipelineHitGroupDesc& setBindingLayout(IBindingLayout* value) { bindingLayout = value; return *this; }
            PipelineHitGroupDesc& setIsProceduralPrimitive(bool value) { isProceduralPrimitive = value; return *this; }
        };

        struct PipelineDesc
        {
            std::vector<PipelineShaderDesc> shaders;
            std::vector<PipelineHitGroupDesc> hitGroups;
            BindingLayoutVector globalBindingLayouts;
            uint32_t maxPayloadSize = 0;
            uint32_t maxAttributeSize = sizeof(float) * 2; // typical case: float2 uv;
            uint32_t maxRecursionDepth = 1;
            int32_t hlslExtensionsUAV = -1;
            bool allowOpacityMicromaps = false;

            PipelineDesc& addShader(const PipelineShaderDesc& value) { shaders.push_back(value); return *this; }
            PipelineDesc& addHitGroup(const PipelineHitGroupDesc& value) { hitGroups.push_back(value); return *this; }
            PipelineDesc& addBindingLayout(IBindingLayout* value) { globalBindingLayouts.push_back(value); return *this; }
            PipelineDesc& setMaxPayloadSize(uint32_t value) { maxPayloadSize = value; return *this; }
            PipelineDesc& setMaxAttributeSize(uint32_t value) { maxAttributeSize = value; return *this; }
            PipelineDesc& setMaxRecursionDepth(uint32_t value) { maxRecursionDepth = value; return *this; }
            PipelineDesc& setHlslExtensionsUAV(int32_t value) { hlslExtensionsUAV = value; return *this; }
            PipelineDesc& setAllowOpacityMicromaps(bool value) { allowOpacityMicromaps = value; return *this; }
        };

        class IPipeline;

        struct ShaderTableDesc
        {
            // Controls the memory usage and building behavior of the shader table.
            //
            // - When a shader table is cached, it creates an additional buffer that holds the built shader table.
            //   This buffer is updated in CommandList::setRayTracingState after the shader table is modified.
            // - When a shader table is uncached, this buffer is suballocated from the upload manager when the shader
            //   table is first used in CommandList::setRayTracingState after opening a command list, and reallocated
            //   and rebuilt on subsequent calls to setRayTracingState if the shader table is modified.
            //
            // The legacy and default behavior is uncached.
            // It is recommended to enable caching for large and infrequently updated shader tables.
            bool isCached = false;

            // Maximum number of entries in a cached shader table.
            // Must be nonzero when isCached == true.
            // Ignored when isCached == false.
            uint32_t maxEntries = 0;

            std::string debugName;

            ShaderTableDesc& setIsCached(bool value) { isCached = value; return *this; }
            ShaderTableDesc& setMaxEntries(uint32_t value) { maxEntries = value; return *this; }
            ShaderTableDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
            ShaderTableDesc& enableCaching(uint32_t _maxEntries) { isCached = true; maxEntries = _maxEntries; return *this; }
        };

        class IShaderTable : public IResource
        {
        public:
            virtual ShaderTableDesc const& getDesc() const = 0;
            virtual uint32_t getNumEntries() const = 0;
            virtual IPipeline* getPipeline() const = 0;
            virtual void setRayGenerationShader(const char* exportName, IBindingSet* bindings = nullptr) = 0;
            virtual int addMissShader(const char* exportName, IBindingSet* bindings = nullptr) = 0;
            virtual int addHitGroup(const char* exportName, IBindingSet* bindings = nullptr) = 0;
            virtual int addCallableShader(const char* exportName, IBindingSet* bindings = nullptr) = 0;
            virtual void clearMissShaders() = 0;
            virtual void clearHitShaders() = 0;
            virtual void clearCallableShaders() = 0;
        };

        typedef RefCountPtr<IShaderTable> ShaderTableHandle;

        class IPipeline : public IResource
        {
        public:
            [[nodiscard]] virtual const rt::PipelineDesc& getDesc() const = 0;
            virtual ShaderTableHandle createShaderTable(ShaderTableDesc const& desc = ShaderTableDesc()) = 0;
        };

        typedef RefCountPtr<IPipeline> PipelineHandle;

        struct State
        {
            IShaderTable* shaderTable = nullptr;

            BindingSetVector bindings;

            State& setShaderTable(IShaderTable* value) { shaderTable = value; return *this; }
            State& addBindingSet(IBindingSet* value) { bindings.push_back(value); return *this; }
        };

        struct DispatchRaysArguments
        {
            uint32_t width = 1;
            uint32_t height = 1;
            uint32_t depth = 1;

            constexpr DispatchRaysArguments& setWidth(uint32_t value) { width = value; return *this; }
            constexpr DispatchRaysArguments& setHeight(uint32_t value) { height = value; return *this; }
            constexpr DispatchRaysArguments& setDepth(uint32_t value) { depth = value; return *this; }
            constexpr DispatchRaysArguments& setDimensions(uint32_t w, uint32_t h = 1, uint32_t d = 1) { width = w; height = h; depth = d; return *this; }
        };
    }
} // namespace nvrhi
