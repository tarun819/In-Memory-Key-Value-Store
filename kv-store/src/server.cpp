#include "server.h"
#include <iostream>
#include <string>

#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <unistd.h>
#endif

// ─── Phase 3: Echo server — just sends back whatever the client types ─────────

void handle_connection(socket_t client_fd) {
    char buf[4096];
    std::string leftover;

    std::cout << "[Client connected]\n";

    while (true) {
        // Read raw bytes from socket
        int n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;  // client disconnected or error

        buf[n] = '\0';
        leftover += buf;

        // Process complete lines (delimited by '\n')
        size_t pos;
        while ((pos = leftover.find('\n')) != std::string::npos) {
            std::string line = leftover.substr(0, pos);
            leftover = leftover.substr(pos + 1);

            // Remove '\r' if present (telnet sends \r\n)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.empty()) continue;

            // Phase 3: just echo back
            std::string response = "ECHO: " + line + "\n";
            send(client_fd, response.c_str(), (int)response.size(), 0);

            std::cout << "  [Recv] " << line << "\n";
        }
    }

    std::cout << "[Client disconnected]\n";

#ifdef _WIN32
    closesocket(client_fd);
#else
    close(client_fd);
#endif
}
