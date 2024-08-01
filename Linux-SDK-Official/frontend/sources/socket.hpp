/**
 * @file socket.hpp
 * @brief Header file of the SocketServer and SocketClient.
 */

#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

#include <poll.h>
#include <sys/types.h>
#include <sys/un.h>

namespace com {
namespace sony {
namespace imaging {
namespace remote {

class SocketClient {
 private:
  int mfd;
  struct sockaddr_un mAddr;
  size_t mAddrLen;
  struct pollfd *fds;

 public:
  SocketClient(char *name);
  ~SocketClient();
  bool connect();
  void prepare_hup(struct pollfd *fds);
  bool is_hup();
  int getCommFD();
  ssize_t write(const void *buf, size_t count);
};

class SocketServer {
 private:
  int listen_fd;
  int comm_fd;
  struct sockaddr_un mAddr;
  size_t mAddrLen;
  struct pollfd *fds;
  bool used;

 public:
  SocketServer(char *name);
  ~SocketServer();
  void prepare_accept(struct pollfd *fds);
  void prepare_wait_data(struct pollfd *fds);
  bool is_recv_data();
  bool is_connect();
  int getCommFD();
  void disconnect();
  void accept();
  bool is_accepted();
  bool is_request_connect();
  bool async_mode();
  ssize_t read(void *buf, size_t count);
};

}  // namespace remote
}  // namespace imaging
}  // namespace sony
}  // namespace com

#endif  // __SOCKET_HPP__
