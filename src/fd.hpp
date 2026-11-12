#pragma once

#include <unistd.h>
#include <utility>

namespace redis {

  /**
   * RAII wrapper for an owned Linux file descriptor.
   *
   * Ensures that the file descriptor is closed when the object is destoyed.
   * Ownership is exclusive: copying is disabled, while move operations
   * transfer ownership to another UniqueFd instance.
   *
   * A value of -1 represents an empty or invalid file descriptor.
   */
  class UniqueFd {
    public:
      UniqueFd() noexcept : fd_(-1) {}
      explicit UniqueFd(int fd) noexcept : fd_(fd) {}

      ~UniqueFd() {
        if (fd_ >= 0) {
          close(fd_);
        }
      }

      // Disable copy constructor and operator
      UniqueFd(const UniqueFd&) = delete;
      UniqueFd& operator=(const UniqueFd&) = delete;

      // Transfer ownership from another UniqueFd
      UniqueFd(UniqueFd&& other) noexcept
        : fd_(std::exchange(other.fd_, -1)) {}

      UniqueFd& operator=(UniqueFd&& other) noexcept {
        if (this != &other) {
          if (fd_ >= 0) {
            close(fd_);
          }

          fd_ = std::exchange(other.fd_, -1);
        }

        return *this;
      }

      int get() const noexcept {
        return fd_;
      }

      explicit operator bool() const noexcept {
        return fd_ >= 0;
      }

    private:
      int fd_;
  };

}

