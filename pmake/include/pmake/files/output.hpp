#pragma once

#include <string_view>
#include <filesystem>
#include <filesystem>

namespace pmake {

void create_from_template(std::filesystem::path templatePath, std::string_view projectName, std::string_view projectLanguage, std::string_view projectStandard);

}
