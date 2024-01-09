#include "setup.hpp"

#include <print>
#include <fstream>

#include <nlohmann/json.hpp>

using namespace std::literals;

std::string pmake::setup_language(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();

    if (!std::filesystem::exists("pmake-templates\\" + language))
    {
        throw std::runtime_error(std::format("The language \"{}\" doesn't have a template setup for it.", language));
    }

    return language;
}

std::string pmake::setup_kind(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();
    auto const kind     = parsedOptions["kind"].as<std::string>();

    if (!std::filesystem::exists("pmake-templates\\" + language + "\\" + kind))
    {
        throw std::runtime_error(std::format("The kind \"{}\" for the language \"{}\" doesn't have a template setup for it.", kind, language));
    }

    return kind;
}


std::string pmake::setup_language_standard(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();
    auto standard       = parsedOptions["standard"].as<std::string>();

    std::ifstream stream { "pmake-templates\\" + language + "\\pmake-info.json" };
    nlohmann::json info {};
    stream >> info;

    auto const fnToString = [] (auto const& object) { return object.template get<std::string>(); };

    if (standard != "latest" && !std::ranges::contains(info["standards"] | std::views::transform(fnToString), standard))
    {
        throw std::runtime_error(
                std::format("The standard \"{}\" for the language \"{}\" isn't available. Available standards: {}", standard, language, info["standards"].dump()));
    }
    else
    {
        if (standard == "latest")
        {
            standard = info["standards"].front().get<std::string>();
        }
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
        throw std::runtime_error(std::format("The project template \"{}\" doesn't exist.", path.string()));
    }

    return path;
}

