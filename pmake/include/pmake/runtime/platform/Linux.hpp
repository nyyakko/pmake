#pragma once

#include <cassert>
#include <filesystem>

inline std::filesystem::path const& get_program_root_dir()
{
    std::filesystem::path static programPath {};
    assert(false && "UNIMPLEMENTED");
    return programPath;
}
