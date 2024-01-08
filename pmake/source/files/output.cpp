#include "pmake/files/output.hpp"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <ranges>

using namespace std::literals;

void copy_and_rename_files_recursively(std::string_view projectName, std::unordered_map<std::string_view, std::string_view> const& wildcards, std::filesystem::path from, std::filesystem::path to);
void replace_wildcards_recursively(std::filesystem::path from, std::unordered_map<std::string_view, std::string_view> const& wildcards);

void pmake::create_from_template(std::filesystem::path templatePath, std::string_view projectName, std::string_view projectLanguage, std::string_view projectStandard)
{
    if (!std::filesystem::exists(projectName)) { std::filesystem::create_directory(projectName); }

    std::unordered_map<std::string_view, std::string_view> wildcards
    {
        { "!PROJECT!", projectName },
        { "!LANGUAGE!", projectLanguage },
        { "!STANDARD!", projectStandard },
    };

    std::filesystem::path const from { templatePath };
    std::filesystem::path const to   { projectName };

    copy_and_rename_files_recursively(projectName, wildcards, from, to);
    replace_wildcards_recursively(from, wildcards);
}

void copy_and_rename_files_recursively(std::string_view projectName, std::unordered_map<std::string_view, std::string_view> const& wildcards, std::filesystem::path from, std::filesystem::path to)
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

            continue;
        }

        auto const workingDirectory = std::filesystem::path(to).append(entryName);
        std::filesystem::create_directory(workingDirectory);
        copy_and_rename_files_recursively(projectName, wildcards, entry, workingDirectory);
    }
}

void replace_wildcards_recursively(std::filesystem::path from, std::unordered_map<std::string_view, std::string_view> const& wildcards)
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

