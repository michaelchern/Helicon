#include "helicon/helicon.hpp"

#include <iostream>

int main() {
    if (helicon::version_string().empty()) {
        std::cerr << "Expected a Helicon version string.\n";
        return 1;
    }

    if (helicon::status_string(helicon::Status::success) != "success") {
        std::cerr << "Expected success status string.\n";
        return 1;
    }

    const auto available = helicon::Context::backend_available(helicon::Backend::vulkan);
    std::cout << "Vulkan backend available: " << (available ? "yes" : "no") << '\n';
    std::cout << "Helicon C++ smoke test passed.\n";
    return 0;
}
