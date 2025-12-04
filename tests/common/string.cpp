#include "common/string.hpp"
#include "doctest.h"

using namespace utils;

TEST_CASE("String utilities - trim") {
    SUBCASE("No whitespace") {
        CHECK(trim("hello") == "hello");
        CHECK(trim("test123") == "test123");
    }

    SUBCASE("Leading whitespace") {
        CHECK(trim("  hello") == "hello");
        CHECK(trim("\thello") == "hello");
        CHECK(trim("\nhello") == "hello");
        CHECK(trim("\r\nhello") == "hello");
        CHECK(trim("   \t  hello") == "hello");
    }

    SUBCASE("Trailing whitespace") {
        CHECK(trim("hello  ") == "hello");
        CHECK(trim("hello\t") == "hello");
        CHECK(trim("hello\n") == "hello");
        CHECK(trim("hello\r\n") == "hello");
        CHECK(trim("hello   \t  ") == "hello");
    }

    SUBCASE("Both leading and trailing whitespace") {
        CHECK(trim("  hello  ") == "hello");
        CHECK(trim("\thello\t") == "hello");
        CHECK(trim("  \t hello world \r\n  ") == "hello world");
    }

    SUBCASE("Whitespace in the middle") {
        CHECK(trim("hello world") == "hello world");
        CHECK(trim("  hello world  ") == "hello world");
        CHECK(trim("\thello\tworld\n") == "hello\tworld");
    }

    SUBCASE("Only whitespace") {
        CHECK(trim("   ") == "");
        CHECK(trim("\t\t") == "");
        CHECK(trim("\r\n") == "");
        CHECK(trim("  \t\r\n  ") == "");
    }

    SUBCASE("Empty string") {
        CHECK(trim("") == "");
    }

    SUBCASE("Single character") {
        CHECK(trim("a") == "a");
        CHECK(trim(" a ") == "a");
    }
}

TEST_CASE("String utilities - isAllDigit") {
    SUBCASE("All digits") {
        CHECK(isAllDigit("0"));
        CHECK(isAllDigit("1"));
        CHECK(isAllDigit("123"));
        CHECK(isAllDigit("456789"));
        CHECK(isAllDigit("0000"));
        CHECK(isAllDigit("9876543210"));
    }

    SUBCASE("Contains non-digits") {
        CHECK_FALSE(isAllDigit("a"));
        CHECK_FALSE(isAllDigit("123a"));
        CHECK_FALSE(isAllDigit("a123"));
        CHECK_FALSE(isAllDigit("12a34"));
        CHECK_FALSE(isAllDigit("12.34"));
        CHECK_FALSE(isAllDigit("12-34"));
        CHECK_FALSE(isAllDigit("12 34"));
    }

    SUBCASE("Empty string") {
        // Edge case: empty string should technically return true
        // since there are no non-digit characters
        CHECK(isAllDigit(""));
    }

    SUBCASE("Special characters") {
        CHECK_FALSE(isAllDigit("+123"));
        CHECK_FALSE(isAllDigit("-123"));
        CHECK_FALSE(isAllDigit("123!"));
    }

    SUBCASE("Whitespace") {
        CHECK_FALSE(isAllDigit(" 123"));
        CHECK_FALSE(isAllDigit("123 "));
        CHECK_FALSE(isAllDigit("12 3"));
        CHECK_FALSE(isAllDigit("\t123"));
        CHECK_FALSE(isAllDigit("123\n"));
    }
}

TEST_CASE("String utilities - toString") {
    SUBCASE("Integer to string") {
        CHECK(toString(0) == "0");
        CHECK(toString(123) == "123");
        CHECK(toString(-456) == "-456");
    }

    SUBCASE("Float to string") {
        CHECK(toString(3.14).find("3.14") != std::string::npos);
        CHECK(toString(0.0) == "0");
    }

    SUBCASE("Boolean to string") {
        CHECK(toString(true) == "1");
        CHECK(toString(false) == "0");
    }

    SUBCASE("String to string") {
        CHECK(toString(std::string("hello")) == "hello");
        CHECK(toString(std::string("")) == "");
    }
}

TEST_CASE("String utilities - fromString") {
    SUBCASE("String to integer") {
        CHECK(fromString<int>("123") == 123);
        CHECK(fromString<int>("0") == 0);
        CHECK(fromString<int>("-456") == -456);
    }

    SUBCASE("String to float") {
        CHECK(fromString<double>("3.14") == doctest::Approx(3.14));
        CHECK(fromString<double>("0.0") == 0.0);
    }

    SUBCASE("String to string") {
        CHECK(fromString<std::string>("hello") == "hello");
    }

    SUBCASE("Invalid conversions") {
        // These should fail to convert properly
        int result = fromString<int>("abc");
        CHECK(result == 0); // Default value when conversion fails
    }
}
