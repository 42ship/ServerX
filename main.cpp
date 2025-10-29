#include "HttpRequestParser.cpp"

int main() {
    RequestParser reqParser;
    reqParser.addIncomingChunk("Hello world", 11);
    return 0;
}
