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
        auto const projectLanguage = pmake::setup_language(parsedOptions);
        auto const projectKind     = pmake::setup_kind(parsedOptions);
        auto const projectStandard = pmake::setup_language_standard(parsedOptions);

        // stolen from https://github.com/ArthurSonzogni/FTXUI/blob/main/cmake/ftxui_message.cmake
        std::println("┌─ pmake ─────────────");
        std::println("│ name....: {}", projectName);
        std::println("│ kind....: {}", projectKind);
        std::println("│ language: {} ({})", parsedOptions["language"].as<std::string>(), projectStandard);
        std::println("└─────────────────────");

        pmake::create_from_template(pmake::setup_template_path(parsedOptions), projectName, projectLanguage, projectStandard);
    }
    catch (std::exception const& exception)
    {
        std::println("{}", exception.what());
    }
}

