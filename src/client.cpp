#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.hpp"

namespace redis {

  void handleClient(UniqueFd client_fd) {
    while (true) {
      char buffer[1024];
      ssize_t bytes_received = recv(client_fd.get(), buffer, sizeof(buffer), 0);

      if (bytes_received == 0) {
        std::cerr << "Client disconnected\n";
        return;
      } else if (bytes_received < 0) {
        std::cerr << "recv failed\n";
        return;
      }

      const char *response = "+PONG\r\n";
      ssize_t bytes_sent = send(client_fd.get(), response, strlen(response), 0);

      if (bytes_sent < 0) {
        std::cerr << "send failed\n";
        return;
      }
    }
  }

}
