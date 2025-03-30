#pragma once

#include <Windows.h>

#include <array>
#include <filesystem>
#include <print>

namespace pmake::runtime::detail
{

inline std::filesystem::path const& get_program_root_dir()
{
    std::filesystem::path static programPath {};

    if (!programPath.empty())
    {
        return programPath;
    }

    std::array<char, MAX_PATH> buffer {};
    GetModuleFileName(nullptr, buffer.data(), buffer.size());

    programPath = buffer.data();
    programPath = programPath.parent_path();

    return programPath;
}

} // namespace pmake::runtime::detail
