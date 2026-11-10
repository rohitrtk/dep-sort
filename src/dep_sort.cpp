#include "dep_sort.hpp"

#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <print>
#include <stdexcept>
#include <string>
#include <system_error>

namespace dep_sort {
Result ReadManifest(std::string_view path)
{
    std::ifstream input(std::filesystem::path(path), std::ios::binary);
    if (!input) {
        std::println(stderr, "ERROR: Could not open '{}' for reading.", path);
        return std::nullopt;
    }

    try {
        const auto manifest = nlohmann::ordered_json::parse(input);
        if (!manifest.is_object()) {
            std::println(stderr, "ERROR: Expected a JSON object in '{}'", path);
            return std::nullopt;
        }

        return manifest;
    } catch (const nlohmann::ordered_json::exception& error) {
        std::println(stderr, "ERROR: Could not read JSON from '{}: {}'", path, error.what());
        return std::nullopt;
    }
}

void SortManifest(Manifest& manifest)
{
    for (const auto& target : DEPENDENCY_KEYS) {
        if (auto deps = manifest.find(target); deps != manifest.end() && deps->is_object()) {
            const nlohmann::json sorted = *deps;
            *deps                       = nlohmann::ordered_json(sorted);
        }
    }
}

Result ProcessManifest(std::string_view path)
{
    auto manifest = dep_sort::ReadManifest(path);
    if (manifest) {
        SortManifest(*manifest);
    }

    return manifest;
}

void WriteManifest(std::string_view path, const Manifest& manifest)
{
    namespace fs = std::filesystem;

    const auto target   = fs::canonical(fs::path{path});
    const auto contents = manifest.dump(2) + '\n';

    auto staging     = target;
    staging         += ".dep-sort.tmp";
    const auto temp  = staging / "manifest.json";

    if (!fs::create_directory(staging)) {
        throw std::runtime_error(
            std::format("Staging directory already exists for '{}'", std::string{path}));
    }

    try {
        std::ofstream output;
        output.exceptions(std::ios::failbit | std::ios::badbit);
        output.open(temp, std::ios::binary);
        output << contents;
        output.close();
        fs::rename(temp, target);
    } catch (...) {
        std::error_code ignored;
        fs::remove(temp, ignored);
        fs::remove(staging, ignored);

        throw;
    }

    std::error_code ignored;
    fs::remove(staging, ignored);
}
} // namespace dep_sort