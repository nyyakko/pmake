#include "Environment.hpp"

#include <cxxopts.hpp>
#include <fplus/fplus.hpp>
#include <liberror/Result.hpp>
#include <liberror/Try.hpp>
#include <libpreprocessor/Processor.hpp>
#include <nlohmann/json.hpp>

#define APPLICATION_RESOURCES_FOLDER get_application_data_path()
#define APPLICATION_TEMPLATES_FOLDER APPLICATION_RESOURCES_FOLDER / "templates"
#define APPLICATION_FEATURES_FOLDER  APPLICATION_TEMPLATES_FOLDER / "features"
#define APPLICATION_COMMON_FOLDER    APPLICATION_TEMPLATES_FOLDER / "common"

liberror::Result<void> copy(std::filesystem::path const& source, std::filesystem::path const& destination)
{
    try
    {
        std::filesystem::copy(source, destination, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    }
    catch (std::filesystem::filesystem_error const& error)
    {
        return liberror::make_error("{}", error.what());
    }

    return {};
}

std::string replace(std::string content, std::pair<std::string, std::string> const& wildcard)
{
    for (auto position = content.find(wildcard.first); position != std::string::npos; position = content.find(wildcard.first))
    {
        auto const first = std::next(content.begin(), static_cast<int>(position));
        auto const last = std::next(first, static_cast<int>(wildcard.first.size()));
        content.replace(first, last, wildcard.second);
    }

    return content;
}

void replace_file_name_wildcards(std::filesystem::path const& path, std::unordered_map<std::string, std::string> const& wildcards)
{
    auto const fnRename = [](auto&& entry, auto&& wildcard) {
        auto path = entry.parent_path();
        auto name = entry.filename().string();
        std::filesystem::rename(entry, path.append(replace(name, wildcard)));
    };

    for (auto const& entry :
            std::filesystem::directory_iterator(path) | std::views::transform(&std::filesystem::directory_entry::path))
    {
        if (std::filesystem::is_directory(entry))
        {
            replace_file_name_wildcards(entry, wildcards);
        }

        auto const fnKeepWildcardIfPresent = std::views::filter([&](auto&& wildcard) { return entry.filename().string().contains(wildcard.first); });
        std::ranges::for_each(wildcards | fnKeepWildcardIfPresent, std::bind_front(fnRename, entry));
    }
}

void replace_file_wildcards(std::filesystem::path const& path, std::unordered_map<std::string, std::string> const& wildcards)
{
    auto const fnReplace = [](auto&& entry, auto&& wildcard) {
        std::stringstream contentStream {};
        contentStream << std::ifstream(entry).rdbuf();
        std::ofstream outputStream(entry, std::ios::trunc);
        outputStream << replace(contentStream.str(), wildcard);
    };

    auto fnFilterRegularFile = std::views::filter([](auto&& entry) {
        return std::filesystem::is_regular_file(entry);
    });

    for (auto const& entry :
            std::filesystem::recursive_directory_iterator(path) | std::views::transform(&std::filesystem::directory_entry::path) | fnFilterRegularFile)
    {
        std::ranges::for_each(wildcards, std::bind_front(fnReplace, entry));
    }
}

struct ProjectSettings
{
    std::string name;
    std::string language;
    std::string standard;
    std::string kind;
    std::string mode;
    std::vector<std::string> features;
    std::unordered_map<std::string, std::string> wildcards;
};

struct Context
{
    cxxopts::ParseResult const& arguments;
    nlohmann::json const& configuration;
};

liberror::Result<void> sanitize_project_settings(Context const& context, ProjectSettings const& project)
{
    auto language = context.arguments["language"].as<std::string>();
    if (!context.configuration["languages"].contains(language))
    {
        return liberror::make_error("Language \"{}\" is not available.", language);
    }

    auto standard = context.arguments["standard"].as<std::string>();
    if (!fplus::is_elem_of(standard, context.configuration["languages"][project.language]["standards"]))
    {
        return liberror::make_error("Standard \"{}\" is not available for {}.", standard, project.language);
    }

    auto kind = context.arguments["kind"].as<std::string>();
    if (!context.configuration["languages"][project.language]["templates"].contains(kind))
    {
        return liberror::make_error("Kind \"{}\" is not available for {}.", kind, project.language);
    }

    auto mode = context.arguments["mode"].as<std::string>();
    if (!context.configuration["languages"][project.language]["templates"][project.kind]["modes"].contains(mode))
    {
        return liberror::make_error("Template kind \"{}\" in mode \"{}\" is not available for {}.", project.kind, mode, project.language);
    }

    return {};
}

ProjectSettings setup_project(Context const& context)
{
    ProjectSettings project {};

    using namespace std::literals;

    auto fnParameterOrDefault = [&context] <class T> (std::string_view parameter, T&& defaultValue) {
        return context.arguments.count(parameter.data()) ? context.arguments[parameter.data()].as<std::decay_t<T>>() : std::forward<T>(defaultValue);
    };

    project.name = fnParameterOrDefault("name", "myproject"s);
    project.language = fnParameterOrDefault("language", "c++"s);
    project.standard = fnParameterOrDefault("standard", "23"s);
    project.kind = fnParameterOrDefault("kind", "executable"s);
    project.mode = fnParameterOrDefault("mode", "console"s);
    project.features = fnParameterOrDefault("features", std::vector<std::string>{});
    project.wildcards = {
        { context.configuration["wildcards"]["name"], project.name },
        { context.configuration["wildcards"]["language"], project.language },
        { context.configuration["wildcards"]["standard"], project.standard },
    };

    return project;
}

liberror::Result<void> install_project_features(ProjectSettings const& project)
{
    for (auto const& feature : project.features)
    {
        if (std::filesystem::is_directory(APPLICATION_FEATURES_FOLDER / feature))
        {
            TRY(::copy(APPLICATION_FEATURES_FOLDER / feature, project.name));
        }
        else
        {
            fmt::println("Feature \"{}\" is unavailable.", feature);
        }
    }

    return {};
}

liberror::Result<void> preprocess_project_files(std::filesystem::path const& path, libpreprocessor::PreprocessorContext const& context)
{
    auto fnFilterRegularFile = std::views::filter([](auto&& entry) {
        return std::filesystem::is_regular_file(entry);
    });

    for (auto const& entry : std::filesystem::recursive_directory_iterator(path) | fnFilterRegularFile)
    {
        auto const content = TRY(process(entry.path(), context));
        std::ofstream outputStream(entry.path());
        outputStream << content;
    }

    return {};
}

liberror::Result<void> create_project(ProjectSettings const& project)
{
    using namespace std::literals;

    if (std::filesystem::exists(project.name))
    {
        return liberror::make_error("Directory \"{}\" already exists.", project.name);
    }

    TRY(::copy(APPLICATION_COMMON_FOLDER, project.name));

    if (!project.features.empty())
    {
        TRY(install_project_features(project));
    }

    libpreprocessor::PreprocessorContext context
    {
        .environmentVariables = {
            { "ENV:LANGUAGE", project.language },
            { "ENV:STANDARD", project.standard },
            { "ENV:KIND", project.kind },
            { "ENV:MODE", project.mode },
            { "ENV:FEATURES", fplus::join(","s, project.features) },
        }
    };

    TRY(preprocess_project_files(project.name, context));

    replace_file_name_wildcards(project.name, project.wildcards);
    replace_file_wildcards(project.name, project.wildcards);

    return {};
}

liberror::Result<void> safe_main(std::span<char const*> const& arguments)
{
    cxxopts::Options options("pmake", "Utility for creating C and C++ projects based on pre-defined templates.");

    options.add_options()("h,help", "");
    options.add_options()("n,name", "", cxxopts::value<std::string>());
    options.add_options()("l,language", "", cxxopts::value<std::string>()->default_value("c++"));
    options.add_options()("s,standard", "", cxxopts::value<std::string>()->default_value("latest"));
    options.add_options()("k,kind", "", cxxopts::value<std::string>()->default_value("executable"));
    options.add_options()("m,mode", "", cxxopts::value<std::string>()->default_value("console"));
    options.add_options()("features", "", cxxopts::value<std::vector<std::string>>());

    auto parsedArguments = options.parse(static_cast<int>(arguments.size()), arguments.data());

    if (parsedArguments.count("help") || parsedArguments.arguments().empty())
    {
        fmt::print("{}", options.help());
        return {};
    }

    auto const configurationPath = APPLICATION_RESOURCES_FOLDER / "pmake-info.json";
    auto const parsedConfiguration = nlohmann::json::parse(std::ifstream(configurationPath), nullptr, false);
    if (parsedConfiguration.is_discarded())
    {
        return liberror::make_error("Couldn't open {}.", configurationPath.string());
    }

    auto project = setup_project({ parsedArguments, parsedConfiguration });
    TRY(create_project(project));

    return {};
}

int main(int argc, char const** argv)
{
    auto result = safe_main(std::span<char const*>(argv, size_t(argc)));

    if (!result.has_value())
    {
        fmt::println("{}", result.error().message());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
