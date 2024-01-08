#pragma once

#include <cxxopts.hpp>

namespace cxxopts {

    Option option(auto&&... arguments)
    {
        return Option(arguments...);
    }

    inline OptionAdder operator|(OptionAdder adder, Option option)
    {
        return adder(option.opts_, option.desc_, option.value_, option.arg_help_);
    }

}
