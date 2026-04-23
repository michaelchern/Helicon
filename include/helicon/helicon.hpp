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

enum class QueueType : std::uint32_t {
    graphics = 1,
    compute = 2,
    copy = 3,
};

enum class DeviceFeature : std::uint32_t {
    graphics = 1u << 0u,
    compute = 1u << 1u,
    copy = 1u << 2u,
    ray_tracing = 1u << 3u,
    cuda_interop = 1u << 4u,
    bindless = 1u << 5u,
    native_debug = 1u << 6u,
};

enum class ExtensionKind : std::uint32_t {
    ray_tracing = 1,
    cuda_interop = 2,
    bindless = 3,
    native_debug = 4,
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

enum class TextureUsage : std::uint32_t {
    transfer_src = 1u << 0u,
    transfer_dst = 1u << 1u,
    sampled = 1u << 2u,
    color_attachment = 1u << 3u,
};

using ImageUsage = TextureUsage;

enum class ResourceState : std::uint32_t {
    undefined = 0,
    common = 1,
    transfer_src = 2,
    transfer_dst = 3,
    vertex_buffer = 4,
    index_buffer = 5,
    constant_buffer = 6,
    shader_resource = 7,
    unordered_access = 8,
    render_target = 9,
    present = 10,
};

enum class ShaderStage : std::uint32_t {
    none = 0,
    vertex = 1u << 0u,
    fragment = 1u << 1u,
    compute = 1u << 2u,
    all_graphics = 3u,
    all = 7u,
};

enum class ResourceType : std::uint32_t {
    none = 0,
    constant_buffer = 1,
    read_only_buffer = 2,
    read_write_buffer = 3,
    texture_srv = 4,
    texture_uav = 5,
    sampler = 6,
    push_constants = 7,
};

enum class SamplerFilter : std::uint32_t {
    nearest = 1,
    linear = 2,
};

enum class AddressMode : std::uint32_t {
    clamp_to_edge = 1,
    repeat = 2,
};

[[nodiscard]] constexpr BufferUsage operator|(BufferUsage left, BufferUsage right) noexcept {
    return static_cast<BufferUsage>(static_cast<std::uint32_t>(left) |
                                    static_cast<std::uint32_t>(right));
}

[[nodiscard]] constexpr TextureUsage operator|(TextureUsage left, TextureUsage right) noexcept {
    return static_cast<TextureUsage>(static_cast<std::uint32_t>(left) |
                                     static_cast<std::uint32_t>(right));
}

[[nodiscard]] constexpr ShaderStage operator|(ShaderStage left, ShaderStage right) noexcept {
    return static_cast<ShaderStage>(static_cast<std::uint32_t>(left) |
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

struct DeviceCapabilities {
    Backend backend{Backend::vulkan};
    bool graphics_queue{false};
    bool compute_queue{false};
    bool copy_queue{false};
    bool ray_tracing{false};
    bool cuda_interop{false};
    bool bindless{false};
    bool native_debug{false};
    std::uint32_t max_color_attachments{1};
    std::uint32_t max_frames_in_flight{1};
};

struct BufferDesc {
    std::size_t size{0};
    BufferUsage usage{BufferUsage::storage};
    ResourceState initial_state{ResourceState::common};
    std::string debug_name;
};

struct TextureDesc {
    Extent2D extent{};
    Format format{Format::rgba8_unorm};
    TextureUsage usage{TextureUsage::color_attachment | TextureUsage::transfer_src};
    Color clear_color{};
    ResourceState initial_state{ResourceState::undefined};
    std::string debug_name;
};

struct ImageDesc {
    Extent2D extent{};
    Format format{Format::rgba8_unorm};
    ImageUsage usage{ImageUsage::color_attachment | ImageUsage::transfer_src};
    Color clear_color{};
    ResourceState initial_state{ResourceState::undefined};
    std::string debug_name;
};

struct SamplerDesc {
    SamplerFilter min_filter{SamplerFilter::linear};
    SamplerFilter mag_filter{SamplerFilter::linear};
    AddressMode address_u{AddressMode::clamp_to_edge};
    AddressMode address_v{AddressMode::clamp_to_edge};
    AddressMode address_w{AddressMode::clamp_to_edge};
    std::string debug_name;
};

struct BindingLayoutItem {
    ResourceType type{ResourceType::none};
    std::uint32_t slot{0};
    std::uint32_t array_size{1};
    ShaderStage visibility{ShaderStage::all};
    std::uint32_t byte_size{0};
};

struct BindingLayoutDesc {
    std::uint32_t set_index{0};
    std::vector<BindingLayoutItem> items;
    std::string debug_name;
};

struct FramebufferInfo {
    Extent2D extent{};
    std::vector<Format> color_formats;
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
struct TextureHandle;
struct SamplerHandle;
struct ShaderModuleHandle;
struct BindingLayoutHandle;
struct BindingSetHandle;
struct GraphicsPipelineHandle;
struct ComputePipelineHandle;
struct FramebufferHandle;
struct CommandListHandle;
} // namespace detail

class DeviceExtension {
public:
    virtual ~DeviceExtension() = default;

    [[nodiscard]] virtual ExtensionKind kind() const noexcept = 0;
};

class Buffer {
public:
    Buffer() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    friend class Device;
    friend class CommandList;

    Buffer(std::shared_ptr<detail::DeviceBackend> owner,
           std::shared_ptr<detail::BufferHandle> handle,
           BufferDesc desc) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::BufferHandle> handle_;
    BufferDesc desc_{};
};

class Texture {
public:
    Texture() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] Extent2D extent() const noexcept;
    [[nodiscard]] Format format() const noexcept;

private:
    friend class Device;
    friend class CommandList;
    friend class RenderGraph;

    Texture(std::shared_ptr<detail::DeviceBackend> owner,
            std::shared_ptr<detail::TextureHandle> handle,
            TextureDesc desc) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::TextureHandle> handle_;
    TextureDesc desc_{};
};

using Image = Texture;

class Sampler {
public:
    Sampler() = default;

    [[nodiscard]] bool valid() const noexcept;

private:
    friend class Device;

    Sampler(std::shared_ptr<detail::DeviceBackend> owner,
            std::shared_ptr<detail::SamplerHandle> handle,
            SamplerDesc desc) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::SamplerHandle> handle_;
    SamplerDesc desc_{};
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

class BindingLayout {
public:
    BindingLayout() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::size_t item_count() const noexcept;

private:
    friend class Device;

    BindingLayout(std::shared_ptr<detail::DeviceBackend> owner,
                  std::shared_ptr<detail::BindingLayoutHandle> handle,
                  BindingLayoutDesc desc) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::BindingLayoutHandle> handle_;
    BindingLayoutDesc desc_{};
};

struct BindingSetItem {
    ResourceType type{ResourceType::none};
    std::uint32_t slot{0};
    std::uint32_t array_element{0};
    Buffer buffer{};
    Texture texture{};
    Sampler sampler{};
};

struct BindingSetDesc {
    BindingLayout layout{};
    std::vector<BindingSetItem> items;
    bool track_liveness{true};
    std::string debug_name;
};

struct GraphicsPipelineDesc {
    ShaderModule vertex_shader{};
    ShaderModule fragment_shader{};
    std::vector<BindingLayout> binding_layouts;
    FramebufferInfo framebuffer_info{};
    std::string debug_name;
};

struct ComputePipelineDesc {
    ShaderModule compute_shader{};
    std::vector<BindingLayout> binding_layouts;
    std::string debug_name;
};

struct FramebufferDesc {
    std::vector<Texture> color_attachments;
    std::string debug_name;
};

class BindingSet {
public:
    BindingSet() = default;

    [[nodiscard]] bool valid() const noexcept;

private:
    friend class Device;

    BindingSet(std::shared_ptr<detail::DeviceBackend> owner,
               std::shared_ptr<detail::BindingSetHandle> handle) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::BindingSetHandle> handle_;
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

class ComputePipeline {
public:
    ComputePipeline() = default;

    [[nodiscard]] bool valid() const noexcept;

private:
    friend class Device;

    ComputePipeline(std::shared_ptr<detail::DeviceBackend> owner,
                    std::shared_ptr<detail::ComputePipelineHandle> handle) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::ComputePipelineHandle> handle_;
};

class Framebuffer {
public:
    Framebuffer() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] const FramebufferInfo &info() const noexcept;

private:
    friend class Device;

    Framebuffer(std::shared_ptr<detail::DeviceBackend> owner,
                std::shared_ptr<detail::FramebufferHandle> handle,
                FramebufferInfo info) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::FramebufferHandle> handle_;
    FramebufferInfo info_{};
};

class CommandList {
public:
    CommandList() = default;

    [[nodiscard]] bool valid() const noexcept;
    void transition_buffer(const Buffer &buffer, ResourceState before, ResourceState after) const;
    void transition_texture(const Texture &texture, ResourceState before, ResourceState after) const;

private:
    friend class Device;
    friend class Queue;

    CommandList(std::shared_ptr<detail::DeviceBackend> owner,
                std::shared_ptr<detail::CommandListHandle> handle,
                QueueType type) noexcept;

    std::shared_ptr<detail::DeviceBackend> owner_;
    std::shared_ptr<detail::CommandListHandle> handle_;
    QueueType type_{QueueType::graphics};
};

class Queue {
public:
    Queue() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] QueueType type() const noexcept;
    void submit_and_wait(CommandList &commands) const;

private:
    friend class Device;

    Queue(std::shared_ptr<detail::DeviceBackend> backend, QueueType type) noexcept;

    std::shared_ptr<detail::DeviceBackend> backend_;
    QueueType type_{QueueType::graphics};
};

class Device {
public:
    Device() = default;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] Backend backend() const;
    [[nodiscard]] DeviceCapabilities capabilities() const;
    [[nodiscard]] bool supports(DeviceFeature feature) const;
    [[nodiscard]] std::shared_ptr<DeviceExtension> query_extension(ExtensionKind kind) const;
    [[nodiscard]] Queue graphics_queue() const;
    [[nodiscard]] Queue queue(QueueType type) const;

    [[nodiscard]] Buffer create_buffer(const BufferDesc &desc) const;
    [[nodiscard]] Texture create_texture(const TextureDesc &desc) const;
    [[nodiscard]] Image create_image(const ImageDesc &desc) const;
    [[nodiscard]] Sampler create_sampler(const SamplerDesc &desc = {}) const;
    [[nodiscard]] ShaderModule create_shader_module(std::span<const std::uint32_t> spirv) const;
    [[nodiscard]] BindingLayout create_binding_layout(const BindingLayoutDesc &desc) const;
    [[nodiscard]] BindingSet create_binding_set(const BindingSetDesc &desc) const;
    [[nodiscard]] GraphicsPipeline create_graphics_pipeline(const GraphicsPipelineDesc &desc) const;
    [[nodiscard]] GraphicsPipeline create_builtin_triangle_pipeline(Format color_format) const;
    [[nodiscard]] ComputePipeline create_compute_pipeline(const ComputePipelineDesc &desc) const;
    [[nodiscard]] Framebuffer create_framebuffer(const FramebufferDesc &desc) const;
    [[nodiscard]] CommandList create_command_list(QueueType type = QueueType::graphics) const;

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
