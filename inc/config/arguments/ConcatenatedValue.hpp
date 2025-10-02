#pragma once

#include "IArgument.hpp"
#include <vector>

namespace config {

class ConcatenatedValue : public IArgument {
public:
    std::string evaluate(http::HttpRequest const &ctx) const {
        std::string res;
        for (size_t i = 0; i < variables_.size(); i++) {
            if (variables_[i])
                res += variables_[i]->evaluate(ctx);
        }
        return res;
    }

private:
    std::vector<ArgumentPtr> variables_;
};

} // namespace config
