import socket
import subprocess
import time


def test_chunked_upload(host, port, path):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((host, port))
    except ConnectionRefusedError:
        print(f"Error: Could not connect to {host}:{port}. Is the server running?")
        return

    headers = (
        f"POST {path} HTTP/1.1\r\n"
        f"Host: {host}\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n"
        "\r\n"
    )
    sock.sendall(headers.encode())

    chunks = ["Hello ", "this is ", "a slow ", "chunked ", "request!"]

    for chunk in chunks:
        chunk_len = hex(len(chunk))[2:]
        payload = f"{chunk_len}\r\n{chunk}\r\n"

        print(f"Sending chunk: {repr(chunk)} ({chunk_len} bytes)")
        sock.sendall(payload.encode())

        time.sleep(1)

    print("Sending final terminator chunk...")
    sock.sendall(b"0\r\n\r\n")

    response = b""
    while True:
        data = sock.recv(4096)
        if not data:
            break
        response += data

    print("\n--- Server Response ---")
    print(response.decode(errors="ignore"))

    sock.close()


def run_test_with_server():
    # Example config for testing
    config = """
server {
    listen localhost:9292;
    root website;
    location /cgi/ {
        cgi_pass;
    }
}
"""
    print("Starting webserv with custom config via stdin...")
    # Start webserv in a subprocess
    process = subprocess.Popen(
        ["./webserv", "-"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    # Send config to stdin
    process.stdin.write(config)
    process.stdin.close()

    # Give the server a moment to start
    time.sleep(1)

    try:
        test_chunked_upload("127.0.0.1", 9292, "/cgi/test.sh")
    finally:
        print("Shutting down webserv...")
        process.terminate()
        process.wait()


if __name__ == "__main__":
    # You can now run the server automatically for the test
    run_test_with_server()

    # Or run against an already running server:
    # SERVER_HOST = "127.0.0.1"
    # SERVER_PORT = 9191
    # ENDPOINT = "/cgi/test.sh"
    # test_chunked_upload(SERVER_HOST, SERVER_PORT, ENDPOINT)
