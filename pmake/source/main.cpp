#include <print>

#include "pmake/cli/options.hpp"
#include "pmake/files/output.hpp"
#include "pmake/setup.hpp"

int main(int argumentCount, char const** argumentValues)
{
    cxxopts::Options options { "pmake" };

    options.add_options()
        | cxxopts::option("h,help", "shows this menu")
        | cxxopts::option("n,name", "name of the project", cxxopts::value<std::string>())
        | cxxopts::option("k,kind", "kind of the project", cxxopts::value<std::string>()->default_value("executable"))
        | cxxopts::option("l,language", "language used in the project", cxxopts::value<std::string>()->default_value("c++"))
        | cxxopts::option("s,standard", "language standard used in the project", cxxopts::value<std::string>()->default_value("latest"))
        | cxxopts::option("static", "", cxxopts::value<bool>())
        | cxxopts::option("header-only", "", cxxopts::value<bool>());

    auto parsedOptions = options.parse(argumentCount, argumentValues);

    if (!parsedOptions.count("name") || parsedOptions.count("help"))
    {
        std::println("{}", options.help());
        return EXIT_SUCCESS;
    }

    auto projectName     = parsedOptions["name"].as<std::string>();
    auto projectKind     = parsedOptions["kind"].as<std::string>();
    auto projectLanguage = pmake::setup_language(parsedOptions);
    auto projectStandard = pmake::setup_language_standard(parsedOptions, projectLanguage);

    std::println("project name....: {}", projectName);
    std::println("project kind....: {}", projectKind);
    std::println("project language: {} ({})", projectLanguage, projectStandard);

    pmake::create_from_template(pmake::setup_template_path(parsedOptions, projectLanguage, projectKind), projectName, projectLanguage, projectStandard);
}

