#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace helicon {

constexpr int version_major = 0;
constexpr int version_minor = 1;
constexpr int version_patch = 0;

enum class Backend : std::uint32_t {
    vulkan = 1,
};

enum class Status : std::uint32_t {
    success = 0,
    unavailable = 1,
    invalid_argument = 2,
    runtime_error = 3,
};

enum class Format : std::uint32_t {
    rgba8_unorm = 1,
};

enum class BufferUsage : std::uint32_t {
    transfer_src = 1u << 0u,
    transfer_dst = 1u << 1u,
    vertex = 1u << 2u,
    index = 1u << 3u,
    uniform = 1u << 4u,
    storage = 1u << 5u,
};

enum class ImageUsage : std::uint32_t {
    transfer_src = 1u << 0u,
    transfer_dst = 1u << 1u,
    sampled = 1u << 2u,
    color_attachment = 1u << 3u,
};

[[nodiscard]] constexpr BufferUsage operator|(BufferUsage left, BufferUsage right) noexcept {
    return static_cast<BufferUsage>(static_cast<std::uint32_t>(left) |
                                    static_cast<std::uint32_t>(right));
}

[[nodiscard]] constexpr ImageUsage operator|(ImageUsage left, ImageUsage right) noexcept {
    return static_cast<ImageUsage>(static_cast<std::uint32_t>(left) |
                                   static_cast<std::uint32_t>(right));
}

struct Extent2D {
    std::uint32_t width{0};
    std::uint32_t height{0};
};

struct Color {
    float r{0.0f};
    float g{0.0f};
    float b{0.0f};
    float a{1.0f};
};

struct BufferDesc {
    std::size_t size{0};
    BufferUsage usage{BufferUsage::storage};
};

struct ImageDesc {
    Extent2D extent{};
    Format format{Format::rgba8_unorm};
    ImageUsage usage{ImageUsage::color_attachment | ImageUsage::transfer_src};
    Color clear_color{};
};

class Error final : public std::runtime_error {
public:
    Error(Status status, std::string message);

    [[nodiscard]] Status status() const noexcept;

private:
    Status status_;
};

[[nodiscard]] std::string_view version_string() noexcept;
[[nodiscard]] std::string_view status_string(Status status) noexcept;
[[nodiscard]] bool backend_available(Backend backend) noexcept;

namespace detail {
class DeviceBackend;
struct BufferHandle;
struct ImageHandle;
struct ShaderModuleHandle;
struct GraphicsPipelineHandle;
struct CommandListHandle;
} // namespace detail

class Buffer {
public:
    Buffer() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    friend class Device;

    Buffer(std::shared_ptr<detail::DeviceBackend> owner,
           std::shared_ptr<detail::BufferHandle> handle,
           std::size_t size) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::BufferHandle> handle_;
    std::size_t size_{0};
};

class Image {
public:
    Image() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] Extent2D extent() const noexcept;
    [[nodiscard]] Format format() const noexcept;

private:
    friend class Device;

    Image(std::shared_ptr<detail::DeviceBackend> owner,
          std::shared_ptr<detail::ImageHandle> handle,
          ImageDesc desc) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::ImageHandle> handle_;
    ImageDesc desc_{};
};

class ShaderModule {
public:
    ShaderModule() = default;

    [[nodiscard]] bool valid() const noexcept;

private:
    friend class Device;

    ShaderModule(std::shared_ptr<detail::DeviceBackend> owner,
                 std::shared_ptr<detail::ShaderModuleHandle> handle) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::ShaderModuleHandle> handle_;
};

class GraphicsPipeline {
public:
    GraphicsPipeline() = default;

    [[nodiscard]] bool valid() const noexcept;

private:
    friend class Device;

    GraphicsPipeline(std::shared_ptr<detail::DeviceBackend> owner,
                     std::shared_ptr<detail::GraphicsPipelineHandle> handle) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::GraphicsPipelineHandle> handle_;
};

class CommandList {
public:
    CommandList() = default;

    [[nodiscard]] bool valid() const noexcept;

private:
    friend class Device;
    friend class Queue;

    CommandList(std::shared_ptr<detail::DeviceBackend> owner,
                std::shared_ptr<detail::CommandListHandle> handle) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::CommandListHandle> handle_;
};

class Queue {
public:
    Queue() = default;

    [[nodiscard]] bool valid() const noexcept;
    void submit_and_wait(CommandList &commands) const;

private:
    friend class Device;

    explicit Queue(std::shared_ptr<detail::DeviceBackend> backend) noexcept;

    std::shared_ptr<detail::DeviceBackend> backend_;
};

class Device {
public:
    Device() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] Backend backend() const;
    [[nodiscard]] Queue graphics_queue() const;

    [[nodiscard]] Buffer create_buffer(const BufferDesc &desc) const;
    [[nodiscard]] Image create_image(const ImageDesc &desc) const;
    [[nodiscard]] ShaderModule create_shader_module(std::span<const std::uint32_t> spirv) const;
    [[nodiscard]] GraphicsPipeline create_builtin_triangle_pipeline(Format color_format) const;
    [[nodiscard]] CommandList create_command_list() const;

private:
    friend class Context;
    friend class RenderGraph;

    explicit Device(std::shared_ptr<detail::DeviceBackend> backend) noexcept;

    std::shared_ptr<detail::DeviceBackend> backend_;
};

class Context {
public:
    [[nodiscard]] static bool backend_available(Backend backend) noexcept;
    [[nodiscard]] Device create_device(Backend backend = Backend::vulkan) const;
};

class RenderGraphImage {
public:
    RenderGraphImage() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::uint32_t index() const noexcept;

private:
    friend class RenderGraph;

    explicit RenderGraphImage(std::uint32_t index) noexcept;

    std::uint32_t index_{UINT32_MAX};
};

class RenderGraphPass {
public:
    void write_color(RenderGraphImage image);
    void write_color(RenderGraphImage image, Color clear_color);
    void draw_builtin_triangle();

private:
    friend class RenderGraph;

    explicit RenderGraphPass(std::string name);

    std::string name_;
    bool has_color_output_{false};
    RenderGraphImage color_output_{};
    Color clear_color_{};
    bool draws_builtin_triangle_{false};
};

class RenderGraph {
public:
    [[nodiscard]] RenderGraphImage create_image(const ImageDesc &desc);
    RenderGraphPass &add_pass(std::string name);
    void execute(const Device &device);

    [[nodiscard]] std::vector<std::uint8_t> read_image_rgba8(RenderGraphImage image) const;

private:
    struct ImageRecord {
        ImageDesc desc{};
        std::vector<std::uint8_t> last_readback;
    };

    std::vector<ImageRecord> images_;
    std::vector<RenderGraphPass> passes_;
};

} // namespace helicon
