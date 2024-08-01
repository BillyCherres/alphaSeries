#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <utime.h>

#include "socket.hpp"

using namespace com::sony::imaging::remote;

SocketServer::SocketServer(char *name) {
  listen_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  mAddrLen = 0;
  memset(&mAddr, 0, sizeof(mAddr));
  mAddrLen += snprintf(mAddr.sun_path, sizeof(mAddr.sun_path), "%s", name);
  mAddr.sun_family = AF_LOCAL;
  mAddrLen += offsetof(struct sockaddr_un, sun_path);
  unlink(mAddr.sun_path);
  int n = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));
  bind(listen_fd, (struct sockaddr *)&mAddr, mAddrLen);
  listen(listen_fd, 4);
  comm_fd = -1;
  used = false;
}

void SocketServer::prepare_accept(struct pollfd *fds) {
  this->fds = fds;
  fds->fd = is_accepted() ? -1 : listen_fd;
  fds->events = POLLIN;
  fds->revents = 0;
}

void SocketServer::prepare_wait_data(struct pollfd *fds) {
  this->fds = fds;
  fds->fd = comm_fd;
  fds->events = POLLIN | POLLHUP;
  fds->revents = 0;
}

bool SocketServer::is_recv_data() { return fds->revents & POLLIN; }

bool SocketServer::is_connect() {
  bool ret = true;
  if (-1 == comm_fd) {
    ret = false;
  } else if (NULL != fds && fds->revents & POLLHUP) {
    ret = false;
  }

  return ret;
}

int SocketServer::getCommFD() { return comm_fd; }

void SocketServer::disconnect() {
  if (-1 != comm_fd) {
    close(comm_fd);
    comm_fd = -1;
  }
}

void SocketServer::accept() {
  comm_fd = ::accept(listen_fd, NULL, NULL);
  used = true;
}

void SocketServer::async_mode() { fcntl(comm_fd, F_SETFL, O_NONBLOCK); }

bool SocketServer::is_accepted() { return -1 != comm_fd; }

bool SocketServer::is_request_connect() { return fds->revents & POLLIN; }

ssize_t SocketServer::read(void *buf, size_t count) {
  ssize_t size = ::read(getCommFD(), buf, count);
  if (size == 0) {
    disconnect();
  }
  if (size < 0) {
    size = 0;
  }
  return size;
}

SocketServer::~SocketServer() {
  if (used) {
    unlink(mAddr.sun_path);
  }
  if (-1 != listen_fd) {
    close(listen_fd);
  }
  if (-1 != comm_fd) {
    close(comm_fd);
  }
}

SocketClient::SocketClient(char *name) {
  mfd = socket(AF_LOCAL, SOCK_STREAM, 0);
  mAddrLen = 0;
  memset(&mAddr, 0, sizeof(mAddr));
  mAddrLen += snprintf(mAddr.sun_path, sizeof(mAddr.sun_path), "%s", name);
  mAddr.sun_family = AF_LOCAL;
  mAddrLen += offsetof(struct sockaddr_un, sun_path);
}

int SocketClient::getCommFD() { return mfd; }

SocketClient::~SocketClient() { close(mfd); }

bool SocketClient::connect() {
  return 0 == ::connect(mfd, (struct sockaddr *)&mAddr, mAddrLen);
}

void SocketClient::prepare_hup(struct pollfd *fds) {
  this->fds = fds;
  fds->fd = mfd;
  fds->events = POLLHUP;
  fds->revents = 0;
}

bool SocketClient::is_hup() { return fds->revents & POLLHUP; }

ssize_t SocketClient::write(const void *buf, size_t count) {
  return ::write(getCommFD(), buf, count);
}
