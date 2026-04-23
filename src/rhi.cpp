#include "rhi_internal.hpp"

#include <utility>

namespace helicon {

namespace {

constexpr std::string_view kVersionString = "0.1.0-render-core-v0";

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
               std::size_t size) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)), size_(size) {}

bool Buffer::valid() const noexcept {
    return handle_ != nullptr;
}

std::size_t Buffer::size() const noexcept {
    return size_;
}

Image::Image(std::shared_ptr<detail::DeviceBackend> owner,
             std::shared_ptr<detail::ImageHandle> handle,
             ImageDesc desc) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)), desc_(desc) {}

bool Image::valid() const noexcept {
    return handle_ != nullptr;
}

Extent2D Image::extent() const noexcept {
    return desc_.extent;
}

Format Image::format() const noexcept {
    return desc_.format;
}

ShaderModule::ShaderModule(std::shared_ptr<detail::DeviceBackend> owner,
                           std::shared_ptr<detail::ShaderModuleHandle> handle) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)) {}

bool ShaderModule::valid() const noexcept {
    return handle_ != nullptr;
}

GraphicsPipeline::GraphicsPipeline(std::shared_ptr<detail::DeviceBackend> owner,
                                   std::shared_ptr<detail::GraphicsPipelineHandle> handle) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)) {}

bool GraphicsPipeline::valid() const noexcept {
    return handle_ != nullptr;
}

CommandList::CommandList(std::shared_ptr<detail::DeviceBackend> owner,
                         std::shared_ptr<detail::CommandListHandle> handle) noexcept
    : owner_(std::move(owner)), handle_(std::move(handle)) {}

bool CommandList::valid() const noexcept {
    return handle_ != nullptr;
}

Queue::Queue(std::shared_ptr<detail::DeviceBackend> backend) noexcept : backend_(std::move(backend)) {}

bool Queue::valid() const noexcept {
    return backend_ != nullptr;
}

void Queue::submit_and_wait(CommandList &commands) const {
    if (!backend_ || !commands.handle_) {
        throw Error(Status::invalid_argument, "Cannot submit an invalid command list.");
    }
    backend_->submit_and_wait(*commands.handle_);
}

Device::Device(std::shared_ptr<detail::DeviceBackend> backend) noexcept : backend_(std::move(backend)) {}

bool Device::valid() const noexcept {
    return backend_ != nullptr;
}

Backend Device::backend() const {
    if (!backend_) {
        throw Error(Status::invalid_argument, "Cannot query an invalid device.");
    }
    return backend_->backend();
}

Queue Device::graphics_queue() const {
    if (!backend_) {
        throw Error(Status::invalid_argument, "Cannot get a queue from an invalid device.");
    }
    return Queue{backend_};
}

Buffer Device::create_buffer(const BufferDesc &desc) const {
    if (!backend_) {
        throw Error(Status::invalid_argument, "Cannot create a buffer from an invalid device.");
    }
    return Buffer{backend_, backend_->create_buffer(desc), desc.size};
}

Image Device::create_image(const ImageDesc &desc) const {
    if (!backend_) {
        throw Error(Status::invalid_argument, "Cannot create an image from an invalid device.");
    }
    return Image{backend_, backend_->create_image(desc), desc};
}

ShaderModule Device::create_shader_module(std::span<const std::uint32_t> spirv) const {
    if (!backend_) {
        throw Error(Status::invalid_argument, "Cannot create a shader module from an invalid device.");
    }
    return ShaderModule{backend_, backend_->create_shader_module(spirv)};
}

GraphicsPipeline Device::create_builtin_triangle_pipeline(Format color_format) const {
    if (!backend_) {
        throw Error(Status::invalid_argument, "Cannot create a pipeline from an invalid device.");
    }
    return GraphicsPipeline{backend_, backend_->create_builtin_triangle_pipeline(color_format)};
}

CommandList Device::create_command_list() const {
    if (!backend_) {
        throw Error(Status::invalid_argument, "Cannot create a command list from an invalid device.");
    }
    return CommandList{backend_, backend_->create_command_list()};
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
