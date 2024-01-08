#include "setup.hpp"

#include <print>
#include <unordered_map>
#include <ranges>

using namespace std::literals;

std::string pmake::setup_language(cxxopts::ParseResult const& parsedOptions)
{
    std::unordered_map<std::string_view, std::string> const availableLanguages
    {
        { "c++"sv, "CXX"s },
        { "c"sv,   "C"s   }
    };

    auto language = "CXX"s;

    if (parsedOptions.count("language"))
    {
        auto const selectedLanguage = parsedOptions["language"].as<std::string>();

        if (!availableLanguages.contains(std::ranges::to<std::string>(selectedLanguage | std::views::transform(::tolower))))
        {
            std::println("The language \"{}\" isn't supported.", selectedLanguage);
            std::exit(EXIT_FAILURE);
        }

        language = availableLanguages.at(selectedLanguage);
    }

    return language;
}

std::string pmake::setup_language_standard(cxxopts::ParseResult const& parsedOptions, std::string const& projectLanguage)
{
    std::unordered_map<std::string_view, std::vector<std::string>> const availableStandards
    {
        { "CXX"sv, { "23"s, "20"s, "17"s, "14"s, "11"s } },
        { "C"sv,   { "23"s, "17"s, "11"s, "99"s, } }
    };

    auto standard = availableStandards.at(projectLanguage).front();

    if (parsedOptions.count("standard"))
    {
        auto const selectedStandard = parsedOptions["standard"].as<std::string>();

        if (selectedStandard == "latest") { return standard; }

        if (!std::ranges::contains(availableStandards.at(projectLanguage), selectedStandard))
        {
            std::println("The standard \"{}\" isn't for the language \"{}\".", selectedStandard, projectLanguage);
            std::exit(EXIT_FAILURE);
        }

        standard = selectedStandard;
    }

    return standard;
}

std::filesystem::path pmake::setup_template_path(cxxopts::ParseResult const& parsedOptions, std::string const& projectLanguage, std::string const& projectKind)
{
    std::filesystem::path path { "pmake-templates\\"s + projectLanguage + "\\" + projectKind };

    if (parsedOptions.count("static")) { path += "\\static"; }
    else if (parsedOptions.count("dynamic")) { path += "\\dynamic"; }
    else if (parsedOptions.count("header-only")){ path += "\\header-only"; }
    else if (parsedOptions.count("console")) { path += "\\console"; }
    else if (parsedOptions.count("graphical")) { path += "\\graphical"; }
    else
    {
        path += (projectKind == "executable") ? "\\console" : "\\static";
    }

    if (!std::filesystem::exists(path))
    {
        std::println("The project template \"{}\" doesn't exist.", path.string());
        std::exit(EXIT_FAILURE);
    }

    return path;
}

