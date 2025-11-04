# How to Add a New Configuration Variable

Our server supports Nginx-style variable interpolation in configuration files (e.g., `$host`, `$request_uri`). This guide explains how to add a new variable to the system.

The core of this system is a map that links variable names to "getter" functions which extract data from an HTTP request.

### Step 1: Create the Getter Function

In `config/arguments/Variable.cpp`, create a function that matches the `FuncVar` signature: `std::string(http::Request const &req)`.

This function should contain the logic to extract the desired value from the request object.

**Example for `$request_uri`:**

```cpp
std::string RequestUriFunc(http::Request const &req) {
    // Logic to extract the request URI from the request object
    return req.uri;
}
````

### Step 2: Register the New Variable

In the same file, add your new function to the map inside the `getMap()` function. The key should be the variable name *without* the `$`.

```cpp
FuncMap const &getMap() {
    static FuncMap map;

    if (!map.empty())
        return map;

    map["host"] = HostFunc;
    map["request_uri"] = RequestUriFunc; // <-- Add your new variable here

    return map;
}
```

That's it\! The system will now automatically recognize and process your new variable in configuration directives.
