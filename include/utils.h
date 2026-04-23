// ============================================================================
// @file        utils.h
// @brief       Helicon utility declarations for descriptor construction,
//              barriers, debug-string conversion, debug-name generation, and
//              lightweight RAII helpers around NVRHI-facing workflows.
// @brief.zh    Helicon/NVRHI 工作流相关的通用工具声明，集中放置描述符构建、
//              屏障、调试字符串转换、调试名称生成以及轻量 RAII 辅助工具。
// @project     Helicon
// @author      Michael Chern <1216866818@qq.com>
// @date        2026-04-23
// @note.en     English comments are optimized for AI retrieval and coding context.
// @note.zh     中文注释用于帮助作者快速理解职责、用途和调用场景。
//
// Copyright (c) 2025-2026 Michael Chern. All rights reserved.
// ============================================================================

#pragma once

#include <mutex>
#include <rhi.h>

namespace helicon::utils
{
    //-----------------------------------------------------------------------------
    // Blend State Utilities
    // 混合状态辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Creates a render-target blend state configured for additive blending.
     * @param src_blend Source blend factor, such as `BlendFactor::SrcAlpha`.
     * @param dst_blend Destination blend factor, such as `BlendFactor::One`.
     * @return A `BlendState::RenderTarget` value ready to be copied into a blend-state description.
     *
     * @details This helper focuses on the color blend factors needed by common
     * additive and alpha-style accumulation paths. It reduces repeated setup code
     * at call sites.
     *
     * @par 中文说明：
     *      生成一个用于加法混合的渲染目标混合状态。
     *      这个辅助函数主要封装颜色混合因子的常见组合，减少调用端重复填写
     *      blend state 细节的代码量。
     */
    NVRHI_API BlendState::RenderTarget create_add_blend_state(
        BlendFactor src_blend,
        BlendFactor dst_blend);

    //-----------------------------------------------------------------------------
    // Buffer Description Utilities
    // 缓冲区描述辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Creates a descriptor for a static, non-volatile constant buffer.
     * @param byte_size Buffer size in bytes.
     * @param debug_name Optional debug name attached to the descriptor.
     * @return A `BufferDesc` configured for constant-buffer usage with static lifetime semantics.
     *
     * @details Use this helper for constant buffers whose contents are uploaded
     * once or updated infrequently. The descriptor is intended for resources
     * that should not be treated as rotating volatile buffers.
     *
     * @par 中文说明：
     *      生成一个静态常量缓冲区的描述符。
     *      适合初始化后很少更新的数据，或者生命周期内只做少量修改的常量数据。
     *      返回的描述符会按“非易变常量缓冲”这一用途进行配置。
     */
    NVRHI_API BufferDesc create_static_constant_buffer_desc(
        uint32_t byte_size,
        const char* debug_name);

    /**
     * @brief Creates a descriptor for a volatile constant buffer that may be updated frequently.
     * @param byte_size Buffer size in bytes.
     * @param debug_name Optional debug name attached to the descriptor.
     * @param max_versions Maximum number of backing versions available for rotation.
     * @return A `BufferDesc` configured for volatile constant-buffer usage.
     *
     * @details This helper is intended for per-frame, per-pass, or per-draw data
     * where frequent CPU updates are expected. `max_versions` expresses how much
     * versioning space the caller wants the runtime/backend to have available.
     *
     * @par 中文说明：
     *      生成一个易变常量缓冲区的描述符，适用于频繁更新的数据。
     *      例如逐帧、逐 pass、逐 draw 改变的常量参数都适合使用这类描述。
     *      `max_versions` 表示可轮转的版本数量上限。
     */
    NVRHI_API BufferDesc create_volatile_constant_buffer_desc(
        uint32_t byte_size,
        const char* debug_name,
        uint32_t max_versions);

    //-----------------------------------------------------------------------------
    // Binding Utilities
    // 绑定布局与绑定集辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Creates a binding layout and its matching binding set from one description.
     * @param device Device used to allocate backend resources.
     * @param visibility Shader visibility applied to the binding layout.
     * @param register_space Register space or descriptor-set index used by the bindings.
     * @param binding_set_desc Binding description shared by the layout and set.
     * @param binding_layout Output handle receiving the created binding layout.
     * @param binding_set Output handle receiving the created binding set.
     * @param register_space_is_descriptor_set Whether `register_space` should be interpreted as a descriptor-set index.
     * @return `true` when both objects are created successfully; otherwise `false`.
     *
     * @details This helper keeps layout creation and binding-set creation paired
     * at the call site so the two objects do not drift apart semantically.
     *
     * @par 中文说明：
     *      根据同一份绑定描述同时创建 BindingLayout 和 BindingSet。
     *      这样可以把“布局定义”和“具体绑定实例”放在同一处创建，避免两边语义脱节。
     *      `register_space_is_descriptor_set` 用于区分某些后端里 register space
     *      是否需要直接视为 descriptor set 索引。
     */
    NVRHI_API bool create_binding_set_and_layout(
        IDevice* device,
        nvrhi::ShaderType visibility,
        uint32_t register_space,
        const BindingSetDesc& binding_set_desc,
        BindingLayoutHandle& binding_layout,
        BindingSetHandle& binding_set,
        bool register_space_is_descriptor_set = false);

    //-----------------------------------------------------------------------------
    // Attachment Clear Utilities
    // 附件清理辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Clears a color attachment on the specified framebuffer.
     * @param command_list Command list that records the clear operation.
     * @param framebuffer Framebuffer containing the target attachment.
     * @param attachment_index Index of the color attachment to clear.
     * @param color Clear color value.
     *
     * @details This helper is a focused convenience wrapper for color clears in
     * utility or setup passes where the caller already knows the framebuffer and
     * attachment slot to target.
     *
     * @par 中文说明：
     *      清理指定 framebuffer 中的某个颜色附件。
     *      适合初始化纹理、重置中间目标，或者在简单图形流程里显式执行颜色清屏。
     */
    NVRHI_API void clear_color_attachment(
        ICommandList* command_list,
        IFramebuffer* framebuffer,
        uint32_t attachment_index,
        Color color);

    /**
     * @brief Clears the depth-stencil attachment on the specified framebuffer.
     * @param command_list Command list that records the clear operation.
     * @param framebuffer Framebuffer containing the depth-stencil attachment.
     * @param depth Depth clear value.
     * @param stencil Stencil clear value.
     *
     * @par 中文说明：
     *      清理指定 framebuffer 的深度/模板附件。
     *      当某个 pass 需要显式重置深度或模板值时，可以通过这个辅助函数直接记录清理命令。
     */
    NVRHI_API void clear_depth_stencil_attachment(
        ICommandList* command_list,
        IFramebuffer* framebuffer,
        float depth,
        uint32_t stencil);

    //-----------------------------------------------------------------------------
    // Ray Tracing Utilities
    // 光追加速结构辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Records commands that build or rebuild a bottom-level acceleration structure.
     * @param command_list Command list used to record the build commands.
     * @param accel_struct Target acceleration-structure object.
     * @param desc Build description containing geometry and build parameters.
     *
     * @details This helper centralizes the common BLAS build path so call sites
     * can stay concise while still expressing the explicit command-list flow.
     *
     * @par 中文说明：
     *      记录构建或重建底层加速结构（BLAS）的命令。
     *      适合把光追构建流程里重复出现的“根据描述发出 build 命令”的逻辑集中起来。
     */
    NVRHI_API void build_bottom_level_accel_struct(
        ICommandList* command_list,
        rt::IAccelStruct* accel_struct,
        const rt::AccelStructDesc& desc);

    //-----------------------------------------------------------------------------
    // UAV Barrier Utilities
    // UAV 屏障辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Places a UAV barrier for the provided texture resource.
     * @param command_list Command list that records the barrier.
     * @param texture Texture that needs UAV ordering guarantees.
     *
     * @details This is useful when multiple dispatches reuse the same UAV
     * texture across consecutive passes and explicit ordering is required.
     * The backend may ignore the barrier when UAV barriers were disabled for the texture.
     *
     * @par 中文说明：
     *      为指定纹理放置一条 UAV barrier。
     *      当同一个 UAV 纹理被多个连续 dispatch 反复读写时，这个辅助函数可用于补充显式顺序保证。
     *      如果该纹理已关闭 UAV barrier，底层实现可能会忽略这条命令。
     */
    NVRHI_API void texture_uav_barrier(
        ICommandList* command_list,
        ITexture* texture);

    /**
     * @brief Places a UAV barrier for the provided buffer resource.
     * @param command_list Command list that records the barrier.
     * @param buffer Buffer that needs UAV ordering guarantees.
     *
     * @details This helper mirrors `texture_uav_barrier` but targets UAV buffers.
     *
     * @par 中文说明：
     *      为指定缓冲区放置一条 UAV barrier。
     *      用法和纹理版本一致，只是目标资源换成了 buffer。
     */
    NVRHI_API void buffer_uav_barrier(
        ICommandList* command_list,
        IBuffer* buffer);

    //-----------------------------------------------------------------------------
    // Format Selection Utilities
    // 格式选择辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Chooses the first format that satisfies all required features on the device.
     * @param device Device used to query format support.
     * @param required_features Feature flags the chosen format must support.
     * @param requested_formats Candidate formats in priority order.
     * @param requested_format_count Number of candidate formats in `requested_formats`.
     * @return The first supported format, or `Format::UNKNOWN` when none match.
     *
     * @details Candidates are tested in the same order they are provided, so
     * callers can encode preference directly in the array order.
     *
     * @par 中文说明：
     *      从候选格式列表中选择第一个满足全部能力要求的格式。
     *      函数会按传入顺序逐个检查，因此数组顺序本身就表达了优先级。
     *      如果没有任何候选格式满足要求，则返回 `Format::UNKNOWN`。
     */
    NVRHI_API Format choose_format(
        IDevice* device,
        nvrhi::FormatSupport required_features,
        const nvrhi::Format* requested_formats,
        size_t requested_format_count);

    //-----------------------------------------------------------------------------
    // String Conversion Utilities
    // 调试字符串转换辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Converts commonly used enums or debug-facing values into readable strings.
     *
     * @details These helpers are intended for logs, assertions, debug UI, and
     * breadcrumbs where raw enum values are difficult to interpret quickly.
     *
     * @par 中文说明：
     *      这组函数把常见枚举和值转换成可读字符串，适合日志、断言、调试界面和问题定位输出。
     */
    NVRHI_API const char* graphics_api_to_string(GraphicsAPI api);
    NVRHI_API const char* texture_dimension_to_string(TextureDimension dimension);
    NVRHI_API const char* debug_name_to_string(const std::string& debug_name);
    NVRHI_API const char* shader_stage_to_string(ShaderType stage);
    NVRHI_API const char* resource_type_to_string(ResourceType type);
    NVRHI_API const char* format_to_string(Format format);
    NVRHI_API const char* command_queue_to_string(CommandQueue queue);

    //-----------------------------------------------------------------------------
    // Debug Name Utilities
    // 调试名称生成辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Generates readable debug names from common resource descriptors.
     *
     * @details These helpers are useful when resource names are derived from
     * descriptor content rather than authored manually at every call site.
     *
     * @par 中文说明：
     *      这组函数根据描述符内容生成更易读的调试名称。
     *      适合在没有手写 debug name 的情况下，为堆、纹理和缓冲区快速生成可追踪名称。
     */
    std::string generate_heap_debug_name(const HeapDesc& desc);
    std::string generate_texture_debug_name(const TextureDesc& desc);
    std::string generate_buffer_debug_name(const BufferDesc& desc);

    //-----------------------------------------------------------------------------
    // Error Utilities
    // 错误与未支持路径辅助函数
    //-----------------------------------------------------------------------------

    /**
     * @brief Emits or triggers a generic "not implemented" failure path.
     * @par 中文说明：
     *      用于标记“功能尚未实现”的路径，方便在开发阶段快速暴露遗漏分支。
     */
    void not_implemented();

    /**
     * @brief Emits or triggers a generic "not supported" failure path.
     * @par 中文说明：
     *      用于标记“当前后端或当前配置不支持”的路径。
     */
    void not_supported();

    /**
     * @brief Emits or triggers a generic invalid-enum failure path.
     * @par 中文说明：
     *      用于标记枚举值非法、未覆盖或不应到达的分支。
     */
    void invalid_enum();

    //-----------------------------------------------------------------------------
    // Allocation Utilities
    // 位图分配辅助类
    //-----------------------------------------------------------------------------

    /**
     * @brief Simple bitset-based index allocator with optional internal locking.
     *
     * @details The allocator tracks slot occupancy using a bitset-like container
     * and returns integer indices. It is suitable for lightweight handle pools,
     * transient slots, or other dense index allocation tasks.
     *
     * @par 中文说明：
     *      一个基于位图思想的简单索引分配器，可选内部互斥保护。
     *      适合管理紧凑的槽位编号，例如句柄池、临时资源索引或固定容量表项。
     */
    class BitSetAllocator
    {
    public:
        /**
         * @brief Creates an allocator with a fixed capacity.
         * @param capacity Number of allocatable slots.
         * @param multi_threaded Whether allocation and release should use internal locking.
         *
         * @par 中文说明：
         *      创建一个固定容量的分配器。
         *      如果 `multi_threaded` 为 `true`，则 `allocate` 和 `release`
         *      会通过内部互斥量保护并发访问。
         */
        explicit BitSetAllocator(size_t capacity, bool multi_threaded);

        /**
         * @brief Allocates one free slot and returns its index.
         * @return Allocated slot index. The exact failure behavior depends on the implementation.
         *
         * @par 中文说明：
         *      分配一个可用槽位并返回其索引。
         */
        int allocate();

        /**
         * @brief Releases a previously allocated slot.
         * @param index Slot index to release.
         *
         * @par 中文说明：
         *      释放一个已经分配过的槽位索引。
         */
        void release(int index);

        /**
         * @brief Returns the total allocator capacity.
         * @return Number of tracked slots.
         *
         * @par 中文说明：
         *      返回分配器总容量，而不是当前剩余容量。
         */
        [[nodiscard]] size_t get_capacity() const { return m_allocated.size(); }

    private:
        int m_next_available = 0;
        std::vector<bool> m_allocated;
        bool m_multi_threaded;
        std::mutex m_mutex;
    };

    //-----------------------------------------------------------------------------
    // Debug Marker Utilities
    // 调试标记作用域辅助类
    //-----------------------------------------------------------------------------

    /**
     * @brief RAII helper that begins a marker on construction and ends it on destruction.
     *
     * @details This wrapper keeps GPU/debug marker scopes exception-safe and
     * early-return-safe by tying `beginMarker` and `endMarker` to object lifetime.
     *
     * @par 中文说明：
     *      一个基于 RAII 的调试标记辅助类：构造时调用 `beginMarker`，
     *      析构时自动调用 `endMarker`。
     *      这样即使函数提前返回，也能保证 marker 成对闭合。
     */
    class ScopedMarker
    {
    public:
        /// Raw command-list pointer that owns the active marker scope.
        /// 当前持有活动 marker 作用域的命令列表指针。
        ICommandList* m_command_list;

        /**
         * @brief Opens a marker scope on the supplied command list.
         * @param command_list Command list that will receive the marker calls.
         * @param marker_name Marker label shown in debug tooling.
         *
         * @par 中文说明：
         *      在给定命令列表上开始一个调试标记作用域。
         */
        ScopedMarker(ICommandList* command_list, const char* marker_name) : m_command_list(command_list)
        {
            m_command_list->beginMarker(marker_name);
        }

        /**
         * @brief Overload that accepts a handle wrapper instead of a raw pointer.
         * @param command_list Handle wrapper whose underlying command list will be used.
         * @param marker_name Marker label shown in debug tooling.
         *
         * @par 中文说明：
         *      这是一个便利重载，允许直接传入 `CommandListHandle`。
         */
        ScopedMarker(CommandListHandle* command_list, const char* marker_name) :
            ScopedMarker(command_list->Get(), marker_name)
        {}

        /**
         * @brief Closes the marker scope opened by the constructor.
         *
         * @par 中文说明：
         *      自动结束构造时开启的调试标记作用域。
         */
        ~ScopedMarker()
        {
            m_command_list->endMarker();
        }
    };

} // namespace helicon::utils
