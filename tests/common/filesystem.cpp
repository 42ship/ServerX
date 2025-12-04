#include "common/filesystem.hpp"
#include "doctest.h"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

using namespace utils;

TEST_CASE("Filesystem utilities - getFileExtension") {
    SUBCASE("Standard file extensions") {
        CHECK(getFileExtension("file.txt") == "txt");
        CHECK(getFileExtension("document.pdf") == "pdf");
        CHECK(getFileExtension("image.jpg") == "jpg");
        CHECK(getFileExtension("archive.tar.gz") == "tar.gz");
    }

    SUBCASE("Path with directories") {
        CHECK(getFileExtension("/path/to/file.txt") == "txt");
        // Note: getFileExtension returns everything after the first dot
        CHECK(getFileExtension("file.html") == "html");
    }

    SUBCASE("No extension") {
        CHECK(getFileExtension("README") == "");
        CHECK(getFileExtension("Makefile") == "");
        CHECK(getFileExtension("/path/to/file") == "");
    }

    SUBCASE("Hidden files") {
        CHECK(getFileExtension(".bashrc") == "bashrc");
        CHECK(getFileExtension(".gitignore") == "gitignore");
    }

    SUBCASE("Edge cases") {
        CHECK(getFileExtension("") == "");
        CHECK(getFileExtension(".") == "");
        CHECK(getFileExtension("file.") == "");
    }
}

TEST_CASE("Filesystem utilities - isDir") {
    SUBCASE("Existing directory") {
        CHECK(isDir("/tmp"));
        CHECK(isDir("."));
    }

    SUBCASE("Non-existent path") {
        CHECK_FALSE(isDir("/this/path/does/not/exist"));
    }

    SUBCASE("Regular file") {
        // Create a temporary file
        const char* tmpFile = "/tmp/test_isdir_file.txt";
        std::ofstream out(tmpFile);
        out << "test";
        out.close();
        
        CHECK_FALSE(isDir(tmpFile));
        
        // Cleanup
        std::remove(tmpFile);
    }
}

TEST_CASE("Filesystem utilities - validateDirectoryPath") {
    SUBCASE("Valid directory") {
        const char* tmpResult = validateDirectoryPath("/tmp");
        CHECK(tmpResult == nullptr);
        const char* dotResult = validateDirectoryPath(".");
        CHECK(dotResult == nullptr);
    }

    SUBCASE("Null or empty path") {
        const char* result = validateDirectoryPath(nullptr);
        CHECK(result != nullptr);
        if (result != nullptr) {
            CHECK(std::string(result) == "path cannot be null or empty");
        }
        
        result = validateDirectoryPath("");
        CHECK(result != nullptr);
        if (result != nullptr) {
            CHECK(std::string(result) == "path cannot be null or empty");
        }
    }

    SUBCASE("Non-existent path") {
        const char* result = validateDirectoryPath("/this/does/not/exist");
        CHECK(result != nullptr);
        if (result != nullptr) {
            CHECK(std::string(result) == "does not exist");
        }
    }

    SUBCASE("File instead of directory") {
        // Create a temporary file
        const char* tmpFile = "/tmp/test_validate_file.txt";
        std::ofstream out(tmpFile);
        out << "test";
        out.close();
        
        const char* result = validateDirectoryPath(tmpFile);
        CHECK(result != nullptr);
        if (result != nullptr) {
            CHECK(std::string(result) == "exists but is not a directory");
        }
        
        // Cleanup
        std::remove(tmpFile);
    }
}

TEST_CASE("Filesystem utilities - joinPaths") {
    SUBCASE("Normal paths") {
        CHECK(joinPaths("/var/www", "html") == "/var/www/html");
        CHECK(joinPaths("/path", "to/file") == "/path/to/file");
    }

    SUBCASE("Path with trailing slash") {
        CHECK(joinPaths("/var/www/", "html") == "/var/www/html");
        CHECK(joinPaths("/path/", "to/file") == "/path/to/file");
    }

    SUBCASE("Second path with leading slash") {
        CHECK(joinPaths("/var/www", "/html") == "/var/www/html");
        CHECK(joinPaths("/path", "/to/file") == "/path/to/file");
    }

    SUBCASE("Both with slashes") {
        CHECK(joinPaths("/var/www/", "/html") == "/var/www/html");
    }

    SUBCASE("Root path") {
        CHECK(joinPaths("/", "html") == "/html");
        CHECK(joinPaths("/", "/html") == "/html");
    }

    SUBCASE("Empty paths") {
        CHECK(joinPaths("", "html") == "/html");
        CHECK(joinPaths("/path", "") == "/path");
        CHECK(joinPaths("", "") == "/");
    }

    SUBCASE("Relative paths") {
        CHECK(joinPaths("var/www", "html") == "var/www/html");
    }
}

TEST_CASE("Filesystem utilities - writeFile") {
    const char* tmpFile = "/tmp/test_write_file.txt";

    SUBCASE("Write simple content") {
        bool result = writeFile("Hello, World!", tmpFile);
        CHECK(result);
        
        // Verify content
        std::ifstream in(tmpFile);
        std::string content;
        std::getline(in, content);
        in.close();
        CHECK(content == "Hello, World!");
        
        // Cleanup
        std::remove(tmpFile);
    }

    SUBCASE("Write empty content") {
        bool result = writeFile("", tmpFile);
        CHECK(result);
        
        // Verify file is empty
        std::ifstream in(tmpFile);
        in.seekg(0, std::ios::end);
        CHECK(in.tellg() == 0);
        in.close();
        
        // Cleanup
        std::remove(tmpFile);
    }

    SUBCASE("Overwrite existing file") {
        writeFile("First content", tmpFile);
        bool result = writeFile("Second content", tmpFile);
        CHECK(result);
        
        // Verify new content
        std::ifstream in(tmpFile);
        std::string content;
        std::getline(in, content);
        in.close();
        CHECK(content == "Second content");
        
        // Cleanup
        std::remove(tmpFile);
    }

    SUBCASE("Write to invalid path") {
        bool result = writeFile("content", "/invalid/path/that/does/not/exist/file.txt");
        CHECK_FALSE(result);
    }
}

TEST_CASE("Filesystem utilities - TempFile") {
    SUBCASE("Create and open temp file") {
        TempFile tempFile;
        CHECK_FALSE(tempFile.isOpen());
        
        bool opened = tempFile.open();
        CHECK(opened);
        CHECK(tempFile.isOpen());
        CHECK(tempFile.fd() >= 0);
        CHECK_FALSE(tempFile.path().empty());
        
        // Verify file exists
        struct stat st;
        CHECK(stat(tempFile.path().c_str(), &st) == 0);
    }

    SUBCASE("Write to temp file") {
        TempFile tempFile;
        tempFile.open();
        
        const char* data = "test data";
        ssize_t written = write(tempFile.fd(), data, strlen(data));
        CHECK(written == static_cast<ssize_t>(strlen(data)));
    }

    SUBCASE("Close temp file") {
        TempFile tempFile;
        tempFile.open();
        std::string path = tempFile.path();
        
        tempFile.close();
        CHECK_FALSE(tempFile.isOpen());
        CHECK(tempFile.fd() == -1);
        
        // Verify file is deleted
        struct stat st;
        CHECK(stat(path.c_str(), &st) != 0);
    }

    SUBCASE("Destructor cleanup") {
        std::string path;
        {
            TempFile tempFile;
            tempFile.open();
            path = tempFile.path();
            CHECK(tempFile.isOpen());
        }
        // File should be deleted after destructor
        struct stat st;
        CHECK(stat(path.c_str(), &st) != 0);
    }

    SUBCASE("Reopen temp file") {
        TempFile tempFile;
        tempFile.open();
        std::string firstPath = tempFile.path();
        
        tempFile.open();
        std::string secondPath = tempFile.path();
        
        CHECK(firstPath != secondPath);
        CHECK(tempFile.isOpen());
    }

    SUBCASE("Operator int conversion") {
        TempFile tempFile;
        tempFile.open();
        
        int fd = tempFile;
        CHECK(fd >= 0);
        CHECK(fd == tempFile.fd());
    }
}
