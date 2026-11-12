#pragma once

#include "fd.hpp"

namespace redis {
  void handleClient(UniqueFd client_fd);
}
