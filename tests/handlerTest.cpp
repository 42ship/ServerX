#include "doctest.h"

#include "http/Handler.hpp"
#include "ServerConfig.hpp"
#include "http/Response.hpp"
#include "test_utils.hpp"
#include "unistd.h"

TEST_CASE("StaticFileHandler test case")
{
    mkdir("handler_test_files", 0777);
    try
    {
        config::ServerConfig cfg("config/example.conf");
        MimeTypes *mime = new MimeTypes();
        http::HttpRequest req;
        
        writeFile("<!DOCTYPE html><html><head><title>Example</title></head><body><p>This is an example of a simple HTML page with one paragraph.</p></body></html>", "handler_test_files/index.html");
        writeFile("<!DOCTYPE html><html><head><title>Example</title></head><body><p>This is an example of a simple HTML page with one paragraph.</p></body></html>", "handler_test_files/default.html");
        
        req = req.parse("GET /index.html HTTP/1.1\nHost: localhost:9191\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:142.0) Gecko/20100101 Firefox/142.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate, br, zstd\nConnection: keep-alive\nUpgrade-Insecure-Requests: 1\nSec-Fetch-Dest: document\nSec-Fetch-Mode: navigate\nSec-Fetch-Site: none\nSec-Fetch-User: ?1\nPriority: u=0, i");
        http::StaticFileHandler h1;
        const config::ServerBlock *s;
        cfg.getServer(9191, "localhost", s);
        const config::LocationBlock *l;
        l = s->getLocation("/");
        http::Response res;
        res = h1.handle(req, s, l, mime);
        CHECK(res.getStatusCode().getCode() == http::OK);

        chmod("handler_test_files/index.html", 0222);
        res = h1.handle(req, s, l, mime);
        CHECK(res.getStatusCode().getCode() == http::FORBIDDEN);
        
        http::HttpRequest req1 = req.parse("GET / HTTP/1.1\nHost: localhost:9191\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:142.0) Gecko/20100101 Firefox/142.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate, br, zstd\nConnection: keep-alive\nUpgrade-Insecure-Requests: 1\nSec-Fetch-Dest: document\nSec-Fetch-Mode: navigate\nSec-Fetch-Site: none\nSec-Fetch-User: ?1\nPriority: u=0, i");
        res = h1.handle(req1, s, l, mime);
        CHECK(res.getStatusCode().getCode() == http::OK);

        unlink("handler_test_files/index.html");
        res = h1.handle(req, s, l, mime);
        CHECK(res.getStatusCode().getCode() == http::NOT_FOUND);

        config::LocationBlock *tmp = new config::LocationBlock();
        tmp->path = "/";
        tmp->root = "./handler_test_files";
        tmp->index.clear();
        res = h1.handle(req1, s, tmp, mime);
        CHECK(res.getStatusCode().getCode() == http::NOT_FOUND);
        
        delete mime;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    removeDirectoryRecursive("handler_test_files");
}