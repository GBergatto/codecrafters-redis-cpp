#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.hpp"

namespace redis {

  /*
   * Processes incoming data from a connected client and responds to it.
   *
   * @param client_fd Reference to the client's socket.
   *
   * @return true if the connection is still open ; false if the connection has
   * been closed or an error occurred and the client should be removed from epoll.
   */
  bool handleClient(const UniqueFd& client_fd) {
    char buffer[1024];
    ssize_t bytes_received = recv(client_fd.get(), buffer, sizeof(buffer), 0);

    if (bytes_received == 0) {
      std::cerr << "Client disconnected\n";
      return false;
    } else if (bytes_received < 0) {
      std::cerr << "recv failed\n";
      return false;
    }

    const char *response = "+PONG\r\n";
    ssize_t bytes_sent = send(client_fd.get(), response, strlen(response), 0);

    if (bytes_sent < 0) {
      std::cerr << "send failed\n";
      return false;
    }

    return true;
  }

}
