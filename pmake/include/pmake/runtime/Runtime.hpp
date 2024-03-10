#pragma once

#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
    #include "platform/Windows.hpp"
#elif defined(__linux__)
    #include "platform/Linux.hpp"
#endif

namespace runtime {

inline std::filesystem::path const& get_program_root_dir()
{
    return detail::get_program_root_dir();
}

}
