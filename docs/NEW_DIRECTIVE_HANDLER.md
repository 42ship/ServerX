# How to Add a New Configuration Directive

Our server's configuration parsing now uses a Strategy Pattern. Each configuration directive (e.g., `listen`, `root`, `upload_path`) is its own class that implements the `IDirective` interface. This design is clean, encapsulated, and follows the Open/Closed Principle—to add new directives, you add new classes without modifying existing ones.

Here is a step-by-step guide to add a new directive for `upload_path` which is only valid inside a `location` block.

### Step 1: Create the Directive Class Files

First, create the header and source files for your new directive.

  * Header: `inc/config/directives/UploadPathDirective.hpp`
  * Source: `src/config/directives/UploadPathDirective.cpp`

### Step 2: Define the Directive Header

In `UploadPathDirective.hpp`, define the class, inheriting from `IDirective` and implementing its virtual functions.

```cpp
// in inc/config/directives/UploadPathDirective.hpp
#pragma once

#include "config/directives/IDirective.hpp"

namespace config {

class UploadPathDirective : public IDirective {
public:
    void process(Block &block, StringVector const &args) const;
    std::string const &getName() const { return name_; }

private:
    static const std::string name_;
};

} // namespace config
```

### Step 3: Implement the Directive's Logic

In `UploadPathDirective.cpp`, implement the `process` method. This is where all the logic for validating and setting the directive lives. The logic that was previously split between `DirectiveHandler` and `Validator` is now cleanly encapsulated in one place.

```cpp
// in src/config/directives/UploadPathDirective.cpp

namespace config {

const std::string UploadPathDirective::name_ = "upload_path";

void UploadPathDirective::process(Block &b, StringVector const &args) const {
    // 1. Argument Validation: Check the number of arguments.
    if (args.size() != 1) {
        throw ConfigError("'" + name_ + "' directive requires exactly one argument.");
    }

    // 2. Context Validation: Ensure it's in a 'location' block.
    if (!dynamic_cast<LocationBlock *>(&b)) {
        throw ConfigError("'" + name_ + "' directive is not allowed here.");
    }

    std::string const &uploadPath = args[0];

    // 3. Value Validation: Check if the path is valid (optional, for startup checks).
    // This logic would be run if `perform_fs_checks` is enabled.
    char const *error = utils::validateDirectoryPath(uploadPath.c_str());
    if (error) {
        issue_warning("'" + name_ + "': path '" + uploadPath + "' " + error);
    }

    // 4. Apply to Block: Use the public 'add' method to store the value.
    // This avoids direct access to internal members and removes the need for 'friend'.
    b.add(name_, uploadPath);
}

} // namespace config
```

### Step 4: Register the New Directive

Finally, the central parsing orchestrator needs to be aware of your new directive class. You would register an instance of `UploadPathDirective` in a factory or registry so that when the parser encounters the "upload\_path" string, it can delegate processing to your new object.

This would involve refactoring the `DirectiveHandler` to hold a map of `IDirective` objects.

```cpp
// in src/config/pipeline/DirectiveHandler.cpp

#include "config/directives/UploadPathDirective.cpp"

DirectiveHandler::DirectiveHandler() {
    ...
    registerHandler(new ListenDirective);
}

}
```
