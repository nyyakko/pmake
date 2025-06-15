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

bool copy(std::filesystem::path const& source, std::filesystem::path const& destination)
{
    try
    {
        std::filesystem::copy(source, destination, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    }
    catch (std::filesystem::filesystem_error const& error)
    {
        return false;
    }

    return true;
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

liberror::Result<void> sanitize_project(nlohmann::json const& configuration, ProjectSettings const& project)
{
    if (!configuration["languages"].contains(project.language))
    {
        return liberror::make_error("Language \"{}\" is not available.", project.language);
    }

    if (!fplus::is_elem_of(project.standard, configuration["languages"][project.language]["standards"]))
    {
        return liberror::make_error("Standard \"{}\" is not available for {}.", project.standard, project.language);
    }

    if (!configuration["languages"][project.language]["templates"].contains(project.kind))
    {
        return liberror::make_error("Kind \"{}\" is not available for {}.", project.kind, project.language);
    }

    if (!configuration["languages"][project.language]["templates"][project.kind]["modes"].contains(project.mode))
    {
        return liberror::make_error("Template kind \"{}\" in mode \"{}\" is not available for {}.", project.kind, project.mode, project.language);
    }

    return {};
}

liberror::Result<ProjectSettings> configure_project(cxxopts::ParseResult const& arguments, nlohmann::json const& configuration)
{
    using namespace std::literals;

    ProjectSettings project {};

    auto fnParameterOrDefault = [&] <class T> (std::string_view parameter, T&& defaultValue) {
        return arguments.count(parameter.data()) ? arguments[parameter.data()].as<std::decay_t<T>>() : std::forward<T>(defaultValue);
    };

    project.name = fnParameterOrDefault("name", "myproject"s);
    project.language = fnParameterOrDefault("language", "c++"s);
    project.standard = fnParameterOrDefault("standard", configuration["languages"][project.language]["standards"][0].get<std::string>());
    project.kind = fnParameterOrDefault("kind", configuration["languages"][project.language]["templates"].begin().key());
    project.mode = fnParameterOrDefault("mode", configuration["languages"][project.language]["templates"][project.kind]["modes"].begin().key());
    project.features = fnParameterOrDefault("features", configuration["languages"][project.language]["templates"][project.kind]["modes"][project.mode]["features"]["required"].get<std::vector<std::string>>());
    project.wildcards = {
        { configuration["wildcards"]["name"], project.name },
        { configuration["wildcards"]["language"], project.language },
        { configuration["wildcards"]["standard"], project.standard },
    };

    TRY(sanitize_project(configuration, project));

    return project;
}

liberror::Result<void> preprocess_project(ProjectSettings const& project)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:LANGUAGE", project.language },
            { "ENV:STANDARD", project.standard },
            { "ENV:KIND", project.kind },
            { "ENV:MODE", project.mode },
            { "ENV:FEATURES", fplus::join(","s, project.features) },
        }
    };

    auto fnFilterRegularFile = std::views::filter([](auto&& entry) { return std::filesystem::is_regular_file(entry); });
    for (auto const& entry : std::filesystem::recursive_directory_iterator(project.name) | fnFilterRegularFile)
    {
        auto const content = TRY(libpreprocessor::process(entry.path(), context));
        std::ofstream outputStream(entry.path());
        outputStream << content;
    }

    return {};
}

liberror::Result<void> create_project(ProjectSettings const& project)
{
    if (std::filesystem::exists(project.name))
    {
        return liberror::make_error("Directory \"{}\" already exists.", project.name);
    }

    ::copy(APPLICATION_COMMON_FOLDER, project.name);

    for (auto const& feature : project.features)
    {
        if (std::filesystem::is_directory(APPLICATION_FEATURES_FOLDER / feature))
        {
            ::copy(APPLICATION_FEATURES_FOLDER / feature, project.name);
        }
        else
        {
            fmt::println("Feature \"{}\" is unavailable.", feature);
        }
    }

    TRY(preprocess_project(project));

    replace_file_name_wildcards(project.name, project.wildcards);
    replace_file_wildcards(project.name, project.wildcards);

    return {};
}

liberror::Result<void> safe_main(std::span<char const*> const& arguments)
{
    cxxopts::Options options("pmake", "Utility for creating C and C++ projects based on pre-defined templates.");

    options.add_options()("h,help", "", cxxopts::value<std::string>());
    options.add_options()("n,name", "", cxxopts::value<std::string>());
    options.add_options()("l,language", "", cxxopts::value<std::string>());
    options.add_options()("s,standard", "", cxxopts::value<std::string>());
    options.add_options()("k,kind", "", cxxopts::value<std::string>());
    options.add_options()("m,mode", "", cxxopts::value<std::string>());
    options.add_options()("features", "", cxxopts::value<std::vector<std::string>>());

    cxxopts::ParseResult parsedArguments {};

    try
    {
        parsedArguments = options.parse(static_cast<int>(arguments.size()), arguments.data());
    }
    catch(std::exception const& exception)
    {
        return liberror::make_error(exception.what());
    }

    if (parsedArguments.count("help") || parsedArguments.arguments().empty())
    {
        fmt::print("{}", options.help());
        return {};
    }

    auto configurationPath = APPLICATION_RESOURCES_FOLDER / "pmake-info.json";
    auto parsedConfiguration = nlohmann::json::parse(std::ifstream(configurationPath), nullptr, false);
    if (parsedConfiguration.is_discarded())
    {
        return liberror::make_error("Couldn't open {}.", configurationPath.string());
    }

    auto project = TRY(configure_project(parsedArguments, parsedConfiguration));
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
