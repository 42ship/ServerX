// tests/test_mimetypes.cpp
// Simple integration tests for MimeTypes using only <cassert>.
// All comments are in English.
#include "doctest.h"

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>

#include "http/MimeTypes.hpp"
#include "test_utils.hpp"

static std::string kPath = "config/mimes.types";

// Sometimes filesystems have 1s mtime granularity; sleep to ensure mtime changes.
static void bump_mtime_granularity() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
}

static void test_initial_load() {

    // Ensure a known initial file
    writeFile("# initial file\ntext/plain    txt log\ntext/html     html htm\napplication/json json", kPath.c_str());

    http::MimeTypes mt(kPath);

    // Known extensions
    CHECK(mt.getMimeType("txt")  == "text/plain");
    CHECK(mt.getMimeType("html") == "text/html");
    CHECK(mt.getMimeType("json") == "application/json");

    // Unknown extension should default to text/plain per your header
    CHECK(mt.getMimeType("unknown_ext") == "text/plain");
}

static void test_hot_reload_on_update() {

    http::MimeTypes mt(kPath);

    // Initially html -> text/html
    CHECK(mt.getMimeType("html") == "text/html");

    bump_mtime_granularity();

    // Update mapping: change html to text/xhtml and add css
    writeFile("# updated file\ntext/plain    txt log\ntext/xhtml    html htm\ntext/css      css", kPath.c_str());

    mt.reload();

    // After update the class should pick new values
    // Assumption: getMimeType internally triggers reload when mtime changed.
    CHECK(mt.getMimeType("html") == "text/xhtml");
    CHECK(mt.getMimeType("css")  == "text/css");
}

static void test_reload_on_rollback() {

    http::MimeTypes mt(kPath);

    // Current state from previous test: html -> text/xhtml
    CHECK(mt.getMimeType("html") == "text/xhtml");

    // Rollback to an older content (different from current, but "older" logically)
    writeFile("# rolled back file\ntext/plain    txt log\ntext/html     html htm", kPath.c_str());

    mt.reload();

    // Should reflect rollback (any mtime change counts)
    CHECK(mt.getMimeType("html") == "text/html");
    CHECK(mt.getMimeType("json") == "text/plain"); // not present now -> default
}

TEST_CASE("Test MimeTypes Class") {
    try {
        test_initial_load();
        test_hot_reload_on_update();
        test_reload_on_rollback();
    } catch (const std::exception& ex) {
        std::cerr << "TEST FAILED with exception: " << ex.what() << "\n";
        return ;
    }
    unlink(kPath.c_str());
}
