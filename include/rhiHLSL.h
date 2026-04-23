// ============================================================================
// @file        rhiHLSL.h
// @brief       Shared C++/HLSL-compatible helper types for ray tracing,
//              indirect arguments, and cluster-related structures mirrored from
//              NVRHI/NVAPI-facing contracts.
// @brief.zh    C++/HLSL 共享的辅助类型头文件，集中放置光追、间接参数以及
//              cluster 相关结构，便于在 NVRHI/NVAPI 约束下复用统一数据布局。
// @project     Helicon
// @author      Michael Chern <1216866818@qq.com>
// @date        2026-04-23
// @note.en     Keep identifiers that are constrained by upstream APIs, specs,
//              shaders, or binary layout contracts unchanged unless the user
//              explicitly asks for a breaking rename.
// @note.zh     受上游 API、规范、shader 或二进制布局约束的名字不要擅自改动；
//              这类文件优先统一注释、模板和排版，而不是盲目重命名。
// ============================================================================

#ifndef NVRHI_HLSL_H
#define NVRHI_HLSL_H

//-----------------------------------------------------------------------------
// Shared Compatibility Types
// C++ / HLSL 共享兼容类型
//-----------------------------------------------------------------------------

#if defined(__cplusplus) || __HLSL_VERSION >= 2021 || __SLANG__
namespace helicon
{
    /**
     * @brief GPU virtual address alias shared by host code and shader-facing layouts.
     *
     * @par 中文说明：
     *      C++ 与 HLSL 共享使用的 GPU 虚拟地址类型别名。
     */
    typedef uint64_t GpuVirtualAddress;

    /**
     * @brief Pair of GPU address and stride used by shader-friendly argument blocks.
     *
     * @par 中文说明：
     *      表示“GPU 地址 + 步长”的组合结构，适合在 shader 友好的参数块中传递。
     */
    struct GpuVirtualAddressAndStride
    {
        GpuVirtualAddress startAddress;
        uint64_t strideInBytes;
    };

    namespace rt
    {
        //-------------------------------------------------------------------------
        // Shader-Friendly Indirect Argument Structures
        // 供 C++ / HLSL 共享的间接参数结构
        //-------------------------------------------------------------------------

        /**
         * @brief Shader-friendly equivalent of `nvrhi::rt::InstanceDesc`.
         *
         * @details This structure is laid out so it can be consumed by shader code
         * while still being usable from C++.
         *
         * @par 中文说明：
         *      `nvrhi::rt::InstanceDesc` 的 shader 友好版本。
         *      结构布局兼顾 C++ 和 shader 访问，适合放进共享参数缓冲或间接参数流中。
         */
        struct IndirectInstanceDesc
        {
#ifdef __cplusplus
            float transform[12];
#else
            float4 transform[3];
#endif
            uint32_t instanceID : 24;
            uint32_t instanceMask : 8;
            uint32_t instanceContributionToHitGroupIndex : 24;
            uint32_t flags : 8;
            GpuVirtualAddress blasDeviceAddress;
        };

        namespace cluster
        {
            //-------------------------------------------------------------------------
            // Cluster Build Constants
            // Cluster 构建常量
            //-------------------------------------------------------------------------

            /// Required byte alignment for CLAS-related data.
            /// CLAS 相关数据要求的字节对齐值。
            static const uint32_t kClasByteAlignment = 128;

            /// Maximum triangle count allowed by the spec.
            /// 规范允许的最大三角形数量。
            static const uint32_t kClasMaxTriangles = 256;

            /// Maximum vertex count allowed by the spec.
            /// 规范允许的最大顶点数量。
            static const uint32_t kClasMaxVertices = 256;

            /// Maximum geometry index allowed by the spec.
            /// 规范允许的最大几何索引值。
            static const uint32_t kMaxGeometryIndex = 16777215;

            //-------------------------------------------------------------------------
            // Cluster Indirect Argument Structures
            // Cluster 间接参数结构
            //-------------------------------------------------------------------------

            /**
             * @brief Mirror of `NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_MULTI_INDIRECT_TRIANGLE_CLUSTER_ARGS`.
             *
             * @details This structure is used when describing one CLAS build input
             * through an indirect argument buffer.
             *
             * @par 中文说明：
             *      对应 NVAPI 的三角形 CLAS 间接参数结构镜像。
             *      适合在间接参数缓冲中描述一次 CLAS 构建输入。
             */
            struct IndirectTriangleClasArgs
            {
                uint32_t clusterId;
                uint32_t clusterFlags;
                uint32_t triangleCount : 9;
                uint32_t vertexCount : 9;
                uint32_t positionTruncateBitCount : 6;
                uint32_t indexFormat : 4;
                uint32_t opacityMicromapIndexFormat : 4;
                uint32_t baseGeometryIndexAndFlags;
                uint16_t indexBufferStride;
                uint16_t vertexBufferStride;
                uint16_t geometryIndexAndFlagsBufferStride;
                uint16_t opacityMicromapIndexBufferStride;
                GpuVirtualAddress indexBuffer;
                GpuVirtualAddress vertexBuffer;
                GpuVirtualAddress geometryIndexAndFlagsBuffer;
                GpuVirtualAddress opacityMicromapArray;
                GpuVirtualAddress opacityMicromapIndexBuffer;
            };

            /**
             * @brief Mirror of `NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_MULTI_INDIRECT_TRIANGLE_TEMPLATE_ARGS`.
             *
             * @par 中文说明：
             *      对应 NVAPI 的三角形 cluster template 间接参数结构镜像。
             *      用于描述 cluster template 的构建输入。
             */
            struct IndirectTriangleTemplateArgs
            {
                uint32_t clusterId;
                uint32_t clusterFlags;
                uint32_t triangleCount : 9;
                uint32_t vertexCount : 9;
                uint32_t positionTruncateBitCount : 6;
                uint32_t indexFormat : 4;
                uint32_t opacityMicromapIndexFormat : 4;
                uint32_t baseGeometryIndexAndFlags;
                uint16_t indexBufferStride;
                uint16_t vertexBufferStride;
                uint16_t geometryIndexAndFlagsBufferStride;
                uint16_t opacityMicromapIndexBufferStride;
                GpuVirtualAddress indexBuffer;
                GpuVirtualAddress vertexBuffer;
                GpuVirtualAddress geometryIndexAndFlagsBuffer;
                GpuVirtualAddress opacityMicromapArray;
                GpuVirtualAddress opacityMicromapIndexBuffer;
                GpuVirtualAddress instantiationBoundingBoxLimit;
            };

            /**
             * @brief Mirror of `NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_MULTI_INDIRECT_INSTANTIATE_TEMPLATE_ARGS`.
             *
             * @par 中文说明：
             *      对应 NVAPI 的 template 实例化间接参数结构镜像。
             *      用于根据已构建的 cluster template 间接生成实例化结果。
             */
            struct IndirectInstantiateTemplateArgs
            {
                uint32_t clusterIdOffset;
                uint32_t geometryIndexOffset;
                GpuVirtualAddress clusterTemplate;
                GpuVirtualAddressAndStride vertexBuffer;
            };

            /**
             * @brief Mirror of `NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_MULTI_INDIRECT_CLUSTER_ARGS`.
             *
             * @par 中文说明：
             *      对应 NVAPI 的 cluster 地址数组参数结构镜像。
             *      用于批量引用已经构建好的 CLAS 地址列表。
             */
            struct IndirectArgs
            {
                uint32_t clusterCount;
                uint32_t reserved;
                GpuVirtualAddress clusterAddresses;
            };
        } // namespace cluster
    } // namespace rt
} // namespace nvrhi

#endif // __HLSL_VERSION >= 2021 || __SLANG__ || __cplusplus
#endif // NVRHI_HLSL_H
