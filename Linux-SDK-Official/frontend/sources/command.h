/**
 * @file command.h
 * @brief Header file of the PTP command.
 */

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdint.h>

#include "socc_ptp.h"
#include "socc_types.h"  // need to be removed

#define FILENAME_MAX_LEN 256
#define OPT_STRING_MAX_LEN 255
namespace com {
namespace sony {
namespace imaging {
namespace remote {

typedef struct _PTPTransaction {
  static const uint32_t DATA_IS_STRING = 0xFFFFFFFF;
  static const uint32_t DATA_IS_FILE = 0xFFFFFFFE;
  uint16_t code;
  uint32_t params[5];
  uint8_t nparam;
  union {
    uint32_t send;
    void *recv;
    char string[OPT_STRING_MAX_LEN + 1];
    char file[FILENAME_MAX_LEN];
  } data;
  uint32_t size;
} PTPTransaction;

class Command {
 private:
  FILE *logout;
  FILE *outfile;

  inline void log(const char *format, ...);
  void setOutfile(char *filename);
  void setLogfile(char *filename);

  int _send(com::sony::imaging::remote::socc_ptp *ptp, PTPTransaction *t);
  int _recv(com::sony::imaging::remote::socc_ptp *ptp, PTPTransaction *t);

 public:
  Command(char *log, char *out);
  Command(int logfd, int outfd);
  ~Command();

  int send(com::sony::imaging::remote::socc_ptp *ptp, PTPTransaction *t);
  int recv(com::sony::imaging::remote::socc_ptp *ptp, PTPTransaction *t);
  int wait(com::sony::imaging::remote::socc_ptp *ptp);
  int reset(com::sony::imaging::remote::socc_ptp *ptp);
  int clear_halt(com::sony::imaging::remote::socc_ptp *ptp);
  int open(com::sony::imaging::remote::socc_ptp *ptp);
  int close(com::sony::imaging::remote::socc_ptp *ptp);
  int auth(com::sony::imaging::remote::socc_ptp *ptp);
  int getall(com::sony::imaging::remote::socc_ptp *ptp);
  int get(com::sony::imaging::remote::socc_ptp *ptp,
          uint16_t device_property_code);
  int getobject(com::sony::imaging::remote::socc_ptp *ptp, uint32_t handle);
  int getliveview(com::sony::imaging::remote::socc_ptp *ptp);
};

}  // namespace remote
}  // namespace imaging
}  // namespace sony
}  // namespace com
#endif  // __COMMAND_H__
