#include <cxxopts.hpp>
#include <nlohmann/json.hpp>

#include <print>
#include <fstream>

namespace cxxopts {

    constexpr auto option(auto&&... arguments)
    {
        return Option(arguments...);
    }

    inline OptionAdder operator|(OptionAdder adder, Option option)
    {
        return adder(option.opts_, option.desc_, option.value_, option.arg_help_);
    }

}

std::string setup_language(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();

    if (!std::filesystem::exists("pmake-templates\\" + language))
    {
        throw std::runtime_error(std::format("The language \"{}\" doesn't have a template setup for it.", language));
    }

    return language;
}

std::pair<std::string, std::string> setup_kind(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();
    auto const kind     = parsedOptions["kind"].as<std::string>();

    if (!std::filesystem::exists("pmake-templates\\" + language + "\\" + kind))
    {
        throw std::runtime_error(std::format("The kind \"{}\" for the language \"{}\" doesn't have a template setup for it.", kind, language));
    }

    // FIXME: should find a better way to handle this in the future. perhaps with another ``pmake-info`` ?
    if (kind == "executable")
    {
        if (parsedOptions.count("console")) { return { kind, "console" }; }
        else
        {
            return { kind, "console" };
        }
    }

    if (kind == "library")
    {
        if (parsedOptions.count("static")) { return { kind, "static" }; }
        else if (parsedOptions.count("header-only")) { return { kind, "header-only" }; }
        else
        {
            return { kind, "static" };
        }
    }

    std::unreachable();
}

std::string setup_language_standard(cxxopts::ParseResult const& parsedOptions)
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

std::filesystem::path setup_template_path(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();
    auto const kind     = parsedOptions["kind"].as<std::string>();

    std::filesystem::path path { "pmake-templates\\" + language + "\\" + kind };

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

std::filesystem::path copy_and_rename_files_recursively(std::string_view projectName, std::unordered_map<std::string, std::string_view> const& wildcards, std::filesystem::path from, std::filesystem::path to)
{
    for (auto const& entry : std::filesystem::directory_iterator(from))
    {
        auto entryName = entry.path().filename().string();

        for (auto const& wildcard : wildcards | std::views::keys)
        {
            auto const position = entryName.find(wildcard);

            if (position != std::string::npos)
            {
                auto const first = std::next(entryName.begin(), static_cast<int>(position));
                auto const last  = std::next(first, static_cast<int>(wildcard.size()));

                entryName.replace(first, last, projectName);
            }
        }

        if (!std::filesystem::is_directory(entry))
        {
            std::filesystem::copy(entry, to);

            if (entryName != entry.path().filename().string())
            {
                std::filesystem::rename(std::filesystem::path(to).append(entry.path().filename().string()), std::filesystem::path(to).append(entryName));
            }
        }
        else
        {
            auto const workingDirectory = std::filesystem::path(to).append(entryName);
            std::filesystem::create_directory(workingDirectory);
            copy_and_rename_files_recursively(projectName, wildcards, entry, workingDirectory);
        }
    }

    return to;
}

void replace_wildcards_recursively(std::filesystem::path from, std::unordered_map<std::string, std::string_view> const& wildcards)
{
    for (auto const& entry : std::filesystem::directory_iterator(from))
    {
        if (std::filesystem::is_directory(entry))
        {
            replace_wildcards_recursively(entry, wildcards);
            continue;
        }

        std::stringstream fileContentStream {};
        fileContentStream << std::fstream(entry.path()).rdbuf();

        auto content = fileContentStream.str();

        for (auto const& wildcard : wildcards | std::views::keys)
        {
            while (auto const wildcardPosition = content.find(wildcard))
            {
                if (wildcardPosition == std::string::npos)
                {
                    break;
                }

                auto const first = std::next(content.begin(), static_cast<int>(wildcardPosition));
                auto const last  = std::next(first, static_cast<int>(wildcard.size()));

                content.replace(first, last, wildcards.at(wildcard));
            }
        }

        std::fstream fileStream { entry.path(), std::ios::in | std::ios::out | std::ios::trunc };
        fileStream << content;
    }
}

void create_from_template(std::filesystem::path templatePath, std::string_view projectName, std::string_view projectLanguage, std::string_view projectStandard)
{
    if (!std::filesystem::exists(projectName)) { std::filesystem::create_directory(projectName); }

    std::ifstream stream { "pmake-templates\\pmake-info.json" };
    nlohmann::json info {};
    stream >> info;

    std::unordered_map<std::string, std::string_view> wildcards {};

    wildcards.emplace(info["wildcards"]["project_name"], projectName);
    wildcards.emplace(info["wildcards"]["project_language"], projectLanguage);
    wildcards.emplace(info["wildcards"]["project_standard"], projectStandard);

    std::filesystem::path const from { templatePath };
    std::filesystem::path const to   { projectName };

    replace_wildcards_recursively(copy_and_rename_files_recursively(projectName, wildcards, from, to), wildcards);
}

int main(int argumentCount, char const** argumentValues)
{
    cxxopts::Options options { "pmake" };

    options.add_options()
        | cxxopts::option("h,help", "shows this menu")
        | cxxopts::option("n,name", "name of the project", cxxopts::value<std::string>())
        | cxxopts::option("l,language", "language used in the project", cxxopts::value<std::string>()->default_value("c++"))
        | cxxopts::option("k,kind", "kind of the project", cxxopts::value<std::string>()->default_value("executable"))
        | cxxopts::option("s,standard", "language standard used in the project", cxxopts::value<std::string>()->default_value("latest"))

        | cxxopts::option("console", "") | cxxopts::option("static", "") | cxxopts::option("header-only", "");

    auto const parsedOptions = options.parse(argumentCount, argumentValues);

    if (!parsedOptions.count("name") || parsedOptions.count("help"))
    {
        std::println("{}", options.help());
        return EXIT_SUCCESS;
    }

    try
    {
        auto const projectName     = parsedOptions["name"].as<std::string>();
        auto const projectLanguage = setup_language(parsedOptions);
        auto const projectKind     = setup_kind(parsedOptions);
        auto const projectStandard = setup_language_standard(parsedOptions);

        std::println("┌─ pmake ─────────────");
        std::println("│ name....: {}", projectName);
        std::println("│ kind....: {} ({})", projectKind.first, projectKind.second);
        std::println("│ language: {} ({})", projectLanguage, projectStandard);
        std::println("└─────────────────────");

        create_from_template(setup_template_path(parsedOptions), projectName, projectLanguage, projectStandard);

    }
    catch (std::exception const& exception)
    {
        std::println("{}", exception.what());
    }
}
