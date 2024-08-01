#include "serverclient.h"

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

#include "command.h"
#include "parser.h"
#include "socket.hpp"

using namespace com::sony::imaging::remote;

static int open_output_file(char *filename, int flag) {
  int outfd = STDOUT_FILENO;
  if (0 == strncmp("-", filename, FILENAME_MAX_LEN)) {
    outfd = STDOUT_FILENO;
  } else if (0 != strnlen(filename, FILENAME_MAX_LEN)) {
    outfd = open(filename, O_WRONLY | O_CREAT | flag,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (-1 == outfd) {
      fprintf(stderr, "err %s: %s\n", filename, strerror(errno));
      outfd = STDOUT_FILENO;
    }
  }

  return outfd;
}

static void close_output_file(int fd) {
  if (STDOUT_FILENO != fd) {
    close(fd);
  }
}

#define SOCKET_NAME_MAX_LEN 100
int com::sony::imaging::remote::client(
    SocketClient *serverport, char *logfile, char *outfile, int command,
    com::sony::imaging::remote::PTPTransaction *transaction,
    uint16_t device_property_code, uint32_t handle) {
  char out_server2client[SOCKET_NAME_MAX_LEN];
  snprintf(out_server2client, SOCKET_NAME_MAX_LEN, "s2c%dout", getpid());
  SocketServer *out = new SocketServer(out_server2client);

  char log_server2client[SOCKET_NAME_MAX_LEN];
  snprintf(log_server2client, SOCKET_NAME_MAX_LEN, "s2c%dlog", getpid());
  SocketServer *log = new SocketServer(log_server2client);

  int outfd = -1;
  int logfd = -1;

  serverport->write(log_server2client, SOCKET_NAME_MAX_LEN);
  serverport->write(out_server2client, SOCKET_NAME_MAX_LEN);
  serverport->write(&command, sizeof(command));
  serverport->write(transaction,
                    sizeof(com::sony::imaging::remote::PTPTransaction));
  serverport->write(&device_property_code, sizeof(device_property_code));
  serverport->write(&handle, sizeof(handle));

  size_t buf_size = 1024 * 1024;
  char *buf = new char[buf_size];

  // connect from client
  struct pollfd fds[3];
  do {
    out->prepare_accept(&fds[0]);
    log->prepare_accept(&fds[1]);
    serverport->prepare_hup(&fds[2]);
    if (poll(fds, 3, -1) < 0) {
      break;
    };
    if (true == out->is_request_connect()) {
      out->accept();
    }
    if (true == log->is_request_connect()) {
      log->accept();
    }
    if (true == serverport->is_hup()) {
      delete[] buf;
      goto bail;
    }
  } while (false == out->is_accepted() || false == log->is_accepted());
  out->async_mode();
  log->async_mode();

  // open output files
  outfd = open_output_file(outfile, O_TRUNC);
  logfd = open_output_file(logfile, O_SYNC | O_APPEND);

  ssize_t read_size;
  do {
    out->prepare_wait_data(&fds[0]);
    log->prepare_wait_data(&fds[1]);
    if (poll(fds, 2, -1) < 0) {
      break;
    };
    if (true == out->is_recv_data()) {
      do {
        read_size = out->read(buf, buf_size);
        if (read_size < 0) {
          break;
        }
        write(outfd, buf, read_size);
      } while (0 < read_size);
    }
    if (true == log->is_recv_data()) {
      do {
        read_size = log->read(buf, buf_size);
        if (read_size < 0) {
          break;
        }
        write(logfd, buf, read_size);
      } while (0 < read_size);
    }

    if (!out->is_connect()) {
      out->disconnect();
    }
    if (!log->is_connect()) {
      log->disconnect();
    }
  } while (true == out->is_connect() || true == log->is_connect());

  close_output_file(outfd);
  close_output_file(logfd);
  delete[] buf;
bail:
  delete out;
  delete log;

  return 0;
}

SocketClient *com::sony::imaging::remote::server_create(int busn, int devn) {
  char socket_name[SOCKET_NAME_MAX_LEN];
  snprintf(socket_name, SOCKET_NAME_MAX_LEN, "c2s%03d%03d", busn, devn);
  SocketClient *client = new SocketClient(socket_name);

  if (false == client->connect()) {
    SocketServer *serverport = new SocketServer(socket_name);
    if (0 == fork()) {
      delete client;
      server(busn, devn, serverport);
      delete serverport;
      exit(0);
    }
    delete serverport;
    if (false == client->connect()) {
      fprintf(stderr, "cannot connect server: %s\n", strerror(errno));
      exit(0);
    }
  }

  return client;
}

static void hotplug_callback(socc_hotplug_event_t event, void *data) {
  int *fd = (int *)data;
  if (SOCC_HOTPLUG_EVENT_REMOVED == event) {
    close(*fd);
    *fd = -1;
  }
}

void com::sony::imaging::remote::server(int busn, int devn,
                                        SocketServer *serverport) {
#if 0
    int wait = 1;
    while(wait) {
        sleep(1);
    }
#endif
  int command = 0;
  com::sony::imaging::remote::PTPTransaction transaction;
  uint16_t device_property_code;
  uint32_t handle;
  char outfilename[SOCKET_NAME_MAX_LEN];
  outfilename[0] = 0;
  char logfilename[SOCKET_NAME_MAX_LEN];
  logfilename[0] = 0;
  int pipefd[2];
  com::sony::imaging::remote::socc_ptp *ptp = NULL;

  ptp = new com::sony::imaging::remote::socc_ptp(busn, devn);
  if (NULL == ptp) {
    return;
  }
  pipe(pipefd);
  ptp->set_hotplug_callback(hotplug_callback, &pipefd[1]);

  if (SOCC_OK != ptp->connect()) {
    fprintf(stderr, "cannot connect to the camera\n");
    close(pipefd[1]);
    pipefd[1] = -1;
  }

  struct pollfd accept_fds[2];
  struct pollfd data_fds[1];
  while (1) {
    serverport->prepare_accept(&accept_fds[0]);
    accept_fds[1].fd = pipefd[0];
    accept_fds[1].events = POLLIN;
    accept_fds[1].revents = 0;
    if (poll(accept_fds, 2, -1) < 0) {
      break;
    };
    if (true == serverport->is_request_connect()) {
      serverport->accept();
    }
    if (0 != accept_fds[1].revents) {
      break;
    }
    serverport->prepare_wait_data(data_fds);
    if (poll(data_fds, 1, -1) < 0) {
      break;
    };
    if (true == serverport->is_recv_data()) {
      serverport->read(logfilename, sizeof(logfilename));
      serverport->read(outfilename, sizeof(outfilename));
      serverport->read(&command, sizeof(command));
      serverport->read(&transaction, sizeof(transaction));
      serverport->read(&device_property_code, sizeof(device_property_code));
      serverport->read(&handle, sizeof(handle));
    } else {
      continue;
    }
    logfilename[SOCKET_NAME_MAX_LEN - 1] = '\0';
    outfilename[SOCKET_NAME_MAX_LEN - 1] = '\0';

    SocketClient *out = new SocketClient(outfilename);
    if (false == out->connect()) {
      fprintf(stderr, "cannot connect client outfile: %s(%d)\n",
              strerror(errno), errno);
      exit(0);
    }

    SocketClient *log = new SocketClient(logfilename);
    if (false == log->connect()) {
      fprintf(stderr, "cannot connect client logfile: %s(%d)\n",
              strerror(errno), errno);
      exit(0);
    }

    com::sony::imaging::remote::Command *c =
        new com::sony::imaging::remote::Command(log->getCommFD(),
                                                out->getCommFD());

    switch (command) {
      case SEND:
        c->send(ptp, &transaction);
        break;
      case RECV:
        if (transaction.size > 64 * 1024) {
          fprintf(stderr, "Incorrect transaction.size value");
          break;
        }
        c->recv(ptp, &transaction);
        break;
      case WAIT:
        c->wait(ptp);
        break;
      case RESET:
        c->reset(ptp);
        break;
      case CLEARHALT:
        c->clear_halt(ptp);
        break;
      case OPEN:
        c->open(ptp);
        break;
      case CLOSE:
        c->close(ptp);
        break;
      case AUTH:
        c->auth(ptp);
        break;
      case GET:
        c->get(ptp, device_property_code);
        break;
      case GETALL:
        c->getall(ptp);
        break;
      case GETOBJECT:
        c->getobject(ptp, handle);
        break;
      case GETLIVEVIEW:
        c->getliveview(ptp);
        break;
    }

    delete c;
    delete log;
    delete out;
    serverport->disconnect();

    if (CLOSE == command || RESET == command) {
      fprintf(stderr,
              "Please power off the camera or disconnect USB cable before next "
              "operations.\n");
      break;
    }
  }

  if (NULL != ptp) {
    ptp->disconnect();
    delete ptp;
  }

  close(pipefd[0]);
  if (-1 != pipefd[1]) {
    close(pipefd[1]);
  }

  fprintf(stderr, "server finished\n");
}

int com::sony::imaging::remote::offline(char *infile, char *outfile,
                                        int command,
                                        uint16_t device_property_code) {
  int ret = 0;

  // open input file
  int infd = STDIN_FILENO;
  if (0 == strncmp("-", infile, FILENAME_MAX_LEN)) {
    infd = STDIN_FILENO;
  } else {
    infd = open(infile, O_RDONLY);
  }

  // open output file
  int outfd = open_output_file(outfile, O_TRUNC);

  void *buf = NULL;
  SDIDevicePropInfoDatasetArray *info = NULL;
  SDIDevicePropInfoDataset *data = NULL;
  SimpleArray<uint32_t> *array = NULL;
  const int size_step = 1024;
  int read_size;
  char *_buf;

  if (-1 == infd) {
    fprintf(stderr, "cannot open %s. the reason is %s\n", infile,
            strerror(errno));
    ret = -1;
    goto err;
  }
  struct stat stat;
  if (0 != fstat(infd, &stat)) {
    fprintf(stderr, "cannot stat %s. the reason is %s\n", infile,
            strerror(errno));
    ret = -1;
    goto err;
  }

  _buf = (char *)buf;
  do {
    void *new_buf;
    new_buf = realloc(buf, _buf - (char *)buf + size_step);
    _buf = (char *)new_buf + (_buf - (char *)buf);
    if (NULL == new_buf) {
      fprintf(stderr, "input data is too big. the size is over %ld\n",
              _buf - (char *)buf);
      ret = -1;
      goto err;
    }
    buf = new_buf;
    read_size = read(infd, _buf, size_step);
    _buf += read_size;
  } while (read_size > 0);

  switch (command) {
    case GET:
      info = new SDIDevicePropInfoDatasetArray(buf);
      data = info->get(device_property_code);
      if (NULL == data) {
        fprintf(stderr, "device_property_code: 0x%04X is not exist\n",
                device_property_code);
      } else {
        data->toString();
      }
      break;
    case GETALL:
      info = new SDIDevicePropInfoDatasetArray(buf);
      info->toString();
      break;
  }

err:
  if (STDIN_FILENO != infd && infd >= 0) {
    close(infd);
  }
  close_output_file(outfd);
  free(buf);
  delete info;
  delete array;

  return ret;
}
