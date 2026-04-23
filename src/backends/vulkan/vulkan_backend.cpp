#include "rhi_internal.hpp"

#include "shaders/builtin_triangle_spirv.hpp"

#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace helicon::detail {

namespace {

constexpr VkFormat kColorFormat = VK_FORMAT_R8G8B8A8_UNORM;

[[noreturn]] void throw_vk(Status status, std::string_view message) {
    throw Error(status, std::string{message});
}

void check(VkResult result, std::string_view message) {
    if (result != VK_SUCCESS) {
        throw Error(Status::runtime_error, std::string{message} + " VkResult=" + std::to_string(result));
    }
}

VkFormat to_vk_format(Format format) {
    switch (format) {
    case Format::rgba8_unorm:
        return kColorFormat;
    }
    throw_vk(Status::invalid_argument, "Unsupported image format.");
}

VkBufferUsageFlags to_vk_usage(BufferUsage usage) {
    auto flags = VkBufferUsageFlags{0};
    const auto bits = static_cast<std::uint32_t>(usage);
    if ((bits & static_cast<std::uint32_t>(BufferUsage::transfer_src)) != 0u) {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ((bits & static_cast<std::uint32_t>(BufferUsage::transfer_dst)) != 0u) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if ((bits & static_cast<std::uint32_t>(BufferUsage::vertex)) != 0u) {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if ((bits & static_cast<std::uint32_t>(BufferUsage::index)) != 0u) {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if ((bits & static_cast<std::uint32_t>(BufferUsage::uniform)) != 0u) {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if ((bits & static_cast<std::uint32_t>(BufferUsage::storage)) != 0u) {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    return flags == 0 ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : flags;
}

VkImageUsageFlags to_vk_usage(ImageUsage usage) {
    auto flags = VkImageUsageFlags{0};
    const auto bits = static_cast<std::uint32_t>(usage);
    if ((bits & static_cast<std::uint32_t>(ImageUsage::transfer_src)) != 0u) {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ((bits & static_cast<std::uint32_t>(ImageUsage::transfer_dst)) != 0u) {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ((bits & static_cast<std::uint32_t>(ImageUsage::sampled)) != 0u) {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if ((bits & static_cast<std::uint32_t>(ImageUsage::color_attachment)) != 0u) {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    return flags == 0 ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : flags;
}

VkFilter to_vk_filter(SamplerFilter filter) {
    switch (filter) {
    case SamplerFilter::nearest:
        return VK_FILTER_NEAREST;
    case SamplerFilter::linear:
        return VK_FILTER_LINEAR;
    }
    throw_vk(Status::invalid_argument, "Unsupported sampler filter.");
}

VkSamplerAddressMode to_vk_address_mode(AddressMode mode) {
    switch (mode) {
    case AddressMode::clamp_to_edge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case AddressMode::repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
    throw_vk(Status::invalid_argument, "Unsupported sampler address mode.");
}

bool layer_available(std::string_view name) {
    auto count = std::uint32_t{0};
    if (vkEnumerateInstanceLayerProperties(&count, nullptr) != VK_SUCCESS) {
        return false;
    }
    auto layers = std::vector<VkLayerProperties>(count);
    if (vkEnumerateInstanceLayerProperties(&count, layers.data()) != VK_SUCCESS) {
        return false;
    }
    return std::any_of(layers.begin(), layers.end(), [&](const auto &layer) {
        return name == layer.layerName;
    });
}

std::uint32_t find_graphics_queue_family(VkPhysicalDevice physical_device) {
    auto count = std::uint32_t{0};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
    auto families = std::vector<VkQueueFamilyProperties>(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, families.data());

    for (std::uint32_t i = 0; i < count; ++i) {
        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
            return i;
        }
    }
    return std::numeric_limits<std::uint32_t>::max();
}

bool physical_device_supports_rgba8_target(VkPhysicalDevice physical_device) {
    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(physical_device, kColorFormat, &properties);
    constexpr auto required = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    return (properties.optimalTilingFeatures & required) == required;
}

std::vector<const char *> validation_layers() {
    auto layers = std::vector<const char *>{};
    if (layer_available("VK_LAYER_KHRONOS_validation")) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }
    return layers;
}

VkInstance create_instance(bool enable_validation) {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Helicon";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = "Helicon";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    const auto layers = enable_validation ? validation_layers() : std::vector<const char *>{};

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
    create_info.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    VkInstance instance = VK_NULL_HANDLE;
    check(vkCreateInstance(&create_info, nullptr, &instance), "Failed to create Vulkan instance.");
    return instance;
}

class VulkanBufferHandle final : public BufferHandle {
public:
    VulkanBufferHandle(VkDevice device,
                       VkBuffer buffer,
                       VkDeviceMemory memory,
                       VkDeviceSize size) noexcept
        : device_(device), buffer_(buffer), memory_(memory), size_(size) {}

    VulkanBufferHandle(const VulkanBufferHandle &) = delete;
    VulkanBufferHandle &operator=(const VulkanBufferHandle &) = delete;

    ~VulkanBufferHandle() override {
        if (buffer_ != VK_NULL_HANDLE) {
            vkDestroyBuffer(device_, buffer_, nullptr);
        }
        if (memory_ != VK_NULL_HANDLE) {
            vkFreeMemory(device_, memory_, nullptr);
        }
    }

    [[nodiscard]] VkBuffer buffer() const noexcept { return buffer_; }
    [[nodiscard]] VkDeviceMemory memory() const noexcept { return memory_; }
    [[nodiscard]] VkDeviceSize size() const noexcept { return size_; }

private:
    VkDevice device_{VK_NULL_HANDLE};
    VkBuffer buffer_{VK_NULL_HANDLE};
    VkDeviceMemory memory_{VK_NULL_HANDLE};
    VkDeviceSize size_{0};
};

class VulkanImageHandle final : public TextureHandle {
public:
    VulkanImageHandle(VkDevice device,
                      VkImage image,
                      VkDeviceMemory memory,
                      Extent2D extent,
                      VkFormat format) noexcept
        : device_(device), image_(image), memory_(memory), extent_(extent), format_(format) {}

    VulkanImageHandle(const VulkanImageHandle &) = delete;
    VulkanImageHandle &operator=(const VulkanImageHandle &) = delete;

    ~VulkanImageHandle() override {
        if (image_ != VK_NULL_HANDLE) {
            vkDestroyImage(device_, image_, nullptr);
        }
        if (memory_ != VK_NULL_HANDLE) {
            vkFreeMemory(device_, memory_, nullptr);
        }
    }

    [[nodiscard]] VkImage image() const noexcept { return image_; }
    [[nodiscard]] Extent2D extent() const noexcept { return extent_; }
    [[nodiscard]] VkFormat format() const noexcept { return format_; }

private:
    VkDevice device_{VK_NULL_HANDLE};
    VkImage image_{VK_NULL_HANDLE};
    VkDeviceMemory memory_{VK_NULL_HANDLE};
    Extent2D extent_{};
    VkFormat format_{VK_FORMAT_UNDEFINED};
};

class VulkanShaderModuleHandle final : public ShaderModuleHandle {
public:
    VulkanShaderModuleHandle(VkDevice device, VkShaderModule module) noexcept
        : device_(device), module_(module) {}

    VulkanShaderModuleHandle(const VulkanShaderModuleHandle &) = delete;
    VulkanShaderModuleHandle &operator=(const VulkanShaderModuleHandle &) = delete;

    ~VulkanShaderModuleHandle() override {
        if (module_ != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device_, module_, nullptr);
        }
    }

    [[nodiscard]] VkShaderModule module() const noexcept { return module_; }

private:
    VkDevice device_{VK_NULL_HANDLE};
    VkShaderModule module_{VK_NULL_HANDLE};
};

class VulkanSamplerHandle final : public SamplerHandle {
public:
    VulkanSamplerHandle(VkDevice device, VkSampler sampler) noexcept : device_(device), sampler_(sampler) {}

    VulkanSamplerHandle(const VulkanSamplerHandle &) = delete;
    VulkanSamplerHandle &operator=(const VulkanSamplerHandle &) = delete;

    ~VulkanSamplerHandle() override {
        if (sampler_ != VK_NULL_HANDLE) {
            vkDestroySampler(device_, sampler_, nullptr);
        }
    }

    [[nodiscard]] VkSampler sampler() const noexcept { return sampler_; }

private:
    VkDevice device_{VK_NULL_HANDLE};
    VkSampler sampler_{VK_NULL_HANDLE};
};

class VulkanBindingLayoutHandle final : public BindingLayoutHandle {};
class VulkanBindingSetHandle final : public BindingSetHandle {};
class VulkanGraphicsPipelineHandle final : public GraphicsPipelineHandle {};
class VulkanComputePipelineHandle final : public ComputePipelineHandle {};
class VulkanFramebufferHandle final : public FramebufferHandle {};

class VulkanCommandListHandle final : public CommandListHandle {
public:
    VulkanCommandListHandle(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer) noexcept
        : device_(device), command_pool_(command_pool), command_buffer_(command_buffer) {}

    VulkanCommandListHandle(const VulkanCommandListHandle &) = delete;
    VulkanCommandListHandle &operator=(const VulkanCommandListHandle &) = delete;

    ~VulkanCommandListHandle() override {
        if (command_buffer_ != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
        }
    }

    [[nodiscard]] VkCommandBuffer command_buffer() const noexcept { return command_buffer_; }

private:
    VkDevice device_{VK_NULL_HANDLE};
    VkCommandPool command_pool_{VK_NULL_HANDLE};
    VkCommandBuffer command_buffer_{VK_NULL_HANDLE};
};

struct ScopedImageView {
    VkDevice device{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
    ScopedImageView() = default;
    ScopedImageView(VkDevice device_in, VkImageView view_in) noexcept : device(device_in), view(view_in) {}
    ScopedImageView(const ScopedImageView &) = delete;
    ScopedImageView &operator=(const ScopedImageView &) = delete;
    ScopedImageView(ScopedImageView &&other) noexcept : device(other.device), view(other.view) {
        other.view = VK_NULL_HANDLE;
    }
    ScopedImageView &operator=(ScopedImageView &&other) noexcept {
        if (this != &other) {
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(device, view, nullptr);
            }
            device = other.device;
            view = other.view;
            other.view = VK_NULL_HANDLE;
        }
        return *this;
    }
    ~ScopedImageView() {
        if (view != VK_NULL_HANDLE) {
            vkDestroyImageView(device, view, nullptr);
        }
    }
};

struct ScopedRenderPass {
    VkDevice device{VK_NULL_HANDLE};
    VkRenderPass render_pass{VK_NULL_HANDLE};
    ScopedRenderPass() = default;
    ScopedRenderPass(VkDevice device_in, VkRenderPass render_pass_in) noexcept
        : device(device_in), render_pass(render_pass_in) {}
    ScopedRenderPass(const ScopedRenderPass &) = delete;
    ScopedRenderPass &operator=(const ScopedRenderPass &) = delete;
    ScopedRenderPass(ScopedRenderPass &&other) noexcept
        : device(other.device), render_pass(other.render_pass) {
        other.render_pass = VK_NULL_HANDLE;
    }
    ScopedRenderPass &operator=(ScopedRenderPass &&other) noexcept {
        if (this != &other) {
            if (render_pass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(device, render_pass, nullptr);
            }
            device = other.device;
            render_pass = other.render_pass;
            other.render_pass = VK_NULL_HANDLE;
        }
        return *this;
    }
    ~ScopedRenderPass() {
        if (render_pass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, render_pass, nullptr);
        }
    }
};

struct ScopedFramebuffer {
    VkDevice device{VK_NULL_HANDLE};
    VkFramebuffer framebuffer{VK_NULL_HANDLE};
    ScopedFramebuffer() = default;
    ScopedFramebuffer(VkDevice device_in, VkFramebuffer framebuffer_in) noexcept
        : device(device_in), framebuffer(framebuffer_in) {}
    ScopedFramebuffer(const ScopedFramebuffer &) = delete;
    ScopedFramebuffer &operator=(const ScopedFramebuffer &) = delete;
    ScopedFramebuffer(ScopedFramebuffer &&other) noexcept
        : device(other.device), framebuffer(other.framebuffer) {
        other.framebuffer = VK_NULL_HANDLE;
    }
    ScopedFramebuffer &operator=(ScopedFramebuffer &&other) noexcept {
        if (this != &other) {
            if (framebuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(device, framebuffer, nullptr);
            }
            device = other.device;
            framebuffer = other.framebuffer;
            other.framebuffer = VK_NULL_HANDLE;
        }
        return *this;
    }
    ~ScopedFramebuffer() {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    }
};

struct ScopedPipelineLayout {
    VkDevice device{VK_NULL_HANDLE};
    VkPipelineLayout layout{VK_NULL_HANDLE};
    ScopedPipelineLayout() = default;
    ScopedPipelineLayout(VkDevice device_in, VkPipelineLayout layout_in) noexcept
        : device(device_in), layout(layout_in) {}
    ScopedPipelineLayout(const ScopedPipelineLayout &) = delete;
    ScopedPipelineLayout &operator=(const ScopedPipelineLayout &) = delete;
    ScopedPipelineLayout(ScopedPipelineLayout &&other) noexcept : device(other.device), layout(other.layout) {
        other.layout = VK_NULL_HANDLE;
    }
    ScopedPipelineLayout &operator=(ScopedPipelineLayout &&other) noexcept {
        if (this != &other) {
            if (layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(device, layout, nullptr);
            }
            device = other.device;
            layout = other.layout;
            other.layout = VK_NULL_HANDLE;
        }
        return *this;
    }
    ~ScopedPipelineLayout() {
        if (layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, layout, nullptr);
        }
    }
};

struct ScopedPipeline {
    VkDevice device{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
    ScopedPipeline() = default;
    ScopedPipeline(VkDevice device_in, VkPipeline pipeline_in) noexcept
        : device(device_in), pipeline(pipeline_in) {}
    ScopedPipeline(const ScopedPipeline &) = delete;
    ScopedPipeline &operator=(const ScopedPipeline &) = delete;
    ScopedPipeline(ScopedPipeline &&other) noexcept : device(other.device), pipeline(other.pipeline) {
        other.pipeline = VK_NULL_HANDLE;
    }
    ScopedPipeline &operator=(ScopedPipeline &&other) noexcept {
        if (this != &other) {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
            device = other.device;
            pipeline = other.pipeline;
            other.pipeline = VK_NULL_HANDLE;
        }
        return *this;
    }
    ~ScopedPipeline() {
        if (pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, pipeline, nullptr);
        }
    }
};

struct ScopedFence {
    VkDevice device{VK_NULL_HANDLE};
    VkFence fence{VK_NULL_HANDLE};
    ScopedFence() = default;
    ScopedFence(VkDevice device_in, VkFence fence_in) noexcept : device(device_in), fence(fence_in) {}
    ScopedFence(const ScopedFence &) = delete;
    ScopedFence &operator=(const ScopedFence &) = delete;
    ScopedFence(ScopedFence &&other) noexcept : device(other.device), fence(other.fence) {
        other.fence = VK_NULL_HANDLE;
    }
    ScopedFence &operator=(ScopedFence &&other) noexcept {
        if (this != &other) {
            if (fence != VK_NULL_HANDLE) {
                vkDestroyFence(device, fence, nullptr);
            }
            device = other.device;
            fence = other.fence;
            other.fence = VK_NULL_HANDLE;
        }
        return *this;
    }
    ~ScopedFence() {
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(device, fence, nullptr);
        }
    }
};

class VulkanDeviceBackend final : public DeviceBackend {
public:
    VulkanDeviceBackend() {
        instance_ = create_instance(true);
        pick_physical_device();
        create_device();
        create_command_pool();
    }

    VulkanDeviceBackend(const VulkanDeviceBackend &) = delete;
    VulkanDeviceBackend &operator=(const VulkanDeviceBackend &) = delete;

    ~VulkanDeviceBackend() override {
        if (device_ != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(device_);
            if (command_pool_ != VK_NULL_HANDLE) {
                vkDestroyCommandPool(device_, command_pool_, nullptr);
            }
            vkDestroyDevice(device_, nullptr);
        }
        if (instance_ != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_, nullptr);
        }
    }

    [[nodiscard]] Backend backend() const noexcept override {
        return Backend::vulkan;
    }

    [[nodiscard]] DeviceCapabilities capabilities() const override {
        auto result = DeviceCapabilities{};
        result.backend = Backend::vulkan;
        result.graphics_queue = true;
        result.compute_queue = false;
        result.copy_queue = false;
        result.ray_tracing = false;
        result.cuda_interop = false;
        result.bindless = false;
        result.native_debug = false;
        result.max_color_attachments = 1;
        result.max_frames_in_flight = 1;
        return result;
    }

    [[nodiscard]] std::shared_ptr<DeviceExtension> query_extension(ExtensionKind) override {
        return nullptr;
    }

    [[nodiscard]] std::shared_ptr<BufferHandle> create_buffer(const BufferDesc &desc) override {
        if (desc.size == 0) {
            throw_vk(Status::invalid_argument, "Buffers must have a non-zero size.");
        }
        return make_buffer(desc.size,
                           to_vk_usage(desc.usage),
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    [[nodiscard]] std::shared_ptr<TextureHandle> create_texture(const TextureDesc &desc) override {
        validate_texture_desc(desc);
        return make_image(desc.extent, to_vk_format(desc.format), to_vk_usage(desc.usage));
    }

    [[nodiscard]] std::shared_ptr<SamplerHandle> create_sampler(const SamplerDesc &desc) override {
        VkSamplerCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_info.magFilter = to_vk_filter(desc.mag_filter);
        create_info.minFilter = to_vk_filter(desc.min_filter);
        create_info.addressModeU = to_vk_address_mode(desc.address_u);
        create_info.addressModeV = to_vk_address_mode(desc.address_v);
        create_info.addressModeW = to_vk_address_mode(desc.address_w);
        create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        create_info.minLod = 0.0f;
        create_info.maxLod = VK_LOD_CLAMP_NONE;

        VkSampler sampler = VK_NULL_HANDLE;
        check(vkCreateSampler(device_, &create_info, nullptr, &sampler), "Failed to create Vulkan sampler.");
        return std::make_shared<VulkanSamplerHandle>(device_, sampler);
    }

    [[nodiscard]] std::shared_ptr<ShaderModuleHandle>
    create_shader_module(std::span<const std::uint32_t> spirv) override {
        if (spirv.empty()) {
            throw_vk(Status::invalid_argument, "Shader module SPIR-V must not be empty.");
        }

        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = spirv.size_bytes();
        create_info.pCode = spirv.data();

        VkShaderModule module = VK_NULL_HANDLE;
        check(vkCreateShaderModule(device_, &create_info, nullptr, &module), "Failed to create shader module.");
        return std::make_shared<VulkanShaderModuleHandle>(device_, module);
    }

    [[nodiscard]] std::shared_ptr<BindingLayoutHandle>
    create_binding_layout(const BindingLayoutDesc &) override {
        return std::make_shared<VulkanBindingLayoutHandle>();
    }

    [[nodiscard]] std::shared_ptr<BindingSetHandle> create_binding_set(const BindingSetDesc &) override {
        return std::make_shared<VulkanBindingSetHandle>();
    }

    [[nodiscard]] std::shared_ptr<GraphicsPipelineHandle>
    create_graphics_pipeline(const GraphicsPipelineDesc &) override {
        return std::make_shared<VulkanGraphicsPipelineHandle>();
    }

    [[nodiscard]] std::shared_ptr<GraphicsPipelineHandle>
    create_builtin_triangle_pipeline(Format color_format) override {
        if (color_format != Format::rgba8_unorm) {
            throw_vk(Status::invalid_argument, "Builtin triangle pipeline v0 only supports rgba8_unorm.");
        }
        return std::make_shared<VulkanGraphicsPipelineHandle>();
    }

    [[nodiscard]] std::shared_ptr<ComputePipelineHandle>
    create_compute_pipeline(const ComputePipelineDesc &) override {
        throw_vk(Status::unavailable, "Compute pipelines are reserved for a later RHI milestone.");
    }

    [[nodiscard]] std::shared_ptr<FramebufferHandle> create_framebuffer(const FramebufferDesc &) override {
        return std::make_shared<VulkanFramebufferHandle>();
    }

    [[nodiscard]] std::shared_ptr<CommandListHandle> create_command_list(QueueType type) override {
        if (type != QueueType::graphics) {
            throw_vk(Status::unavailable, "Vulkan v0 only supports graphics command lists.");
        }
        auto command_buffer = allocate_command_buffer();
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        check(vkBeginCommandBuffer(command_buffer, &begin_info), "Failed to begin empty command list.");
        check(vkEndCommandBuffer(command_buffer), "Failed to end empty command list.");
        return std::make_shared<VulkanCommandListHandle>(device_, command_pool_, command_buffer);
    }

    void transition_buffer(CommandListHandle &commands,
                           BufferHandle &buffer,
                           ResourceState,
                           ResourceState) override {
        if (dynamic_cast<VulkanCommandListHandle *>(&commands) == nullptr ||
            dynamic_cast<VulkanBufferHandle *>(&buffer) == nullptr) {
            throw_vk(Status::invalid_argument, "Buffer transition objects do not belong to the Vulkan backend.");
        }
    }

    void transition_texture(CommandListHandle &commands,
                            TextureHandle &texture,
                            ResourceState,
                            ResourceState) override {
        if (dynamic_cast<VulkanCommandListHandle *>(&commands) == nullptr ||
            dynamic_cast<VulkanImageHandle *>(&texture) == nullptr) {
            throw_vk(Status::invalid_argument, "Texture transition objects do not belong to the Vulkan backend.");
        }
    }

    void submit_and_wait(QueueType type, CommandListHandle &commands) override {
        if (type != QueueType::graphics) {
            throw_vk(Status::unavailable, "Vulkan v0 only supports graphics queue submission.");
        }
        auto *vk_commands = dynamic_cast<VulkanCommandListHandle *>(&commands);
        if (vk_commands == nullptr) {
            throw_vk(Status::invalid_argument, "Command list does not belong to the Vulkan backend.");
        }
        submit_command_buffer(vk_commands->command_buffer());
    }

    [[nodiscard]] std::vector<std::uint8_t>
    render_rgba8(const ImageDesc &target_desc, Color clear_color, bool draw_builtin_triangle) override {
        validate_image_desc(target_desc);

        const auto width = target_desc.extent.width;
        const auto height = target_desc.extent.height;
        const auto readback_size = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * 4u;

        auto color_image = make_image(target_desc.extent,
                                      to_vk_format(target_desc.format),
                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        auto readback_buffer = make_buffer(static_cast<std::size_t>(readback_size),
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        auto image_view = create_image_view(*color_image);
        auto render_pass = create_render_pass(to_vk_format(target_desc.format));
        auto framebuffer = create_framebuffer(render_pass.render_pass, image_view.view, target_desc.extent);
        auto pipeline_layout = create_pipeline_layout();
        auto pipeline = draw_builtin_triangle
                            ? create_triangle_pipeline(render_pass.render_pass, pipeline_layout.layout, target_desc.extent)
                            : ScopedPipeline{device_, VK_NULL_HANDLE};

        const auto command_buffer = allocate_command_buffer();
        auto commands = VulkanCommandListHandle{device_, command_pool_, command_buffer};

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        check(vkBeginCommandBuffer(command_buffer, &begin_info), "Failed to begin triangle command buffer.");

        VkClearValue clear_value{};
        clear_value.color = {{clear_color.r, clear_color.g, clear_color.b, clear_color.a}};

        VkRenderPassBeginInfo pass_begin{};
        pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        pass_begin.renderPass = render_pass.render_pass;
        pass_begin.framebuffer = framebuffer.framebuffer;
        pass_begin.renderArea.offset = {0, 0};
        pass_begin.renderArea.extent = {width, height};
        pass_begin.clearValueCount = 1;
        pass_begin.pClearValues = &clear_value;

        vkCmdBeginRenderPass(command_buffer, &pass_begin, VK_SUBPASS_CONTENTS_INLINE);
        if (draw_builtin_triangle) {
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
            vkCmdDraw(command_buffer, 3, 1, 0, 0);
        }
        vkCmdEndRenderPass(command_buffer);

        VkBufferImageCopy copy_region{};
        copy_region.bufferOffset = 0;
        copy_region.bufferRowLength = 0;
        copy_region.bufferImageHeight = 0;
        copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.imageSubresource.mipLevel = 0;
        copy_region.imageSubresource.baseArrayLayer = 0;
        copy_region.imageSubresource.layerCount = 1;
        copy_region.imageOffset = {0, 0, 0};
        copy_region.imageExtent = {width, height, 1};
        vkCmdCopyImageToBuffer(command_buffer,
                               color_image->image(),
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               readback_buffer->buffer(),
                               1,
                               &copy_region);

        check(vkEndCommandBuffer(command_buffer), "Failed to end triangle command buffer.");
        submit_command_buffer(command_buffer);

        auto pixels = std::vector<std::uint8_t>(static_cast<std::size_t>(readback_size));
        void *mapped = nullptr;
        check(vkMapMemory(device_, readback_buffer->memory(), 0, readback_size, 0, &mapped),
              "Failed to map readback buffer.");
        std::memcpy(pixels.data(), mapped, pixels.size());
        vkUnmapMemory(device_, readback_buffer->memory());
        return pixels;
    }

private:
    void pick_physical_device() {
        auto count = std::uint32_t{0};
        check(vkEnumeratePhysicalDevices(instance_, &count, nullptr), "Failed to enumerate Vulkan devices.");
        if (count == 0) {
            throw_vk(Status::unavailable, "No Vulkan physical device is available.");
        }

        auto devices = std::vector<VkPhysicalDevice>(count);
        check(vkEnumeratePhysicalDevices(instance_, &count, devices.data()), "Failed to list Vulkan devices.");
        for (auto device : devices) {
            const auto family = find_graphics_queue_family(device);
            if (family != std::numeric_limits<std::uint32_t>::max() &&
                physical_device_supports_rgba8_target(device)) {
                physical_device_ = device;
                graphics_queue_family_ = family;
                return;
            }
        }

        throw_vk(Status::unavailable, "No Vulkan device supports graphics and RGBA8 render targets.");
    }

    void create_device() {
        constexpr float queue_priority = 1.0f;

        VkDeviceQueueCreateInfo queue_info{};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = graphics_queue_family_;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &queue_priority;

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = 1;
        create_info.pQueueCreateInfos = &queue_info;

        check(vkCreateDevice(physical_device_, &create_info, nullptr, &device_), "Failed to create Vulkan device.");
        vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);
    }

    void create_command_pool() {
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = graphics_queue_family_;
        check(vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_), "Failed to create command pool.");
    }

    [[nodiscard]] std::uint32_t find_memory_type(std::uint32_t type_bits,
                                                 VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memory_properties{};
        vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties);
        for (std::uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
            const auto supported = (type_bits & (1u << i)) != 0u;
            const auto matches =
                (memory_properties.memoryTypes[i].propertyFlags & properties) == properties;
            if (supported && matches) {
                return i;
            }
        }
        throw_vk(Status::runtime_error, "No compatible Vulkan memory type was found.");
    }

    [[nodiscard]] std::shared_ptr<VulkanBufferHandle> make_buffer(std::size_t size,
                                                                  VkBufferUsageFlags usage,
                                                                  VkMemoryPropertyFlags memory_flags) {
        VkBufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = static_cast<VkDeviceSize>(size);
        create_info.usage = usage;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer buffer = VK_NULL_HANDLE;
        check(vkCreateBuffer(device_, &create_info, nullptr, &buffer), "Failed to create Vulkan buffer.");

        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device_, buffer, &requirements);

        VkMemoryAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = requirements.size;
        allocate_info.memoryTypeIndex = find_memory_type(requirements.memoryTypeBits, memory_flags);

        VkDeviceMemory memory = VK_NULL_HANDLE;
        const auto allocate_result = vkAllocateMemory(device_, &allocate_info, nullptr, &memory);
        if (allocate_result != VK_SUCCESS) {
            vkDestroyBuffer(device_, buffer, nullptr);
            check(allocate_result, "Failed to allocate Vulkan buffer memory.");
        }

        const auto bind_result = vkBindBufferMemory(device_, buffer, memory, 0);
        if (bind_result != VK_SUCCESS) {
            vkFreeMemory(device_, memory, nullptr);
            vkDestroyBuffer(device_, buffer, nullptr);
            check(bind_result, "Failed to bind Vulkan buffer memory.");
        }

        return std::make_shared<VulkanBufferHandle>(device_, buffer, memory, create_info.size);
    }

    [[nodiscard]] std::shared_ptr<VulkanImageHandle> make_image(Extent2D extent,
                                                                VkFormat format,
                                                                VkImageUsageFlags usage) {
        VkImageCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.format = format;
        create_info.extent = {extent.width, extent.height, 1};
        create_info.mipLevels = 1;
        create_info.arrayLayers = 1;
        create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.usage = usage;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImage image = VK_NULL_HANDLE;
        check(vkCreateImage(device_, &create_info, nullptr, &image), "Failed to create Vulkan image.");

        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(device_, image, &requirements);

        VkMemoryAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = requirements.size;
        allocate_info.memoryTypeIndex =
            find_memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkDeviceMemory memory = VK_NULL_HANDLE;
        const auto allocate_result = vkAllocateMemory(device_, &allocate_info, nullptr, &memory);
        if (allocate_result != VK_SUCCESS) {
            vkDestroyImage(device_, image, nullptr);
            check(allocate_result, "Failed to allocate Vulkan image memory.");
        }

        const auto bind_result = vkBindImageMemory(device_, image, memory, 0);
        if (bind_result != VK_SUCCESS) {
            vkFreeMemory(device_, memory, nullptr);
            vkDestroyImage(device_, image, nullptr);
            check(bind_result, "Failed to bind Vulkan image memory.");
        }

        return std::make_shared<VulkanImageHandle>(device_, image, memory, extent, format);
    }

    [[nodiscard]] ScopedImageView create_image_view(const VulkanImageHandle &image) {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = image.image();
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = image.format();
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        ScopedImageView result{device_, VK_NULL_HANDLE};
        check(vkCreateImageView(device_, &create_info, nullptr, &result.view), "Failed to create image view.");
        return result;
    }

    [[nodiscard]] ScopedRenderPass create_render_pass(VkFormat format) {
        VkAttachmentDescription attachment{};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_ref;

        VkSubpassDependency dependencies[2]{};
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        VkRenderPassCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &attachment;
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        create_info.dependencyCount = 2;
        create_info.pDependencies = dependencies;

        ScopedRenderPass result{device_, VK_NULL_HANDLE};
        check(vkCreateRenderPass(device_, &create_info, nullptr, &result.render_pass),
              "Failed to create render pass.");
        return result;
    }

    [[nodiscard]] ScopedFramebuffer create_framebuffer(VkRenderPass render_pass,
                                                       VkImageView image_view,
                                                       Extent2D extent) {
        VkFramebufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = render_pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &image_view;
        create_info.width = extent.width;
        create_info.height = extent.height;
        create_info.layers = 1;

        ScopedFramebuffer result{device_, VK_NULL_HANDLE};
        check(vkCreateFramebuffer(device_, &create_info, nullptr, &result.framebuffer),
              "Failed to create framebuffer.");
        return result;
    }

    [[nodiscard]] ScopedPipelineLayout create_pipeline_layout() {
        VkPipelineLayoutCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        ScopedPipelineLayout result{device_, VK_NULL_HANDLE};
        check(vkCreatePipelineLayout(device_, &create_info, nullptr, &result.layout),
              "Failed to create pipeline layout.");
        return result;
    }

    [[nodiscard]] ScopedPipeline create_triangle_pipeline(VkRenderPass render_pass,
                                                          VkPipelineLayout pipeline_layout,
                                                          Extent2D extent) {
        auto vertex_shader = create_shader_module(shaders::builtin_triangle_vert);
        auto fragment_shader = create_shader_module(shaders::builtin_triangle_frag);
        const auto *vk_vertex_shader = dynamic_cast<VulkanShaderModuleHandle *>(vertex_shader.get());
        const auto *vk_fragment_shader = dynamic_cast<VulkanShaderModuleHandle *>(fragment_shader.get());

        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vk_vertex_shader->module();
        stages[0].pName = "main";
        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = vk_fragment_shader->module();
        stages[1].pName = "main";

        VkPipelineVertexInputStateCreateInfo vertex_input{};
        vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {extent.width, extent.height};

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterization{};
        rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization.depthClampEnable = VK_FALSE;
        rasterization.rasterizerDiscardEnable = VK_FALSE;
        rasterization.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization.cullMode = VK_CULL_MODE_NONE;
        rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization.depthBiasEnable = VK_FALSE;
        rasterization.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo color_blend{};
        color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend.attachmentCount = 1;
        color_blend.pAttachments = &color_blend_attachment;

        VkGraphicsPipelineCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.stageCount = 2;
        create_info.pStages = stages;
        create_info.pVertexInputState = &vertex_input;
        create_info.pInputAssemblyState = &input_assembly;
        create_info.pViewportState = &viewport_state;
        create_info.pRasterizationState = &rasterization;
        create_info.pMultisampleState = &multisample;
        create_info.pColorBlendState = &color_blend;
        create_info.layout = pipeline_layout;
        create_info.renderPass = render_pass;
        create_info.subpass = 0;

        ScopedPipeline result{device_, VK_NULL_HANDLE};
        check(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &create_info, nullptr, &result.pipeline),
              "Failed to create builtin triangle pipeline.");
        return result;
    }

    [[nodiscard]] VkCommandBuffer allocate_command_buffer() {
        VkCommandBufferAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = command_pool_;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        check(vkAllocateCommandBuffers(device_, &allocate_info, &command_buffer),
              "Failed to allocate command buffer.");
        return command_buffer;
    }

    void submit_command_buffer(VkCommandBuffer command_buffer) {
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        ScopedFence fence{device_, VK_NULL_HANDLE};
        check(vkCreateFence(device_, &fence_info, nullptr, &fence.fence), "Failed to create fence.");

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        check(vkQueueSubmit(graphics_queue_, 1, &submit_info, fence.fence), "Failed to submit command buffer.");
        check(vkWaitForFences(device_, 1, &fence.fence, VK_TRUE, UINT64_MAX), "Failed to wait for command buffer.");
    }

    static void validate_texture_fields(Extent2D extent, Format format) {
        if (extent.width == 0 || extent.height == 0) {
            throw_vk(Status::invalid_argument, "Textures must have non-zero dimensions.");
        }
        if (format != Format::rgba8_unorm) {
            throw_vk(Status::invalid_argument, "Vulkan v0 only supports rgba8_unorm textures.");
        }
    }

    static void validate_texture_desc(const TextureDesc &desc) {
        validate_texture_fields(desc.extent, desc.format);
    }

    static void validate_image_desc(const ImageDesc &desc) {
        if (desc.extent.width == 0 || desc.extent.height == 0) {
            throw_vk(Status::invalid_argument, "Images must have non-zero dimensions.");
        }
        if (desc.format != Format::rgba8_unorm) {
            throw_vk(Status::invalid_argument, "Vulkan v0 only supports rgba8_unorm images.");
        }
    }

    VkInstance instance_{VK_NULL_HANDLE};
    VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE};
    VkQueue graphics_queue_{VK_NULL_HANDLE};
    VkCommandPool command_pool_{VK_NULL_HANDLE};
    std::uint32_t graphics_queue_family_{std::numeric_limits<std::uint32_t>::max()};
};

} // namespace

bool vulkan_backend_available() noexcept {
    VkInstance instance = VK_NULL_HANDLE;
    try {
        instance = create_instance(false);

        auto count = std::uint32_t{0};
        if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS || count == 0) {
            vkDestroyInstance(instance, nullptr);
            return false;
        }

        auto devices = std::vector<VkPhysicalDevice>(count);
        if (vkEnumeratePhysicalDevices(instance, &count, devices.data()) != VK_SUCCESS) {
            vkDestroyInstance(instance, nullptr);
            return false;
        }

        const auto found = std::any_of(devices.begin(), devices.end(), [](auto device) {
            return find_graphics_queue_family(device) != std::numeric_limits<std::uint32_t>::max() &&
                   physical_device_supports_rgba8_target(device);
        });
        vkDestroyInstance(instance, nullptr);
        return found;
    } catch (...) {
        if (instance != VK_NULL_HANDLE) {
            vkDestroyInstance(instance, nullptr);
        }
        return false;
    }
}

std::shared_ptr<DeviceBackend> create_vulkan_backend() {
    if (!vulkan_backend_available()) {
        throw Error(Status::unavailable, "Vulkan backend is not available on this machine.");
    }
    return std::make_shared<VulkanDeviceBackend>();
}

} // namespace helicon::detail
