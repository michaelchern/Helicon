#pragma once

#include "helicon/helicon.hpp"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace helicon::detail {

struct BufferHandle {
    virtual ~BufferHandle() = default;
};

struct TextureHandle {
    virtual ~TextureHandle() = default;
};

struct SamplerHandle {
    virtual ~SamplerHandle() = default;
};

struct ShaderModuleHandle {
    virtual ~ShaderModuleHandle() = default;
};

struct BindingLayoutHandle {
    virtual ~BindingLayoutHandle() = default;
};

struct BindingSetHandle {
    virtual ~BindingSetHandle() = default;
};

struct GraphicsPipelineHandle {
    virtual ~GraphicsPipelineHandle() = default;
};

struct ComputePipelineHandle {
    virtual ~ComputePipelineHandle() = default;
};

struct FramebufferHandle {
    virtual ~FramebufferHandle() = default;
};

struct CommandListHandle {
    virtual ~CommandListHandle() = default;
};

class DeviceBackend {
public:
    virtual ~DeviceBackend() = default;

    [[nodiscard]] virtual Backend backend() const noexcept = 0;
    [[nodiscard]] virtual DeviceCapabilities capabilities() const = 0;
    [[nodiscard]] virtual std::shared_ptr<DeviceExtension> query_extension(ExtensionKind kind) = 0;
    [[nodiscard]] virtual std::shared_ptr<BufferHandle> create_buffer(const BufferDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<TextureHandle> create_texture(const TextureDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<SamplerHandle> create_sampler(const SamplerDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<ShaderModuleHandle>
    create_shader_module(std::span<const std::uint32_t> spirv) = 0;
    [[nodiscard]] virtual std::shared_ptr<BindingLayoutHandle>
    create_binding_layout(const BindingLayoutDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<BindingSetHandle>
    create_binding_set(const BindingSetDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<GraphicsPipelineHandle>
    create_graphics_pipeline(const GraphicsPipelineDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<GraphicsPipelineHandle>
    create_builtin_triangle_pipeline(Format color_format) = 0;
    [[nodiscard]] virtual std::shared_ptr<ComputePipelineHandle>
    create_compute_pipeline(const ComputePipelineDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<FramebufferHandle>
    create_framebuffer(const FramebufferDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<CommandListHandle> create_command_list(QueueType type) = 0;
    virtual void transition_buffer(CommandListHandle &commands,
                                   BufferHandle &buffer,
                                   ResourceState before,
                                   ResourceState after) = 0;
    virtual void transition_texture(CommandListHandle &commands,
                                    TextureHandle &texture,
                                    ResourceState before,
                                    ResourceState after) = 0;
    virtual void submit_and_wait(QueueType type, CommandListHandle &commands) = 0;
    [[nodiscard]] virtual std::vector<std::uint8_t>
    render_rgba8(const ImageDesc &target_desc, Color clear_color, bool draw_builtin_triangle) = 0;
};

[[nodiscard]] bool vulkan_backend_available() noexcept;
[[nodiscard]] std::shared_ptr<DeviceBackend> create_vulkan_backend();

} // namespace helicon::detail
