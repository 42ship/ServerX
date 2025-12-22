#include "config/directives/ReturnDirective.hpp"
#include "config/Block.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include "doctest.h"

TEST_CASE("ReturnDirective") {
    config::ReturnDirective directive;
    config::Block block("location");

    SUBCASE("Should throw on empty arguments") {
        config::ParsedDirectiveArgs empty_args;
        CHECK_THROWS_AS(directive.process(block, empty_args), const config::ConfigError &);
    }

    SUBCASE("Should throw on too many arguments") {
        config::ParsedDirectiveArgs too_many_args;
        config::Token arg1 = {"301", config::NUMBER};
        config::Token arg2 = {"http://example.com", config::STRING};
        config::Token arg3 = {"extra", config::STRING};
        too_many_args.push_back(arg1);
        too_many_args.push_back(arg2);
        too_many_args.push_back(arg3);
        
        CHECK_THROWS_AS(directive.process(block, too_many_args), const config::ConfigError &);
    }

    SUBCASE("Should accept single argument (URL)") {
        config::ParsedDirectiveArgs single_arg;
        config::Token url_arg = {"http://example.com", config::STRING};
        single_arg.push_back(url_arg);
        
        CHECK_NOTHROW(directive.process(block, single_arg));
        CHECK(block.has("return"));
        
        // Verify the argument was stored
        std::vector<std::string> raw_values = block.getRawValues("return");
        CHECK(raw_values.size() == 1);
        CHECK(raw_values[0] == "http://example.com");
    }

    SUBCASE("Should accept single argument (text)") {
        config::Block block2("location");
        config::ParsedDirectiveArgs single_arg;
        config::Token text_arg = {"Some text response", config::STRING};
        single_arg.push_back(text_arg);
        
        CHECK_NOTHROW(directive.process(block2, single_arg));
        CHECK(block2.has("return"));
    }

    SUBCASE("Should accept two arguments with valid status code (301)") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"301", config::NUMBER};
        config::Token url_arg = {"http://example.com", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(url_arg);
        
        CHECK_NOTHROW(directive.process(block2, two_args));
        CHECK(block2.has("return"));
        
        // Verify the arguments were stored
        std::vector<std::string> raw_values = block2.getRawValues("return");
        CHECK(raw_values.size() == 2);
        CHECK(raw_values[0] == "301");
        CHECK(raw_values[1] == "http://example.com");
    }

    SUBCASE("Should accept two arguments with valid status code (200)") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"200", config::NUMBER};
        config::Token text_arg = {"Success", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(text_arg);
        
        CHECK_NOTHROW(directive.process(block2, two_args));
        CHECK(block2.has("return"));
    }

    SUBCASE("Should accept two arguments with valid status code (404)") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"404", config::NUMBER};
        config::Token text_arg = {"Not Found", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(text_arg);
        
        CHECK_NOTHROW(directive.process(block2, two_args));
        CHECK(block2.has("return"));
    }

    SUBCASE("Should accept two arguments with valid status code (500)") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"500", config::NUMBER};
        config::Token text_arg = {"Internal Server Error", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(text_arg);
        
        CHECK_NOTHROW(directive.process(block2, two_args));
        CHECK(block2.has("return"));
    }

    SUBCASE("Should accept two arguments with boundary status code (100)") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"100", config::NUMBER};
        config::Token text_arg = {"Continue", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(text_arg);
        
        CHECK_NOTHROW(directive.process(block2, two_args));
        CHECK(block2.has("return"));
    }

    SUBCASE("Should accept two arguments with boundary status code (999)") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"999", config::NUMBER};
        config::Token text_arg = {"Custom", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(text_arg);
        
        CHECK_NOTHROW(directive.process(block2, two_args));
        CHECK(block2.has("return"));
    }

    SUBCASE("Should throw on non-numeric first argument when two arguments provided") {
        config::ParsedDirectiveArgs two_args;
        config::Token non_numeric_arg = {"not_a_number", config::STRING};
        config::Token url_arg = {"http://example.com", config::STRING};
        two_args.push_back(non_numeric_arg);
        two_args.push_back(url_arg);
        
        CHECK_THROWS_AS(directive.process(block, two_args), const config::ConfigError &);
    }

    SUBCASE("Should throw on non-numeric first argument (IDENTIFIER type)") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token non_numeric_arg = {"text", config::IDENTIFIER};
        config::Token url_arg = {"http://example.com", config::STRING};
        two_args.push_back(non_numeric_arg);
        two_args.push_back(url_arg);
        
        CHECK_THROWS_AS(directive.process(block2, two_args), const config::ConfigError &);
    }

    SUBCASE("Should throw on invalid status code less than 100") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"99", config::NUMBER};
        config::Token url_arg = {"http://example.com", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(url_arg);
        
        CHECK_THROWS_AS(directive.process(block2, two_args), const config::ConfigError &);
    }

    SUBCASE("Should throw on invalid status code of 0") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"0", config::NUMBER};
        config::Token url_arg = {"http://example.com", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(url_arg);
        
        CHECK_THROWS_AS(directive.process(block2, two_args), const config::ConfigError &);
    }

    SUBCASE("Should throw on invalid status code greater than or equal to 1000") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"1000", config::NUMBER};
        config::Token url_arg = {"http://example.com", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(url_arg);
        
        CHECK_THROWS_AS(directive.process(block2, two_args), const config::ConfigError &);
    }

    SUBCASE("Should throw on invalid status code of 1001") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"1001", config::NUMBER};
        config::Token url_arg = {"http://example.com", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(url_arg);
        
        CHECK_THROWS_AS(directive.process(block2, two_args), const config::ConfigError &);
    }

    SUBCASE("Should throw on negative status code") {
        config::Block block2("location");
        config::ParsedDirectiveArgs two_args;
        config::Token status_arg = {"-1", config::NUMBER};
        config::Token url_arg = {"http://example.com", config::STRING};
        two_args.push_back(status_arg);
        two_args.push_back(url_arg);
        
        CHECK_THROWS_AS(directive.process(block2, two_args), const config::ConfigError &);
    }

    SUBCASE("Should verify directive name is 'return'") {
        CHECK(directive.getName() == "return");
    }
}
