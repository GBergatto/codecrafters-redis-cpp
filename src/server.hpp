#pragma once

#include "fd.hpp"

#define REDIS_PORT 6379

namespace redis {
  UniqueFd createServerSocket();
  void runServer(const UniqueFd& server_fd);
}
