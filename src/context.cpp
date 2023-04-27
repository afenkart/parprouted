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

#include "context.h"

#include <cstdlib>
#include <sys/ioctl.h>
#include <sys/socket.h>

namespace {

class ContextImpl final : public Context {
  int system(const char *command) override { return ::system(command); }
  int socket(int domain, int type, int protocol) override {
    return ::socket(domain, type, protocol);
  }
  int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) override {
    return ::bind(sockfd, addr, addrlen);
  }
  ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                 const struct sockaddr *dest_addr, socklen_t addrlen) override {
    return ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
  }
  int close(int fd) override { return ::close(fd); }

  int ioctl3(int fd, unsigned long request, void *arg) override {
    return ::ioctl(fd, request, arg);
  }
};

} // namespace

std::unique_ptr<Context> makeContext() { return std::make_unique<ContextImpl>(); };
