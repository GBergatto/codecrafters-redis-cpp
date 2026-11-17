#pragma once

#include "fd.hpp"

namespace redis {
  bool handleClient(const UniqueFd& client_fd);
}
