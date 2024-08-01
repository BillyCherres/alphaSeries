#include "command.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "parser.h"
#include "socc_ptp.h"
#include "socc_types.h"

// for stat
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// for open
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// for malloc
#include <stdlib.h>

// for errno
#include <errno.h>

using namespace com::sony::imaging::remote;

static struct timeval _time;
static struct tm *time_st;

inline void Command::log(const char *format, ...) {
  gettimeofday(&_time, NULL);
  time_st = localtime(&_time.tv_sec);
  fprintf(logout, "%02d:%02d:%02d.%06ld ", time_st->tm_hour, time_st->tm_min,
          time_st->tm_sec, _time.tv_usec);
  va_list ap;
  va_start(ap, format);
  vfprintf(logout, format, ap);
  va_end(ap);
  fflush(logout);
}

static void write(void *data, size_t size, FILE *target) {
  char *_data = (char *)data;
  size_t max = 4 * 1024;
  while (size > 0) {
    size_t write_size = max < size ? max : size;
    size_t ret = fwrite(_data, 1, write_size, target);
    size -= ret;
    _data += ret;
  }
}

Command::Command(char *log, char *out) {
  logout = stdout;
  setLogfile(log);
  outfile = stdout;
  setOutfile(out);
}

Command::Command(int logfd, int outfd) {
  logout = fdopen(logfd, "r+");
  outfile = fdopen(outfd, "r+");
}

Command::~Command() {
  if (stdout != outfile) {
    fclose(outfile);
  }
  if (stdout != logout) {
    fclose(logout);
  }
}

void Command::setLogfile(char *filename) {
  if (stdout != logout) {
    fclose(logout);
  }
  if (0 == strncmp("-", filename, FILENAME_MAX_LEN)) {
    logout = stdout;
  } else if (0 != strnlen(filename, FILENAME_MAX_LEN)) {
    logout = fopen(filename, "a");
  }
}

void Command::setOutfile(char *filename) {
  if (0 == strncmp("-", filename, FILENAME_MAX_LEN)) {
    outfile = stdout;
  } else if (0 != strnlen(filename, FILENAME_MAX_LEN)) {
    outfile = fopen(filename, "w");
  }
}

#define htole16a(a, x) (a)[1] = (uint8_t)((x) >> 8), (a)[0] = (uint8_t)(x)
int Command::_send(com::sony::imaging::remote::socc_ptp *ptp,
                   PTPTransaction *t) {
  com::sony::imaging::remote::Container res;
  int ret;
  if (com::sony::imaging::remote::PTPTransaction::DATA_IS_STRING == t->size) {
    // string
    uint8_t string_length = strnlen(t->data.string, PTP_MAXSTRLEN);
    uint32_t send_size = sizeof(uint8_t) + (string_length + 1) * 2;
    char *send_data = new char[send_size];
    char *_data = send_data;

    // length
    *(uint8_t *)_data = string_length;
    _data += sizeof(uint8_t);
    // string
    for (int i = 0; i < string_length + 1; i++) {
      htole16a(&_data[i * 2], (uint16_t)t->data.string[i]);
    }

    log("send > code=0x%04X, n=%d, p1=0x%08X, p2=0x%08X, p3=0x%08X, p4=0x%08X, "
        "p5=0x%08X, data=\"%s\", size=%d\n",
        t->code, t->nparam, t->params[0], t->params[1], t->params[2],
        t->params[3], t->params[4], t->data.string, send_size);
    ret = ptp->send(t->code, t->params, t->nparam, res, send_data, send_size);
    log("send < ret=%d, session=%d, transaction=%d, code=0x%04X, n=%d, "
        "p1=0x%08X, p2=0x%08X, p3=0x%08X, p4=0x%08X, p5=0x%08X\n",
        ret, res.session_id, res.transaction_id, res.code, res.nparam,
        res.param1, res.param2, res.param3, res.param4, res.param5);
    delete[] send_data;
  } else if (com::sony::imaging::remote::PTPTransaction::DATA_IS_FILE ==
             t->size) {
    int err = 0;
    ret = SOCC_ERROR_INVALID_PARAMETER;
    struct stat file_stat;
    do {
      int fd;
      fd = ::open(t->data.file, O_RDONLY);
      if (-1 == fd) {
        err = 2;
        break;
      }
      if (0 != fstat(fd, &file_stat)) {
        err = 1;
        ::close(fd);
        break;
      }
      char *buf = (char *)malloc(file_stat.st_size);
      if (NULL == buf) {
        err = 3;
      }
      if (0 == err) {
        ssize_t size = file_stat.st_size;
        ssize_t read_size;
        size_t offset = 0;
        while (size > 0) {
          read_size = read(fd, buf + offset, 1024 * 128);
          if (-1 == read_size) {
            err = 4;
            break;
          }
          size -= read_size;
          offset += read_size;
        }
      }
      ::close(fd);

      if (0 == err) {
        log("send > code=0x%04X, n=%d, p1=0x%08X, p2=0x%08X, p3=0x%08X, "
            "p4=0x%08X, p5=0x%08X, file=\"%s\", size=%ld\n",
            t->code, t->nparam, t->params[0], t->params[1], t->params[2],
            t->params[3], t->params[4], t->data.file, file_stat.st_size);
        ret = ptp->send(t->code, t->params, t->nparam, res, buf,
                        file_stat.st_size);
        log("send < ret=%d, session=%d, transaction=%d, code=0x%04X, n=%d, "
            "p1=0x%08X, p2=0x%08X, p3=0x%08X, p4=0x%08X, p5=0x%08X\n",
            ret, res.session_id, res.transaction_id, res.code, res.nparam,
            res.param1, res.param2, res.param3, res.param4, res.param5);
      }
      free(buf);
    } while (0);

    switch (err) {
      case 1:
        log("don't send anything. cannot get the status of %s\n", t->data.file);
        break;
      case 2:
        log("don't send anything. cannot open the file(%s). the reason is %s\n",
            t->data.file, strerror(errno));
        break;
      case 3:
        log("don't send anything. cannot malloc the memory. the size is %ld\n",
            file_stat.st_size);
        break;
      case 4:
        log("don't send anything. cannot read the file(%s). the reason is %s\n",
            t->data.file, strerror(errno));
        break;
    }
  } else {
    // integer
    log("send > code=0x%04X, n=%d, p1=0x%08X, p2=0x%08X, p3=0x%08X, p4=0x%08X, "
        "p5=0x%08X, data=0x%0*X, size=%d\n",
        t->code, t->nparam, t->params[0], t->params[1], t->params[2],
        t->params[3], t->params[4], t->size * 2, t->data.send, t->size);

    ret = ptp->send(t->code, t->params, t->nparam, res, &t->data.send, t->size);
    log("send < ret=%d, session=%d, transaction=%d, code=0x%04X, n=%d, "
        "p1=0x%08X, p2=0x%08X, p3=0x%08X, p4=0x%08X, p5=0x%08X\n",
        ret, res.session_id, res.transaction_id, res.code, res.nparam,
        res.param1, res.param2, res.param3, res.param4, res.param5);
  }

  return ret;
}

static char byte2char[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

int Command::_recv(com::sony::imaging::remote::socc_ptp *ptp,
                   PTPTransaction *t) {
  com::sony::imaging::remote::Container res;
  int ret;
  log("recv > code=0x%04X, n=%d, p1=0x%08X, p2=0x%08X, p3=0x%08X, p4=0x%08X, "
      "p5=0x%08X\n",
      t->code, t->nparam, t->params[0], t->params[1], t->params[2],
      t->params[3], t->params[4]);
  ret =
      ptp->receive(t->code, t->params, t->nparam, res, &t->data.recv, t->size);

  char buf[16];

  char *data = (char *)t->data.recv;

  if (t->size > 0) {
    buf[0] = '0';
    buf[1] = 'x';
    char *_buf = buf + 2;
    int i = 0;
    while (_buf - buf < 16 - 2 && (_buf - buf - 2) / 2 < t->size) {
      *_buf++ = byte2char[(data[i] >> 4) & 0xF];
      *_buf++ = byte2char[data[i] & 0xF];
      i++;
    }
    *_buf = 0;

  } else {
    buf[0] = 'N';
    buf[1] = 'A';
    buf[2] = 0;
  }
  log("recv < ret=%d, session=%d, transaction=%d, code=0x%04X, n=%d, "
      "p1=0x%08X, p2=0x%08X, p3=0x%08X, p4=0x%08X, p5=0x%08X, size=%u, "
      "data=%s\n",
      ret, res.session_id, res.transaction_id, res.code, res.nparam, res.param1,
      res.param2, res.param3, res.param4, res.param5, t->size, buf);

  return ret;
}

int Command::send(com::sony::imaging::remote::socc_ptp *ptp,
                  PTPTransaction *t) {
  int ret;
  ret = _send(ptp, t);
  return ret;
}

int Command::recv(com::sony::imaging::remote::socc_ptp *ptp,
                  PTPTransaction *t) {
  int ret;
  ret = _recv(ptp, t);
  write(t->data.recv, t->size, outfile);
  ptp->dispose_data(&t->data.recv);
  return ret;
}

int Command::wait(com::sony::imaging::remote::socc_ptp *ptp) {
  int ret;
  com::sony::imaging::remote::Container res;
  log("wait >\n");
  ret = ptp->wait_event(res);
  log("wait < ret=%d, session=%d, transaction=%d, code=0x%04X, n=%d, "
      "p1=0x%08X, p2=0x%08X, p3=0x%08X, p4=0x%08X, p5=0x%08X\n",
      ret, res.session_id, res.transaction_id, res.code, res.nparam, res.param1,
      res.param2, res.param3, res.param4, res.param5);

  return ret;
}

int Command::reset(com::sony::imaging::remote::socc_ptp *ptp) {
  int ret;
  log("reset >\n");
  ret = ptp->reset();
  log("reset < ret=%d\n", ret);
  return ret;
}

int Command::clear_halt(com::sony::imaging::remote::socc_ptp *ptp) {
  int ret;
  log("clear_halt >\n");
  ret = ptp->clear_halt();
  log("clear_halt < ret=%d\n", ret);
  return ret;
}

int Command::open(com::sony::imaging::remote::socc_ptp *ptp) {
  PTPTransaction transaction = {
      0x1002,           // .code
      {1, 0, 0, 0, 0},  // .params
      1,                // .nparam
      {0},              // .data
      0,                // .size
  };

  return _send(ptp, &transaction);
}

int Command::close(com::sony::imaging::remote::socc_ptp *ptp) {
  PTPTransaction transaction = {
      0x1003,           // .code
      {0, 0, 0, 0, 0},  // .params
      0,                // .nparam
      {0},              // .data
      0,                // .size
  };

  return _send(ptp, &transaction);
}

int Command::auth(com::sony::imaging::remote::socc_ptp *ptp) {
  int ret;
  uint16_t version = 0;

  PTPTransaction transaction = {
      0x9201,  // .code
      {        // .params
       1, 0x00000000, 0x00000000, 0, 0},
      3,    // .nparam
      {0},  // .data
      0,    // .size
  };
  ret = _recv(ptp, &transaction);
  ptp->dispose_data(&transaction.data.recv);
  if (SOCC_OK != ret) goto bail;

  transaction = PTPTransaction{
      0x9201,  // .code
      {        // .params
       2, 0x00000000, 0x00000000, 0, 0},
      3,    // .nparam
      {0},  // .data
      0,    // .size
  };
  ret = _recv(ptp, &transaction);
  ptp->dispose_data(&transaction.data.recv);
  if (SOCC_OK != ret) goto bail;

  do {
    transaction = PTPTransaction{
        0x9202,                // .code
        {                      // .params
         0x012C, 0, 0, 0, 0},  // 0x00C8/0x012C
        1,                     // .nparam
        {0},                   // .data
        0,                     // .size
    };
    ret = _recv(ptp, &transaction);
    if (0 != transaction.size) {
      version = *(uint16_t *)transaction.data.recv;
    }
    ptp->dispose_data(&transaction.data.recv);
    if (SOCC_OK != ret) goto bail;

  } while (0x012C != version);  // 0x00C8/0x012C

  transaction = PTPTransaction{
      0x9201,  // .code
      {        // .params
       3, 0x00000000, 0x00000000, 0, 0},
      3,    // .nparam
      {0},  // .data
      0,    // .size
  };
  ret = _recv(ptp, &transaction);
  ptp->dispose_data(&transaction.data.recv);
  if (SOCC_OK != ret) goto bail;

bail:
  return ret;
}

int Command::getall(com::sony::imaging::remote::socc_ptp *ptp) {
  int ret;
  std::string str;
  SDIDevicePropInfoDatasetArray *info;
  PTPTransaction transaction = {
      0x9209,           // .code
      {0, 1, 0, 0, 0},  // .params
      2,                // .nparam
      {0},              // .data
      0,                // .size
  };
  ret = _recv(ptp, &transaction);
  if (SOCC_OK != ret) goto bail;
  info = new SDIDevicePropInfoDatasetArray(transaction.data.recv);
  info->toString(str);
  fprintf(outfile, "%s", str.c_str());
  delete info;

bail:
  ptp->dispose_data(&transaction.data.recv);
  return ret;
}

int Command::get(com::sony::imaging::remote::socc_ptp *ptp,
                 uint16_t device_property_code) {
  int ret;
  SDIDevicePropInfoDatasetArray *info;
  SDIDevicePropInfoDataset *data;
  PTPTransaction transaction = {
      0x9209,           // .code
      {0, 1, 0, 0, 0},  // .params
      2,                // .nparam
      {0},              // .data
      0,                // .size
  };
  ret = _recv(ptp, &transaction);
  if (SOCC_OK != ret) goto bail;
  info = new SDIDevicePropInfoDatasetArray(transaction.data.recv);
  data = info->get(device_property_code);
  if (NULL != data) {
    std::string str;
    data->toString(str);
    fprintf(outfile, "%s", str.c_str());
  } else {
    log("cannot find the data of 0x%04X\n", device_property_code);
  }
  delete info;

bail:
  ptp->dispose_data(&transaction.data.recv);
  return ret;
}

int Command::getobject(com::sony::imaging::remote::socc_ptp *ptp,
                       uint32_t handle) {
  int ret;
  PTPTransaction transaction = {
      0x1009,                // .code
      {handle, 0, 0, 0, 0},  // .params
      1,                     // .nparam
      {0},                   // .data
      0,                     // .size
  };
  ret = _recv(ptp, &transaction);
  if (SOCC_OK != ret) goto bail;
  write(transaction.data.recv, transaction.size, outfile);

bail:
  ptp->dispose_data(&transaction.data.recv);
  return ret;
}

int Command::getliveview(com::sony::imaging::remote::socc_ptp *ptp) {
  int ret;
  LiveViewImage *live;
  PTPTransaction transaction = {
      0x1009,                    // .code
      {0xFFFFC002, 0, 0, 0, 0},  // .params
      1,                         // .nparam
      {0},                       // .data
      0,                         // .size
  };
  ret = _recv(ptp, &transaction);
  if (SOCC_OK != ret) goto bail;
  live = new LiveViewImage(transaction.data.recv);
  write(live->get(), live->size(), outfile);
  delete live;

bail:
  ptp->dispose_data(&transaction.data.recv);
  return ret;
}
