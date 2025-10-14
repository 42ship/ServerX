#pragma once

#include "config/Block.hpp"
#include "config/internal/types.hpp"
#include <string>

namespace config {

/**
 * @interface IDirective
 * @brief Defines the contract for a configuration directive handler.
 *
 * This interface uses the Strategy Pattern to encapsulate the logic for processing
 * a specific directive (e.g., 'listen', 'root') from the configuration file.
 * Each concrete directive class will implement this interface to handle its
 * specific syntax and validation.
 */
class IDirective {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IDirective() {}

    /**
     * @brief The primary logic for processing the directive's arguments.
     * @param block The configuration block (ServerBlock or LocationBlock) to modify.
     * @param args The vector of string arguments for the directive.
     */
    virtual void process(Block &block, ParsedDirectiveArgs const &args) const = 0;

    /**
     * @brief Gets the name of the directive.
     * @return A constant reference to the directive's name (e.g., "listen").
     */
    virtual std::string const &getName() const = 0;
};

} // namespace config
