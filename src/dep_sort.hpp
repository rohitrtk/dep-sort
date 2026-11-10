#pragma once

#include <array>
#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>

namespace dep_sort {
using Manifest = nlohmann::ordered_json;
using Result   = std::optional<Manifest>;

constexpr std::array<std::string_view, 4> DEPENDENCY_KEYS = {
    std::string_view("dependencies"),
    std::string_view("devDependencies"),
    std::string_view("optionalDependencies"),
    std::string_view("peerDependencies"),
};

Result ReadManifest(std::string_view path);
Result ProcessManifest(std::string_view path);

void SortManifest(Manifest& manifest);
void WriteManifest(std::string_view path, const Manifest& manifest);
} // namespace dep_sort