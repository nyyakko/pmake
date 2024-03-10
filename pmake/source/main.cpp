#include "runtime/Runtime.hpp"

#include <cxxopts.hpp>
#include <err_or/ErrorOr.hpp>
#include <minwindef.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <print>
#include <ranges>

auto static const ROOT_FOLDER      = runtime::get_program_root_dir().string();
auto static const TEMPLATES_FOLDER = std::format("{}\\pmake-templates", ROOT_FOLDER);

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

error::ErrorOr<std::string> setup_language(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();

    if (!std::filesystem::exists(std::format("{}\\{}", TEMPLATES_FOLDER, language)))
    {
        return error::make_error("The language \"{}\" doesn't have a template setup for it. Available languages: [{}]", language, [] {
                return std::filesystem::directory_iterator(TEMPLATES_FOLDER)
                    | std::views::filter([] (auto entry) { return std::filesystem::is_directory(entry) && !entry.path().filename().string().starts_with('.'); })
                    | std::views::transform([] (auto entry) { return std::format("{}", entry.path().filename().string()); })
                    | std::views::join_with(',')
                    | std::ranges::to<std::string>();
        }());
    }

    return language;
}

error::ErrorOr<std::pair<std::string, std::string>> setup_kind(cxxopts::ParseResult const& parsedOptions)
{
    using namespace std::literals;

    auto const language = parsedOptions["language"].as<std::string>();
    auto const kind     = parsedOptions["kind"].as<std::string>();

    if (!std::filesystem::exists(std::format("{}\\{}\\{}", TEMPLATES_FOLDER, language, kind)))
    {
        return error::make_error("The kind \"{}\" for the language \"{}\" doesn't have a template setup for it.", kind, language);
    }

    // FIXME: should find a better way to handle this in the future. perhaps with another ``pmake-info`` ?
    if (kind == "executable")
    {
        // cppcheck-suppress duplicateBranch
        if (parsedOptions.count("console")) { return std::pair { kind, "console"s }; }
        else
        {
            return std::pair { kind, "console"s };
        }
    }
    else if (kind == "library")
    {
        if (parsedOptions.count("static")) { return std::pair { kind, "static"s }; }
        else if (parsedOptions.count("header-only")) { return std::pair { kind, "header-only"s }; }
        else
        {
            return std::pair { kind, "static"s };
        }
    }

    std::unreachable();
}

error::ErrorOr<std::string> setup_language_standard(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();
    auto const standard = parsedOptions["standard"].as<std::string>();

    std::ifstream stream { std::format("{}\\{}\\pmake-info.json", TEMPLATES_FOLDER, language) };

    if (stream.fail())
    {
        return error::make_error("The language \"{}\" doesn't have standards available.", language);
    }

    nlohmann::json info {};
    stream >> info;

    auto const fnToString = [] (auto object) { return object.template get<std::string>(); };

    if (standard != "latest" && !std::ranges::contains(info["standards"] | std::views::transform(fnToString), standard))
    {
        return error::make_error("The standard \"{}\" for the language \"{}\" isn't available. Available standards: [{}]", standard, language, [&] {
                return info["standards"]
                    | std::views::transform([] (auto entry) { return entry.template get<std::string>(); })
                    | std::views::join_with(',')
                    | std::ranges::to<std::string>();
        }());
    }
    else
    {
        if (standard == "latest")
        {
            return info["standards"].front().get<std::string>();
        }
    }

    return standard;
}

error::ErrorOr<std::filesystem::path> setup_template_path(cxxopts::ParseResult const& parsedOptions)
{
    auto const language = parsedOptions["language"].as<std::string>();
    auto const kind     = parsedOptions["kind"].as<std::string>();

    std::filesystem::path path { std::format("{}\\{}\\{}", TEMPLATES_FOLDER, language, kind) };

    if (kind == "executable")
    {
        // cppcheck-suppress duplicateBranch
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
        return error::make_error("The project template \"{}\" doesn't exist.", path.string());
    }

    return path;
}

std::filesystem::path copy_and_rename_files_recursively(std::string projectName, std::unordered_map<std::string, std::string> const& wildcards, std::filesystem::path from, std::filesystem::path to)
{
    for (auto const& entry : std::filesystem::directory_iterator(from))
    {
        auto entryName = entry.path().filename().string();

        for (auto const& wildcard : wildcards | std::views::keys)
        {
            auto const position = entryName.find(wildcard);

            if (position != std::string::npos)
            {
                auto const first = std::next(entryName.begin(), static_cast<int64_t>(position));
                auto const last  = std::next(first, static_cast<int64_t>(wildcard.size()));

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

void replace_wildcards_recursively(std::filesystem::path from, std::unordered_map<std::string, std::string> const& wildcards)
{
    for (auto const& entry : std::filesystem::directory_iterator(from))
    {
        if (std::filesystem::is_directory(entry))
        {
            replace_wildcards_recursively(entry, wildcards);
            continue;
        }

        std::fstream fileStream { entry.path() };
        std::stringstream contentStream {};
        contentStream << fileStream.rdbuf();

        auto content = contentStream.str();

        for (auto const& wildcard : wildcards | std::views::keys)
        {
            while (auto const wildcardPosition = content.find(wildcard))
            {
                if (wildcardPosition == std::string::npos) { break; }

                auto const first = std::next(content.begin(), static_cast<int>(wildcardPosition));
                auto const last  = std::next(first, static_cast<int>(wildcard.size()));

                content.replace(first, last, wildcards.at(wildcard));
            }
        }

        std::fstream { entry.path(), std::ios::in | std::ios::out | std::ios::trunc } << content;
    }
}

error::ErrorOr<std::filesystem::path> create_from_template(std::string projectName, std::string projectLanguage, std::string projectStandard, std::filesystem::path templatePath)
{
    if (!std::filesystem::exists(projectName)) { std::filesystem::create_directory(projectName); }

    std::ifstream fileStream { std::format("{}\\pmake-info.json", TEMPLATES_FOLDER) };

    if (fileStream.fail())
    {
        return error::make_error("Couldn't open {}\\pmake-info.json", TEMPLATES_FOLDER);
    }

    auto const info = nlohmann::json::parse(fileStream);

    std::unordered_map<std::string, std::string> wildcards {};

    wildcards.emplace(info["wildcards"]["project_name"], projectName);
    wildcards.emplace(info["wildcards"]["project_language"], projectLanguage);
    wildcards.emplace(info["wildcards"]["project_standard"], projectStandard);

    std::filesystem::path const from { templatePath };
    std::filesystem::path const to   { std::filesystem::current_path().append(projectName) };

    replace_wildcards_recursively(copy_and_rename_files_recursively(projectName, wildcards, from, to), wildcards);

    return to;
}

void print_project_setup(std::string projectName, std::string projectLanguage, std::string projectStandard, std::pair<std::string, std::string> projectKind, std::filesystem::path projectDestination)
{
    std::println("┌– [pmake] –––");
    std::println("| name.......: {}", projectName);
    std::println("| language...: {} ({})", projectLanguage, projectStandard);
    std::println("| kind.......: {} ({})", projectKind.first, projectKind.second);
    std::println("|");
    std::println("| output.....: {}", projectDestination.string());
    std::println("└–––––––––––––");
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
        | cxxopts::option("console", "")
        | cxxopts::option("static", "")
        // cppcheck-suppress constStatement
        | cxxopts::option("header-only", "");

    auto const parsedOptions = options.parse(argumentCount, argumentValues);

    if (!parsedOptions.count("name") || parsedOptions.count("help"))
    {
        std::println("{}", options.help());
        return EXIT_FAILURE;
    }

    if (!std::filesystem::exists(TEMPLATES_FOLDER))
    {
        std::println("Couldn't find pmake-templates folder. Did you install the program properly?");
        return EXIT_FAILURE;
    }

    auto const projectName     = parsedOptions["name"].as<std::string>();
    auto const projectLanguage = MUST(setup_language(parsedOptions));
    auto const projectKind     = MUST(setup_kind(parsedOptions));
    auto const projectStandard = MUST(setup_language_standard(parsedOptions));

    auto const result = MUST(create_from_template(projectName, projectLanguage, projectStandard, MUST(setup_template_path(parsedOptions))));

    print_project_setup(projectName, projectLanguage, projectStandard, projectKind, result);
}

