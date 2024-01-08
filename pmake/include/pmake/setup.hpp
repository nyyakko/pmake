#pragma once

#include <string>
#include <filesystem>

#include <cxxopts.hpp>

namespace pmake {

std::string setup_language(cxxopts::ParseResult const& parsedOptions);
std::string setup_language_standard(cxxopts::ParseResult const& parsedOptions, std::string const& projectLanguage);
std::filesystem::path setup_template_path(cxxopts::ParseResult const& parsedOptions, std::string const& projectLanguage, std::string const& projectKind);

}
