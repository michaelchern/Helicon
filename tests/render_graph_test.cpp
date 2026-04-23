#include "helicon/helicon.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <vector>

namespace {

bool rgb_non_black(const std::vector<std::uint8_t> &pixels, std::uint32_t width, std::uint32_t x, std::uint32_t y) {
    const auto offset = (static_cast<std::size_t>(y) * width + x) * 4u;
    return pixels[offset] > 8 || pixels[offset + 1] > 8 || pixels[offset + 2] > 8;
}

bool rgb_black(const std::vector<std::uint8_t> &pixels, std::uint32_t width, std::uint32_t x, std::uint32_t y) {
    const auto offset = (static_cast<std::size_t>(y) * width + x) * 4u;
    return pixels[offset] < 8 && pixels[offset + 1] < 8 && pixels[offset + 2] < 8;
}

} // namespace

int main() {
    try {
        if (!helicon::Context::backend_available(helicon::Backend::vulkan)) {
            std::cout << "Vulkan backend unavailable; skipping GPU render graph regression.\n";
            return 0;
        }

        helicon::Context context;
        auto device = context.create_device();
        auto buffer = device.create_buffer({1024, helicon::BufferUsage::transfer_dst});
        auto image = device.create_image({{16, 16},
                                          helicon::Format::rgba8_unorm,
                                          helicon::ImageUsage::color_attachment |
                                              helicon::ImageUsage::transfer_src,
                                          {0.0f, 0.0f, 0.0f, 1.0f}});
        auto command_list = device.create_command_list();
        auto sampler = device.create_sampler();
        auto binding_layout = device.create_binding_layout(
            {0,
             {{helicon::ResourceType::texture_srv, 0, 1, helicon::ShaderStage::fragment, 0},
              {helicon::ResourceType::sampler, 1, 1, helicon::ShaderStage::fragment, 0}},
             "texture_and_sampler"});

        helicon::BindingSetDesc binding_set_desc;
        binding_set_desc.layout = binding_layout;
        binding_set_desc.items.push_back(
            {helicon::ResourceType::texture_srv, 0, 0, {}, image, {}});
        binding_set_desc.items.push_back(
            {helicon::ResourceType::sampler, 1, 0, {}, {}, sampler});
        auto binding_set = device.create_binding_set(binding_set_desc);

        helicon::FramebufferDesc framebuffer_desc;
        framebuffer_desc.color_attachments.push_back(image);
        auto framebuffer = device.create_framebuffer(framebuffer_desc);

        const auto capabilities = device.capabilities();
        if (!capabilities.graphics_queue || !device.supports(helicon::DeviceFeature::graphics)) {
            std::cerr << "Expected Vulkan device to report graphics support.\n";
            return 1;
        }
        if (device.query_extension(helicon::ExtensionKind::ray_tracing) != nullptr) {
            std::cerr << "Expected ray tracing extension to be unavailable in v0.\n";
            return 1;
        }

        command_list.transition_buffer(buffer, helicon::ResourceState::common, helicon::ResourceState::transfer_dst);
        command_list.transition_texture(image, helicon::ResourceState::undefined, helicon::ResourceState::render_target);

        if (!device.valid() || !buffer.valid() || !image.valid() || !command_list.valid() ||
            !sampler.valid() || !binding_layout.valid() || !binding_set.valid() || !framebuffer.valid()) {
            std::cerr << "Expected Vulkan RHI handles to be valid.\n";
            return 1;
        }

        device.graphics_queue().submit_and_wait(command_list);

        helicon::RenderGraph clear_graph;
        auto clear_target = clear_graph.create_image({{32, 32},
                                                      helicon::Format::rgba8_unorm,
                                                      helicon::ImageUsage::color_attachment |
                                                          helicon::ImageUsage::transfer_src,
                                                      {0.0f, 0.0f, 0.0f, 1.0f}});
        auto &clear_pass = clear_graph.add_pass("clear");
        clear_pass.write_color(clear_target, {0.0f, 0.0f, 0.0f, 1.0f});
        clear_graph.execute(device);
        const auto clear_pixels = clear_graph.read_image_rgba8(clear_target);
        if (clear_pixels.size() != 32u * 32u * 4u || !rgb_black(clear_pixels, 32, 16, 16)) {
            std::cerr << "Expected clear-only render graph pass to produce black RGBA8 pixels.\n";
            return 1;
        }

        helicon::RenderGraph triangle_graph;
        auto triangle_target = triangle_graph.create_image({{64, 64},
                                                            helicon::Format::rgba8_unorm,
                                                            helicon::ImageUsage::color_attachment |
                                                                helicon::ImageUsage::transfer_src,
                                                            {0.0f, 0.0f, 0.0f, 1.0f}});
        auto &triangle_pass = triangle_graph.add_pass("triangle");
        triangle_pass.write_color(triangle_target, {0.0f, 0.0f, 0.0f, 1.0f});
        triangle_pass.draw_builtin_triangle();
        triangle_graph.execute(device);

        const auto triangle_pixels = triangle_graph.read_image_rgba8(triangle_target);
        if (triangle_pixels.size() != 64u * 64u * 4u) {
            std::cerr << "Expected triangle readback to contain 64x64 RGBA8 pixels.\n";
            return 1;
        }
        if (!rgb_non_black(triangle_pixels, 64, 32, 32)) {
            std::cerr << "Expected triangle center pixel to be non-black.\n";
            return 1;
        }
        if (!rgb_black(triangle_pixels, 64, 2, 2)) {
            std::cerr << "Expected triangle corner pixel to remain clear black.\n";
            return 1;
        }

        std::cout << "Helicon render graph tests passed.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
