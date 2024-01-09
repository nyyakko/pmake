#include "setup.hpp"

#include <print>
#include <unordered_map>
#include <ranges>

using namespace std::literals;

std::string pmake::setup_language(cxxopts::ParseResult const& parsedOptions)
{
    // TODO: perhaps the user could provide these?
    std::unordered_map<std::string_view, std::string> const availableLanguages
    {
        { "c++"sv, "CXX"s },
        { "c"sv,   "C"s   }
    };

    auto language = availableLanguages.at(parsedOptions["language"].as<std::string>());

    if (parsedOptions.count("language"))
    {
        auto const selectedLanguage = parsedOptions["language"].as<std::string>();

        if (!availableLanguages.contains(std::ranges::to<std::string>(selectedLanguage)))
        {
            std::println("The language \"{}\" isn't supported.", selectedLanguage);
            std::exit(EXIT_FAILURE);
        }

        language = availableLanguages.at(selectedLanguage);
    }

    return language;
}

std::string pmake::setup_kind(cxxopts::ParseResult const& parsedOptions)
{
    std::unordered_map<std::string_view, std::vector<std::string>> const availableKinds
    {
        { "c++"sv, { "executable"s, "library"s } },
        { "c"sv,   { "executable"s, "library"s } }
    };

    auto const language = parsedOptions["language"].as<std::string>();
    auto kind           = availableKinds.at(language).front();

    if (parsedOptions.count("kind"))
    {
        auto const selectedKind = parsedOptions["kind"].as<std::string>();

        if (!std::ranges::contains(availableKinds.at(language), selectedKind))
        {
            std::println("The kind \"{}\" for the language \"{}\" isn't available.", selectedKind, parsedOptions["language"].as<std::string>());
            std::exit(EXIT_FAILURE);
        }

        kind = selectedKind;
    }

    return kind;
}


std::string pmake::setup_language_standard(cxxopts::ParseResult const& parsedOptions)
{
    // TODO: perhaps the user could provide these?
    std::unordered_map<std::string_view, std::vector<std::string>> const availableStandards
    {
        { "c++"sv, { "23"s, "20"s, "17"s, "14"s, "11"s } },
        { "c"sv,   { "23"s, "17"s, "11"s, "99"s, } }
    };

    auto const language = parsedOptions["language"].as<std::string>();
    auto standard       = availableStandards.at(language).front();

    if (parsedOptions.count("standard"))
    {
        auto const selectedStandard = parsedOptions["standard"].as<std::string>();

        if (selectedStandard == "latest") { return standard; }

        if (!std::ranges::contains(availableStandards.at(language), selectedStandard))
        {
            std::println("The standard \"{}\" isn't for the language \"{}\".", selectedStandard, language);
            std::exit(EXIT_FAILURE);
        }

        standard = selectedStandard;
    }

    return standard;
}

std::filesystem::path pmake::setup_template_path(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();
    auto const kind     = parsedOptions["kind"].as<std::string>();

    std::filesystem::path path { "pmake-templates\\"s + language + "\\" + kind };

    if (kind == "executable")
    {
        if (parsedOptions.count("console")) { path += "\\console"; }
        else
        {
            path += "\\console";
        }
    }
    else if (kind == "library")
    {
        if (parsedOptions.count("static")) { path += "\\static"; }
        else if (parsedOptions.count("header-only")) { path += "\\header-only"; }
        else
        {
            path += "\\static";
        }
    }

    if (!std::filesystem::exists(path))
    {
        std::println("The project template \"{}\" doesn't exist.", path.string());
        std::exit(EXIT_FAILURE);
    }

    return path;
}

