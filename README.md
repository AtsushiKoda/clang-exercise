# clang-exercise
## TCP/IP Client Quick Start

- server
    ```sh
    user@cbbc57f4a060:/workspaces/clang_exercise$ python3 -m http.server 8080
    Serving HTTP on 0.0.0.0 port 8080 (http://0.0.0.0:8080/) ...
    ```
- client  
    ```sh
    user@cbbc57f4a060:/workspaces/clang_exercise$ make build
    user@cbbc57f4a060:/workspaces/clang_exercise$ ./build/webclinet 0.0.0.0 8080 /
    HTTP/1.0 200 OK
    Server: SimpleHTTP/0.6 Python/3.9.0
    Date: Sun, 22 Jan 2023 16:11:41 GMT
    Content-type: text/html; charset=utf-8
    Content-Length: 758
    ```