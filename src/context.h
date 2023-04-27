/*
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#pragma once

#include <memory>

struct Context {
  virtual int system(const char *command) = 0;
  virtual int socket(int domain, int type, int protocol) = 0;
  virtual int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = 0;

  virtual int ioctl3(int fd, unsigned long request, void *arg) = 0;

  template <typename... Args> int ioctl(int fd, unsigned long request, Args... args) {
    switch (sizeof...(Args)) {
    case 1:
      return (ioctl3(fd, request, args), ...);
    default:
      std::abort();
    }
  }

  virtual ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                         const struct sockaddr *dest_addr, socklen_t addrlen) = 0;

  virtual int close(int fd) = 0;
  virtual ~Context() = default;
};

std::unique_ptr<Context> makeContext();
