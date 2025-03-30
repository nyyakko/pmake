#pragma once

#include <fmt/format.h>

#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#include "platform/Windows.hpp"
#elif defined(__linux__)
#include "platform/Linux.hpp"
#endif

namespace pmake::runtime
{

inline std::filesystem::path const& get_program_root_dir()
{
    return detail::get_program_root_dir();
}

inline auto get_program_dir()
{
    return get_program_root_dir().string();
}

inline auto get_templates_dir()
{
    return fmt::format("{}/assets/templates", get_program_dir());
}

inline auto get_features_dir()
{
    return fmt::format("{}/features", get_templates_dir());
}

} // namespace pmake::runtime
