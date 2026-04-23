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

struct ImageHandle {
    virtual ~ImageHandle() = default;
};

struct ShaderModuleHandle {
    virtual ~ShaderModuleHandle() = default;
};

struct GraphicsPipelineHandle {
    virtual ~GraphicsPipelineHandle() = default;
};

struct CommandListHandle {
    virtual ~CommandListHandle() = default;
};

class DeviceBackend {
public:
    virtual ~DeviceBackend() = default;

    [[nodiscard]] virtual Backend backend() const noexcept = 0;
    [[nodiscard]] virtual std::shared_ptr<BufferHandle> create_buffer(const BufferDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<ImageHandle> create_image(const ImageDesc &desc) = 0;
    [[nodiscard]] virtual std::shared_ptr<ShaderModuleHandle>
    create_shader_module(std::span<const std::uint32_t> spirv) = 0;
    [[nodiscard]] virtual std::shared_ptr<GraphicsPipelineHandle>
    create_builtin_triangle_pipeline(Format color_format) = 0;
    [[nodiscard]] virtual std::shared_ptr<CommandListHandle> create_command_list() = 0;
    virtual void submit_and_wait(CommandListHandle &commands) = 0;
    [[nodiscard]] virtual std::vector<std::uint8_t>
    render_rgba8(const ImageDesc &target_desc, Color clear_color, bool draw_builtin_triangle) = 0;
};

[[nodiscard]] bool vulkan_backend_available() noexcept;
[[nodiscard]] std::shared_ptr<DeviceBackend> create_vulkan_backend();

} // namespace helicon::detail
