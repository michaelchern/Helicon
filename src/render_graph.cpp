#include "rhi_internal.hpp"

#include <limits>
#include <utility>

namespace helicon {

RenderGraphImage::RenderGraphImage(std::uint32_t index) noexcept : index_(index) {}

bool RenderGraphImage::valid() const noexcept {
    return index_ != std::numeric_limits<std::uint32_t>::max();
}

std::uint32_t RenderGraphImage::index() const noexcept {
    return index_;
}

RenderGraphPass::RenderGraphPass(std::string name) : name_(std::move(name)) {}

void RenderGraphPass::write_color(RenderGraphImage image) {
    write_color(image, Color{});
}

void RenderGraphPass::write_color(RenderGraphImage image, Color clear_color) {
    if (!image.valid()) {
        throw Error(Status::invalid_argument, "Render pass color output uses an invalid image handle.");
    }
    color_output_ = image;
    clear_color_ = clear_color;
    has_color_output_ = true;
}

void RenderGraphPass::draw_builtin_triangle() {
    draws_builtin_triangle_ = true;
}

RenderGraphImage RenderGraph::create_image(const ImageDesc &desc) {
    if (desc.extent.width == 0 || desc.extent.height == 0) {
        throw Error(Status::invalid_argument, "Render graph images must have non-zero dimensions.");
    }
    if (desc.format != Format::rgba8_unorm) {
        throw Error(Status::invalid_argument, "Render graph v0 only supports rgba8_unorm images.");
    }

    const auto index = static_cast<std::uint32_t>(images_.size());
    images_.push_back(ImageRecord{desc, {}});
    return RenderGraphImage{index};
}

RenderGraphPass &RenderGraph::add_pass(std::string name) {
    if (name.empty()) {
        throw Error(Status::invalid_argument, "Render graph pass names must not be empty.");
    }
    passes_.push_back(RenderGraphPass{std::move(name)});
    return passes_.back();
}

void RenderGraph::execute(const Device &device) {
    if (!device.backend_) {
        throw Error(Status::invalid_argument, "Cannot execute a render graph with an invalid device.");
    }
    if (passes_.empty()) {
        throw Error(Status::invalid_argument, "Render graph has no passes to execute.");
    }

    for (auto &pass : passes_) {
        if (!pass.has_color_output_) {
            throw Error(Status::invalid_argument, "Render graph v0 requires every pass to write one color output.");
        }
        const auto image_index = pass.color_output_.index();
        if (image_index >= images_.size()) {
            throw Error(Status::invalid_argument, "Render pass references an image that does not exist.");
        }

        auto &record = images_[image_index];
        record.last_readback =
            device.backend_->render_rgba8(record.desc, pass.clear_color_, pass.draws_builtin_triangle_);
    }
}

std::vector<std::uint8_t> RenderGraph::read_image_rgba8(RenderGraphImage image) const {
    if (!image.valid() || image.index() >= images_.size()) {
        throw Error(Status::invalid_argument, "Cannot read an invalid render graph image.");
    }

    const auto &readback = images_[image.index()].last_readback;
    if (readback.empty()) {
        throw Error(Status::invalid_argument, "Render graph image has not been executed yet.");
    }
    return readback;
}

} // namespace helicon
