#pragma once

#include <string>
#include <filesystem>

#include <cxxopts.hpp>

namespace pmake {

std::string setup_language(cxxopts::ParseResult const& parsedOptions);
std::string setup_kind(cxxopts::ParseResult const& parsedOptions);
std::string setup_language_standard(cxxopts::ParseResult const& parsedOptions);
std::filesystem::path setup_template_path(cxxopts::ParseResult const& parsedOptions);

}
