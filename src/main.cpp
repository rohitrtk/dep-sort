#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <future>
#include <print>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "dep_sort.hpp"

namespace {
enum class Mode
{
    Check,
    Write
};

dep_sort::Result ProcessAll(const std::vector<std::string_view>& paths)
{
    if (paths.size() == 1) {
        return dep_sort::ProcessManifest(paths.front());
    }

    const auto limit  = std::max(1u, std::thread::hardware_concurrency());
    auto       output = dep_sort::Manifest::array();
    std::vector<std::future<dep_sort::Result>> jobs;

    for (std::size_t next = 0; next < paths.size();) {
        jobs.clear();
        while (next < paths.size() && jobs.size() < limit) {
            jobs.push_back(
                std::async(std::launch::async, dep_sort::ProcessManifest, paths[next++]));
        }

        for (auto& job : jobs) {
            auto manifest = job.get();
            if (!manifest) {
                return std::nullopt;
            }
            output.push_back(std::move(*manifest));
        }
    }

    return output;
}
} // namespace

int main(int argc, char* argv[])
{
    try {
        std::optional<Mode>           mode;
        std::vector<std::string_view> paths;
        bool                          parse_flags = true;

        for (int i = 1; i < argc; ++i) {
            const std::string_view arg = argv[i];

            if (parse_flags && arg == "--") {
                parse_flags = false;
            } else if (parse_flags && (arg == "--check" || arg == "--write")) {
                const auto requested = arg == "--write" ? Mode::Write : Mode::Check;
                if (mode && *mode != requested) {
                    throw std::invalid_argument("Use either --check or --write.");
                }
                mode = requested;
            } else if (parse_flags && arg.starts_with('-')) {
                throw std::invalid_argument("Unknown option. Usage: dep-sort "
                                            "[--check | --write] [--] [manifest ...]");
            } else {
                paths.push_back(arg);
            }
        }

        if (paths.empty()) {
            paths.push_back("package.json");
        }

        const auto output = ProcessAll(paths);
        if (!output) {
            return 1;
        }

        if (mode.value_or(Mode::Check) == Mode::Check) {
            std::println("{}", output->dump(2));
        } else if (paths.size() == 1) {
            dep_sort::WriteManifest(paths.front(), *output);
        } else {
            for (std::size_t i = 0; i < paths.size(); ++i) {
                dep_sort::WriteManifest(paths[i], (*output)[i]);
            }
        }

        return 0;
    } catch (const std::exception& error) {
        std::println(stderr, "ERROR: {}", error.what());
        return 1;
    }
}
