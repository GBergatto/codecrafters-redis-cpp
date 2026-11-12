#include <iostream>
#include <unistd.h>

#include "fd.hpp"
#include "server.hpp"

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  redis::UniqueFd server_fd = redis::createServerSocket();
  if (!server_fd) {
    return 1;
  }

  redis::runServer(server_fd);

  return 0;
}
