#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <utility>

#include "server.hpp"
#include "client.hpp"

namespace redis {

  UniqueFd createServerSocket() {
    UniqueFd server_fd{socket(AF_INET, SOCK_STREAM, 0)};

    if (!server_fd) {
      std::cerr << "Failed to create server socket\n";
      return {};
    }

    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd.get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
      std::cerr << "setsockopt failed\n";
      return {};
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(REDIS_PORT);

    if (bind(server_fd.get(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) != 0) {
      std::cerr << "Failed to bind to port " << REDIS_PORT << "\n";
      return {};
    }

    int connection_backlog = 5;
    if (listen(server_fd.get(), connection_backlog) != 0) {
      std::cerr << "listen failed\n";
      return {};
    }

    return server_fd;
  }

  void runServer(const UniqueFd& server_fd) {
    while (true) {
      struct sockaddr_in client_addr{};
      socklen_t client_addr_len = sizeof(client_addr);
      std::cout << "Waiting for a client to connect...\n";

      UniqueFd client_fd{accept(server_fd.get(), reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len)};
      if (!client_fd) {
        std::cerr << "accept failed";
        return;
      }
      std::cout << "Client connected\n";

      // Transfer ownership of client fd
      handleClient(std::move(client_fd));
    }
  }

}
