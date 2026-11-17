#include <iostream>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unordered_map>
#include <utility>
#include <unistd.h>

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
    std::unordered_map<int, UniqueFd> clients;

    UniqueFd epoll_fd{epoll_create1(EPOLL_CLOEXEC)};

    if (!epoll_fd) {
      std::cerr << "epoll_create1 failed\n";
      return;
    }

    // Monitor server socket
    epoll_event server_event{};
    server_event.events = EPOLLIN; // server socket is readable = client waiting
    server_event.data.fd = server_fd.get();

    if (epoll_ctl(
          epoll_fd.get(),
          EPOLL_CTL_ADD,
          server_fd.get(),
          &server_event
          ) < 0) {
      std::cerr << "epoll_ctl failed\n";
      return;
    }

    // Create buffer for reporting events on sockets
    constexpr int MAX_EVENTS = 10;
    epoll_event events[MAX_EVENTS];

    while (true) {
      int num_events = epoll_wait(
          epoll_fd.get(),
          events,
          MAX_EVENTS,
          -1
          );

      if (num_events < 0) {
        std::cerr << "epoll_wait failed\n";
        return;
      }

      // Loop on received events
      for (int i = 0; i < num_events; ++i) {
        int fd = events[i].data.fd;

        if (fd == server_fd.get()) {
          struct sockaddr_in client_addr{};
          socklen_t client_addr_len = sizeof(client_addr);

          // Accept client connection
          UniqueFd client_fd{accept(server_fd.get(), reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len)};
          if (!client_fd) {
            std::cerr << "accept failed";
            return;
          }
          std::cout << "Client connected fd=" << client_fd.get() << "\n";

          // Monitor client socket
          epoll_event client_event{};
          client_event.events = EPOLLIN; // client socket is readable = data available
          client_event.data.fd = client_fd.get();

          if (epoll_ctl(
                epoll_fd.get(),
                EPOLL_CTL_ADD,
                client_fd.get(),
                &client_event
                ) < 0) {
            std::cerr << "epoll_ctl failed\n";
            return;
          }

          // Transfer ownership of client socket UniqueFd to clients map
          clients.emplace(client_fd.get(), std::move(client_fd));

        } else { // event on client socket
          auto it = clients.find(fd);

          if (it == clients.end()) {
            std::cerr << "Unknown client fd\n";
            continue;
          }

          if (!handleClient(it->second)) {
            epoll_ctl(epoll_fd.get(), EPOLL_CTL_DEL, fd, nullptr);
            clients.erase(fd);
          }
        }
      }
    }
  }

}
