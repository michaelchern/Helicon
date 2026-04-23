#include "rhi_internal.hpp"

#include <utility>

namespace helicon {

namespace {

constexpr std::string_view kVersionString = "0.1.0-render-core-v0";

[[nodiscard]] TextureDesc to_texture_desc(const ImageDesc &desc) {
    return TextureDesc{desc.extent, desc.format, desc.usage, desc.clear_color, desc.initial_state, desc.debug_name};
}

[[nodiscard]] bool same_owner(const std::shared_ptr<detail::DeviceBackend> &left,
                              const std::shared_ptr<detail::DeviceBackend> &right) noexcept {
    return left && right && left.get() == right.get();
}

void ensure_device(const std::shared_ptr<detail::DeviceBackend> &backend, std::string_view action) {
    if (!backend) {
        throw Error(Status::invalid_argument, "Cannot " + std::string{action} + " with an invalid device.");
    }
}

[[nodiscard]] bool capabilities_support(const DeviceCapabilities &capabilities, DeviceFeature feature) noexcept {
    switch (feature) {
    case DeviceFeature::graphics:
        return capabilities.graphics_queue;
    case DeviceFeature::compute:
        return capabilities.compute_queue;
    case DeviceFeature::copy:
        return capabilities.copy_queue;
    case DeviceFeature::ray_tracing:
        return capabilities.ray_tracing;
    case DeviceFeature::cuda_interop:
        return capabilities.cuda_interop;
    case DeviceFeature::bindless:
        return capabilities.bindless;
    case DeviceFeature::native_debug:
        return capabilities.native_debug;
    }
    return false;
}

void validate_binding_layout(const BindingLayoutDesc &desc) {
    if (desc.items.empty()) {
        throw Error(Status::invalid_argument, "Binding layouts must contain at least one item.");
    }
    for (const auto &item : desc.items) {
        if (item.type == ResourceType::none) {
            throw Error(Status::invalid_argument, "Binding layout items must declare a resource type.");
        }
        if (item.array_size == 0) {
            throw Error(Status::invalid_argument, "Binding layout array sizes must be non-zero.");
        }
        if (item.type == ResourceType::push_constants && item.byte_size == 0) {
            throw Error(Status::invalid_argument, "Push constant binding layout items must declare a byte size.");
        }
    }
}

} // namespace

Error::Error(Status status, std::string message) : std::runtime_error(std::move(message)), status_(status) {}

Status Error::status() const noexcept {
    return status_;
}

std::string_view version_string() noexcept {
    return kVersionString;
}

std::string_view status_string(Status status) noexcept {
    switch (status) {
    case Status::success:
        return "success";
    case Status::unavailable:
        return "unavailable";
    case Status::invalid_argument:
        return "invalid argument";
    case Status::runtime_error:
        return "runtime error";
    }
    return "unknown status";
}

bool backend_available(Backend backend) noexcept {
    switch (backend) {
    case Backend::vulkan:
        return detail::vulkan_backend_available();
    }
    return false;
}

Buffer::Buffer(std::shared_ptr<detail::DeviceBackend> owner,
               std::shared_ptr<detail::BufferHandle> handle,
               BufferDesc desc) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)), desc_(std::move(desc)) {}

bool Buffer::valid() const noexcept {
    return handle_ != nullptr;
}

std::size_t Buffer::size() const noexcept {
    return desc_.size;
}

Texture::Texture(std::shared_ptr<detail::DeviceBackend> owner,
                 std::shared_ptr<detail::TextureHandle> handle,
                 TextureDesc desc) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)), desc_(std::move(desc)) {}

bool Texture::valid() const noexcept {
    return handle_ != nullptr;
}

Extent2D Texture::extent() const noexcept {
    return desc_.extent;
}

Format Texture::format() const noexcept {
    return desc_.format;
}

Sampler::Sampler(std::shared_ptr<detail::DeviceBackend> owner,
                 std::shared_ptr<detail::SamplerHandle> handle,
                 SamplerDesc desc) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)), desc_(std::move(desc)) {}

bool Sampler::valid() const noexcept {
    return handle_ != nullptr;
}

ShaderModule::ShaderModule(std::shared_ptr<detail::DeviceBackend> owner,
                           std::shared_ptr<detail::ShaderModuleHandle> handle) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)) {}

bool ShaderModule::valid() const noexcept {
    return handle_ != nullptr;
}

BindingLayout::BindingLayout(std::shared_ptr<detail::DeviceBackend> owner,
                             std::shared_ptr<detail::BindingLayoutHandle> handle,
                             BindingLayoutDesc desc) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)), desc_(std::move(desc)) {}

bool BindingLayout::valid() const noexcept {
    return handle_ != nullptr;
}

std::size_t BindingLayout::item_count() const noexcept {
    return desc_.items.size();
}

BindingSet::BindingSet(std::shared_ptr<detail::DeviceBackend> owner,
                       std::shared_ptr<detail::BindingSetHandle> handle) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)) {}

bool BindingSet::valid() const noexcept {
    return handle_ != nullptr;
}

GraphicsPipeline::GraphicsPipeline(std::shared_ptr<detail::DeviceBackend> owner,
                                   std::shared_ptr<detail::GraphicsPipelineHandle> handle) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)) {}

bool GraphicsPipeline::valid() const noexcept {
    return handle_ != nullptr;
}

ComputePipeline::ComputePipeline(std::shared_ptr<detail::DeviceBackend> owner,
                                 std::shared_ptr<detail::ComputePipelineHandle> handle) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)) {}

bool ComputePipeline::valid() const noexcept {
    return handle_ != nullptr;
}

Framebuffer::Framebuffer(std::shared_ptr<detail::DeviceBackend> owner,
                         std::shared_ptr<detail::FramebufferHandle> handle,
                         FramebufferInfo info) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)), info_(std::move(info)) {}

bool Framebuffer::valid() const noexcept {
    return handle_ != nullptr;
}

const FramebufferInfo &Framebuffer::info() const noexcept {
    return info_;
}

CommandList::CommandList(std::shared_ptr<detail::DeviceBackend> owner,
                         std::shared_ptr<detail::CommandListHandle> handle,
                         QueueType type) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)), type_(type) {}

bool CommandList::valid() const noexcept {
    return handle_ != nullptr;
}

void CommandList::transition_buffer(const Buffer &buffer, ResourceState before, ResourceState after) const {
    if (!handle_ || !buffer.handle_ || !same_owner(owner_, buffer.owner_)) {
        throw Error(Status::invalid_argument, "Cannot transition an invalid or foreign buffer.");
    }
    owner_->transition_buffer(*handle_, *buffer.handle_, before, after);
}

void CommandList::transition_texture(const Texture &texture, ResourceState before, ResourceState after) const {
    if (!handle_ || !texture.handle_ || !same_owner(owner_, texture.owner_)) {
        throw Error(Status::invalid_argument, "Cannot transition an invalid or foreign texture.");
    }
    owner_->transition_texture(*handle_, *texture.handle_, before, after);
}

Queue::Queue(std::shared_ptr<detail::DeviceBackend> backend, QueueType type) noexcept
    : backend_(std::move(backend)), type_(type) {}

bool Queue::valid() const noexcept {
    return backend_ != nullptr;
}

QueueType Queue::type() const noexcept {
    return type_;
}

void Queue::submit_and_wait(CommandList &commands) const {
    if (!backend_ || !commands.handle_ || !same_owner(backend_, commands.owner_)) {
        throw Error(Status::invalid_argument, "Cannot submit an invalid or foreign command list.");
    }
    if (commands.type_ != type_) {
        throw Error(Status::invalid_argument, "Cannot submit a command list to a queue of a different type.");
    }
    backend_->submit_and_wait(type_, *commands.handle_);
}

Device::Device(std::shared_ptr<detail::DeviceBackend> backend) noexcept : backend_(std::move(backend)) {}

bool Device::valid() const noexcept {
    return backend_ != nullptr;
}

Backend Device::backend() const {
    ensure_device(backend_, "query");
    return backend_->backend();
}

DeviceCapabilities Device::capabilities() const {
    ensure_device(backend_, "query capabilities");
    return backend_->capabilities();
}

bool Device::supports(DeviceFeature feature) const {
    return capabilities_support(capabilities(), feature);
}

std::shared_ptr<DeviceExtension> Device::query_extension(ExtensionKind kind) const {
    ensure_device(backend_, "query extensions");
    return backend_->query_extension(kind);
}

Queue Device::graphics_queue() const {
    return queue(QueueType::graphics);
}

Queue Device::queue(QueueType type) const {
    ensure_device(backend_, "get a queue");
    switch (type) {
    case QueueType::graphics:
        if (!supports(DeviceFeature::graphics)) {
            throw Error(Status::unavailable, "Graphics queue is not available on this device.");
        }
        break;
    case QueueType::compute:
        if (!supports(DeviceFeature::compute)) {
            throw Error(Status::unavailable, "Compute queue is not available on this device.");
        }
        break;
    case QueueType::copy:
        if (!supports(DeviceFeature::copy)) {
            throw Error(Status::unavailable, "Copy queue is not available on this device.");
        }
        break;
    }
    return Queue{backend_, type};
}

Buffer Device::create_buffer(const BufferDesc &desc) const {
    ensure_device(backend_, "create a buffer");
    return Buffer{backend_, backend_->create_buffer(desc), desc};
}

Texture Device::create_texture(const TextureDesc &desc) const {
    ensure_device(backend_, "create a texture");
    return Texture{backend_, backend_->create_texture(desc), desc};
}

Image Device::create_image(const ImageDesc &desc) const {
    return create_texture(to_texture_desc(desc));
}

Sampler Device::create_sampler(const SamplerDesc &desc) const {
    ensure_device(backend_, "create a sampler");
    return Sampler{backend_, backend_->create_sampler(desc), desc};
}

ShaderModule Device::create_shader_module(std::span<const std::uint32_t> spirv) const {
    ensure_device(backend_, "create a shader module");
    return ShaderModule{backend_, backend_->create_shader_module(spirv)};
}

BindingLayout Device::create_binding_layout(const BindingLayoutDesc &desc) const {
    ensure_device(backend_, "create a binding layout");
    validate_binding_layout(desc);
    return BindingLayout{backend_, backend_->create_binding_layout(desc), desc};
}

BindingSet Device::create_binding_set(const BindingSetDesc &desc) const {
    ensure_device(backend_, "create a binding set");
    if (!desc.layout.valid() || !same_owner(desc.layout.owner_, backend_)) {
        throw Error(Status::invalid_argument, "Binding sets require a valid binding layout from the same device.");
    }
    if (desc.items.empty()) {
        throw Error(Status::invalid_argument, "Binding sets must contain at least one item.");
    }
    for (const auto &item : desc.items) {
        switch (item.type) {
        case ResourceType::constant_buffer:
        case ResourceType::read_only_buffer:
        case ResourceType::read_write_buffer:
            if (!item.buffer.valid() || !same_owner(item.buffer.owner_, backend_)) {
                throw Error(Status::invalid_argument, "Binding set buffer items must reference a valid buffer.");
            }
            break;
        case ResourceType::texture_srv:
        case ResourceType::texture_uav:
            if (!item.texture.valid() || !same_owner(item.texture.owner_, backend_)) {
                throw Error(Status::invalid_argument, "Binding set texture items must reference a valid texture.");
            }
            break;
        case ResourceType::sampler:
            if (!item.sampler.valid() || !same_owner(item.sampler.owner_, backend_)) {
                throw Error(Status::invalid_argument, "Binding set sampler items must reference a valid sampler.");
            }
            break;
        case ResourceType::push_constants:
            break;
        case ResourceType::none:
            throw Error(Status::invalid_argument, "Binding set items must declare a resource type.");
        }
    }
    return BindingSet{backend_, backend_->create_binding_set(desc)};
}

GraphicsPipeline Device::create_graphics_pipeline(const GraphicsPipelineDesc &desc) const {
    ensure_device(backend_, "create a graphics pipeline");
    if (!desc.vertex_shader.valid() || !same_owner(desc.vertex_shader.owner_, backend_)) {
        throw Error(Status::invalid_argument, "Graphics pipelines require a valid vertex shader.");
    }
    if (!desc.fragment_shader.valid() || !same_owner(desc.fragment_shader.owner_, backend_)) {
        throw Error(Status::invalid_argument, "Graphics pipelines require a valid fragment shader.");
    }
    if (desc.framebuffer_info.color_formats.empty()) {
        throw Error(Status::invalid_argument, "Graphics pipelines require at least one color target format.");
    }
    for (const auto &layout : desc.binding_layouts) {
        if (!layout.valid() || !same_owner(layout.owner_, backend_)) {
            throw Error(Status::invalid_argument, "Graphics pipeline binding layouts must be valid.");
        }
    }
    return GraphicsPipeline{backend_, backend_->create_graphics_pipeline(desc)};
}

GraphicsPipeline Device::create_builtin_triangle_pipeline(Format color_format) const {
    ensure_device(backend_, "create a pipeline");
    return GraphicsPipeline{backend_, backend_->create_builtin_triangle_pipeline(color_format)};
}

ComputePipeline Device::create_compute_pipeline(const ComputePipelineDesc &desc) const {
    ensure_device(backend_, "create a compute pipeline");
    if (!desc.compute_shader.valid() || !same_owner(desc.compute_shader.owner_, backend_)) {
        throw Error(Status::invalid_argument, "Compute pipelines require a valid compute shader.");
    }
    for (const auto &layout : desc.binding_layouts) {
        if (!layout.valid() || !same_owner(layout.owner_, backend_)) {
            throw Error(Status::invalid_argument, "Compute pipeline binding layouts must be valid.");
        }
    }
    return ComputePipeline{backend_, backend_->create_compute_pipeline(desc)};
}

Framebuffer Device::create_framebuffer(const FramebufferDesc &desc) const {
    ensure_device(backend_, "create a framebuffer");
    if (desc.color_attachments.empty()) {
        throw Error(Status::invalid_argument, "Framebuffers must contain at least one color attachment.");
    }

    auto info = FramebufferInfo{};
    info.extent = desc.color_attachments.front().extent();
    for (const auto &attachment : desc.color_attachments) {
        if (!attachment.valid() || !same_owner(attachment.owner_, backend_)) {
            throw Error(Status::invalid_argument, "Framebuffer attachments must be valid textures.");
        }
        if (attachment.extent().width != info.extent.width || attachment.extent().height != info.extent.height) {
            throw Error(Status::invalid_argument, "Framebuffer color attachments must have matching extents.");
        }
        info.color_formats.push_back(attachment.format());
    }
    return Framebuffer{backend_, backend_->create_framebuffer(desc), info};
}

CommandList Device::create_command_list(QueueType type) const {
    ensure_device(backend_, "create a command list");
    (void)queue(type);
    return CommandList{backend_, backend_->create_command_list(type), type};
}

bool Context::backend_available(Backend backend) noexcept {
    return helicon::backend_available(backend);
}

Device Context::create_device(Backend backend) const {
    switch (backend) {
    case Backend::vulkan:
        return Device{detail::create_vulkan_backend()};
    }
    throw Error(Status::unavailable, "Requested backend is not available.");
}

} // namespace helicon
