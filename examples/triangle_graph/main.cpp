#include "helicon/helicon.hpp"

#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

namespace {

void write_ppm(const char *path,
               const std::vector<std::uint8_t> &pixels,
               std::uint32_t width,
               std::uint32_t height) {
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << width << ' ' << height << "\n255\n";
    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            const auto offset = (static_cast<std::size_t>(y) * width + x) * 4u;
            out.put(static_cast<char>(pixels[offset]));
            out.put(static_cast<char>(pixels[offset + 1]));
            out.put(static_cast<char>(pixels[offset + 2]));
        }
    }
}

} // namespace

int main() {
    try {
        if (!helicon::Context::backend_available(helicon::Backend::vulkan)) {
            std::cout << "Vulkan backend is unavailable on this machine.\n";
            return 0;
        }

        constexpr auto width = std::uint32_t{256};
        constexpr auto height = std::uint32_t{256};

        helicon::Context context;
        auto device = context.create_device();

        helicon::RenderGraph graph;
        auto target = graph.create_image({{width, height},
                                          helicon::Format::rgba8_unorm,
                                          helicon::ImageUsage::color_attachment |
                                              helicon::ImageUsage::transfer_src,
                                          {0.0f, 0.0f, 0.0f, 1.0f}});
        auto &pass = graph.add_pass("triangle");
        pass.write_color(target, {0.0f, 0.0f, 0.0f, 1.0f});
        pass.draw_builtin_triangle();
        graph.execute(device);

        const auto pixels = graph.read_image_rgba8(target);
        write_ppm("triangle_graph.ppm", pixels, width, height);
        std::cout << "Wrote triangle_graph.ppm\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
