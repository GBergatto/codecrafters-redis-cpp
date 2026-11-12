#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

#include "client.hpp"

namespace redis {

  void handleClient(UniqueFd client_fd) {
    const char *response = "+PONG\r\n";
    send(client_fd.get(), response, strlen(response), 0);
  }

}
