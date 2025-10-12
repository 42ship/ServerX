#pragma once

#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include "utils/utils.hpp"

/**
 * @def EXPECT_ARG_COUNT(args, expected, name)
 * @brief Validates the number of arguments for a directive.
 */
#define EXPECT_ARG_COUNT(args, expected, name)                                                     \
    if ((args).size() != (expected)) {                                                             \
        throw config::ConfigError("'" + (name) + "' directive requires exactly " +                 \
                                  utils::toString(expected) + " argument(s).");                    \
    }

/**
 * @def EXPECT_ARG_TYPE(arg, expected_type, name)
 * @brief Validates the type of a specific argument.
 */
/*
#define EXPECT_ARG_TYPE(arg, expected_type, name) \
    if ((arg)->getType() != (expected_type)) { \
        throw config::ConfigError("Invalid argument type for '" + (name) + "': expected " + \
                                  config::getArgumentTypeName(expected_type) + " but got " + \
                                  config::getArgumentTypeName((arg)->getType()) + \
                                  " with a value of '" + (arg)->getRawValue() + "'."); \
    }

#define GET_ARG_AS(var_name, type, arg_ptr, directive_name) \
    type *var_name = dynamic_cast<type *>((arg_ptr)); \
    if (!(var_name)) { \
        throw config::ConfigError("Internal error for '" + (directive_name) + \
                                  "': expected argument to be of type " #type "."); \
    }*/
