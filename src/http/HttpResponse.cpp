#include <http/HttpResponse.hpp>

namespace http {

std::ostream &operator<<(std::ostream &o, HttpResponse const &r) {
    o << "version(" << r.version << "); status(" << r.status << "); phrase(" << r.responsePhrase
      << ")";
    return o;
}

} // namespace http
