/**
 * @page frontend The document of the Frontend part
 * @section contents Table of contents
 * - @ref outline
 * - @ref structure
 * - @ref command_reference
 * - @ref log_sample
 * @section outline Outline
 * This program executes a PTP command in linux shell and outputs the logs and a
object(a contents) to a file.
 * It is easy to confirm a camera behavior and to compare the behavior with your
code
 * because one by one PTP command can be executed from linux shell.\n
 * The logs sample is @link log_sample the bellow @endlink.\n
 *
 * @section structure Structure
 * The following figure is the software structure.
 * Analysis Command Line Option: @link main.cpp main.cpp @endlink \n
 * Server/Client: Server and Client model is employed for keeping USB
connection. See @link serverclient.h serverclient.h @endlink \n
 * Primitive and Utility command: @link command.h command.h @endlink \n
 * Dataset Parser: @link parser.h parser.h @endlink \n
 * @image html structure.png
 *
 * @section command_reference Command Reference
 * @note This program needs to be executed in 'root' user for the permission of
the usb device node.
 * And the environment variable LD_LIBRARY_PATH to out/lib/ directory needs to
be set.
 * @par Primitive Command
 * - control send \-\-op=OperationCode [\-\-p1=param1] [\-\-p2=param2]
[\-\-p3=param3] [\-\-p4=param4] [\-\-p5=param5] [\-\-size=size] [\-\-data=data]
[\-\-log=logfile] [\-\-bus=busn] [\-\-dev=devn]\n
 *   This command is used at the transaction which is like sending the command
(\em OperationCode and <em>param1-param5</em> ),
 *   sending the \em data as a number and the size is \em size and receiving a
response.\n
 *   The communication log are written in \em logfile as append mode. The log
includes the time, the sended command and the response.
 *   If \-\-log=\- is set or this option is omitted, the log outputs to
stdout.\n
 *   This command is sent to the bus number \em busn and the device number \em
devn. These numbers can be gotten by lsusb linux command.
 *   If these numbers are omitted, a device is found  automatically.\n
 *   if \-\-size=string is set, sending the \em data as a string.\n
 *   if \-\-size=file is set and the \em data is set in a filename, the file of
the filename is sent.\n
 *   if \em data and \em size are omitted, the transaction changes to sending
the command and receiving a response.
 * - control recv \-\-op=OperationCode [\-\-p1=param1] [\-\-p2=param2]
[\-\-p3=param3] [\-\-p4=param4] [\-\-p5=param5] [\-\-of=outfile]
[\-\-log=logfile] [\-\-bus=busn] [\-\-dev=devn]\n
 *   This command is used at the transaction which is like sending the command
(\em OperationCode and <em>param1-param5</em> ),
 *   receiving a data and receiving a response. The data outputs to \em
outfile.\n
 *   If \-\-of=\- is set or this option is omitted, the data outputs to stdout.
 * - control wait [\-\-log=logfile] [\-\-bus=busn] [\-\-dev=devn]\n
 *   waiting a event. The event code and the parameters are written in \em
logfile.
 * - control clear [\-\-bus=busn] [\-\-dev=devn]\n
 *   When Bulk-in and Bulk-out endpoints of the USB are stalled, recover from it
by calling this.\n
 *   STALL occurs at sending unexecutable commmand (wrong operation code or
parameters).\n
 *   If this command is success, a send, recv and etc operation can be executed.
 * - control reset [\-\-bus=busn] [\-\-dev=devn]\n
 *   When unrecoverable error occurs, for example, "clear" command returns
error, the device may recover by calling this.\n
 *   After this command, USB connection is down. So "open" command is needed to
be called.
 *
 * @par Utility Command
 * - control open [\-\-log=logfile] [\-\-bus=busn] [\-\-dev=devn]\n
 *   execute "open session"
 * - control close [\-\-log=logfile] [\-\-bus=busn] [\-\-dev=devn]\n
 *   execute "close session"
 * - control auth [\-\-log=logfile] [\-\-bus=busn] [\-\-dev=devn]\n
 *   execute "Authentication" sequence.
 * - control getall [\-\-if=infile] [\-\-of=outfile] [\-\-log=logfile]
[\-\-bus=busn] [\-\-dev=devn]\n
 *   execute SONY_GETALLEXTDEVICEPROPINFO and then parse it and all data output
to \em outfile.\n
 *   If \em infile which is the outfile by getting "recv" command is set, the
parsing it is only executed.\n
 *   If \-\-if=\- is set, read a data from stdin.
 * - control get DevicePropertyCode [\-\-if=infile] [\-\-of=outfile]
[\-\-log=logfile] [\-\-bus=busn] [\-\-dev=devn]\n
 *   execute SONY_GETALLEXTDEVICEPROPINFO and then parse it and the dataset of
\em DevicePropertyCode is output to \em outfile.\n
 * - control getobject handle [\-\-of=outfile] [\-\-log=logfile] [\-\-bus=busn]
[\-\-dev=devn]\n
 *   execute GetObject command for \em handle. The object data output to \em
outfile.
 * - control getliveview [\-\-of=outfile] [\-\-log=logfile] [\-\-bus=busn]
[\-\-dev=devn]\n
 *   get the LiveView image and output to \em outfile.
 *
 * @section log_sample Log Sample
 * The following log is the log when
'frontend/scripts/shoot_an_image_and_get_it.sh' is executed.
 * \code
open session
command: open
12:50:34.082687 send > code=0x1002, n=1, p1=0x00000001, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, data=0x0, size=0 12:50:34.101946
send < ret=0, session=0, transaction=0, code=0x2001, n=0, p1=0x00000000,
p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000 authentication
command: auth
12:50:34.106939 recv > code=0x96FE, n=3, p1=0x00000001, p2=0x0000DA01,
p3=0x0000DA01, p4=0x00000000, p5=0x00000000 12:50:34.115281 recv < ret=0,
session=0, transaction=1, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=8, data=0x000000000000
12:50:34.115319 recv > code=0x96FE, n=3, p1=0x00000002, p2=0x0000DA01,
p3=0x0000DA01, p4=0x00000000, p5=0x00000000 12:50:34.138502 recv < ret=0,
session=0, transaction=2, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=8, data=0x000000000000
12:50:34.138535 recv > code=0x96FD, n=1, p1=0x000000C8, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 12:50:34.144529 recv < ret=0,
session=0, transaction=3, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=0, data=NA 12:50:34.144553
recv > code=0x96FD, n=1, p1=0x000000C8, p2=0x00000000, p3=0x00000000,
p4=0x00000000, p5=0x00000000 12:50:34.150554 recv < ret=0, session=0,
transaction=4, code=0x2001, n=0, p1=0x00000000, p2=0x00000000, p3=0x00000000,
p4=0x00000000, p5=0x00000000, size=0, data=NA 12:50:34.150574 recv >
code=0x96FD, n=1, p1=0x000000C8, p2=0x00000000, p3=0x00000000, p4=0x00000000,
p5=0x00000000 12:50:34.898787 recv < ret=0, session=0, transaction=5,
code=0x2001, n=0, p1=0x00000000, p2=0x00000000, p3=0x00000000, p4=0x00000000,
p5=0x00000000, size=112, data=0x080014000000 12:50:34.898837 recv > code=0x96FE,
n=3, p1=0x00000003, p2=0x0000DA01, p3=0x0000DA01, p4=0x00000000, p5=0x00000000
12:50:34.904601 recv < ret=0, session=0, transaction=6, code=0x2001, n=0,
p1=0x00000000, p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000,
size=8, data=0x000000000000 waiting set time API command: get outfile: fifo17858
12:50:34.910046 recv > code=0x96F6, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 12:50:34.929809 recv < ret=0,
session=0, transaction=7, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=1277, data=0x120000000000
out=" dataset DevicePropertyCode: D6B1 dataset DataType: FFFF dataset GetSet: 01
dataset IsEnable: 01 dataset FormFlag: 00 dataset DefaultValue:
"20150101T000000+0900" dataset CurrentValue: "20150101T000000+0900"" set time
command: send
op: 0x96FA
p1: 54961
data: 20150801
op: 0x96FA
p1: 54961
data: 20150801T150000+0900
12:50:34.942791 send > code=0x96FA, n=1, p1=0x0000D6B1, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, data="20150801T150000+0900",
size=43 12:50:34.959577 send < ret=0, session=0, transaction=8, code=0x2001,
n=0, p1=0x00000000, p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000
waiting live view
command: get
outfile: fifo17858
12:50:34.963677 recv > code=0x96F6, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 12:50:34.995521 recv < ret=0,
session=0, transaction=9, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=1277, data=0x120000000000
out=" dataset DevicePropertyCode: D6DE dataset DataType: 0002 dataset GetSet: 01
dataset IsEnable: 01 dataset FormFlag: 02 dataset DefaultValue: 00 dataset
CurrentValue: 00 dataset Enumeration-Form dataset NumOfValues: 3 dataset Value:
00 dataset Value: 01 dataset Value: 02"

...snip...

command: get
outfile: fifo17858
12:50:36.526663 recv > code=0x96F6, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 12:50:36.555119 recv < ret=0,
session=0, transaction=91, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=1277, data=0x120000000000
out=" dataset DevicePropertyCode: D6DE dataset DataType: 0002 dataset GetSet: 01
dataset IsEnable: 01 dataset FormFlag: 02 dataset DefaultValue: 00 dataset
CurrentValue: 00 dataset Enumeration-Form dataset NumOfValues: 3 dataset Value:
00 dataset Value: 01 dataset Value: 02" command: get outfile: fifo17858
12:50:36.566368 recv > code=0x96F6, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 12:50:36.573842 recv < ret=0,
session=0, transaction=92, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=1277, data=0x120000000000
out=" dataset DevicePropertyCode: D6DE dataset DataType: 0002 dataset GetSet: 01
dataset IsEnable: 01 dataset FormFlag: 02 dataset DefaultValue: 00 dataset
CurrentValue: 01 dataset Enumeration-Form dataset NumOfValues: 3 dataset Value:
00 dataset Value: 01 dataset Value: 02" shooting command: send op: 0x96F8 p1:
54813 data: 2 size: 2 12:50:36.586895 send > code=0x96F8, n=1, p1=0x0000D61D,
p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000, data=0x0002, size=2
12:50:36.609893 send < ret=0, session=0, transaction=93, code=0x2001, n=0,
p1=0x00000000, p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000
command: send
op: 0x96F8
p1: 54807
data: 2
size: 2
12:50:36.614666 send > code=0x96F8, n=1, p1=0x0000D617, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, data=0x0002, size=2 12:50:36.620887
send < ret=0, session=0, transaction=94, code=0x2001, n=0, p1=0x00000000,
p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000 command: send op:
0x96F8 p1: 54807 data: 1 size: 2 12:50:36.625761 send > code=0x96F8, n=1,
p1=0x0000D617, p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000,
data=0x0001, size=2 12:50:36.651327 send < ret=0, session=0, transaction=95,
code=0x2001, n=0, p1=0x00000000, p2=0x00000000, p3=0x00000000, p4=0x00000000,
p5=0x00000000 command: send op: 0x96F8 p1: 54813 data: 1 size: 2 12:50:36.656070
send > code=0x96F8, n=1, p1=0x0000D61D, p2=0x00000000, p3=0x00000000,
p4=0x00000000, p5=0x00000000, data=0x0001, size=2 12:50:36.666193 send < ret=0,
session=0, transaction=96, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 waiting the event of adding a image
command: wait
logfile: fifo17858
12:50:36.671570 wait > 12:50:36.675579 wait < ret=0, session=0, transaction=-1,
code=0xC203, n=1, p1=0x0000D6CD, p2=0x00000000, p3=0x00000000, p4=0x00000000,
p5=0x00000000

...snip...

command: wait
logfile: fifo17858
12:50:37.792812 wait > 12:50:38.235595 wait < ret=0, session=0, transaction=-1,
code=0xC203, n=1, p1=0x0000D6CD, p2=0x00000000, p3=0x00000000, p4=0x00000000,
p5=0x00000000 command: wait logfile: fifo17858 12:50:38.243241 wait >
12:50:38.319697 wait < ret=0, session=0, transaction=-1, code=0xC201, n=1,
p1=0xFFFFC001, p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000 get
captured_info command: get 12:50:38.334088 recv > code=0x96F6, n=0,
p1=0x00000000, p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000
12:50:38.340356 recv < ret=0, session=0, transaction=97, code=0x2001, n=0,
p1=0x00000000, p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000,
size=1277, data=0x120000000000 dataset DevicePropertyCode: D6C6 dataset
DataType: 0004 dataset GetSet: 00 dataset IsEnable: 01 dataset FormFlag: 01
  dataset DefaultValue: 0000
  dataset CurrentValue: 8001
  dataset Range-Form
    dataset MinimumValue: 0000
    dataset MaximumValue: FFFF
    dataset StepSize: 0001
getobjectinfo
command: recv
op: 0x1008
p1: 4294950913
12:50:38.343438 recv > code=0x1008, n=1, p1=0xFFFFC001, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 12:50:38.357009 recv < ret=0,
session=0, transaction=98, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=82, data=0x000001000118
getobject
command: getobject
outfile: shoot.jpg
12:50:38.359880 recv > code=0x1009, n=1, p1=0xFFFFC001, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 12:50:38.622962 recv < ret=0,
session=0, transaction=99, code=0x2001, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000, size=5630413, data=0x0F080F010D0E
get captured_info
command: get
12:50:38.653581 recv > code=0x96F6, n=0, p1=0x00000000, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000 dataset DevicePropertyCode: D6C6
  dataset DataType: 0004
  dataset GetSet: 00
  dataset IsEnable: 01
  dataset FormFlag: 01
  dataset DefaultValue: 0000
  dataset CurrentValue: 0000
  dataset Range-Form
    dataset MinimumValue: 0000
    dataset MaximumValue: FFFF
    dataset StepSize: 0001
12:50:38.658133 recv < ret=0, session=0, transaction=100, code=0x2001, n=0,
p1=0x00000000, p2=0x00000000, p3=0x00000000, p4=0x00000000, p5=0x00000000,
size=1277, data=0x120000000000 waiting the event of removing a image command:
wait logfile: fifo17858 12:50:38.663772 wait > 12:50:38.667648 wait < ret=0,
session=0, transaction=-1, code=0xC202, n=1, p1=0xFFFFC001, p2=0x00000000,
p3=0x00000000, p4=0x00000000, p5=0x00000000
 * \endcode
*/

/**
 * @file main.cpp
 * @brief analyze command line options.
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#include "command.h"
#include "serverclient.h"
#include "socket.hpp"

void usage() {
  fprintf(stderr, "USAGE: control command [OPTION]\n\n");
  fprintf(stderr,
          "Commands:\n"
          "  send, recv, wait, clear, reset, open, close, auth, getall, get, "
          "getobject, getliveview\n");
  fprintf(stderr,
          "Options:\n"
          "  --op=OPERATION-CODE          Operation code\n"
          "  --p1-p5=params               Operation parameters\n"
          "  --data                       The sended data\n"
          "  --size=size                  Data size\n"
          "  --log=logfile                Output a log to logfile\n"
          "  --of=outfile                 Output a output to outfile\n"
          "  --if=infile                  Input from infile\n"
          "  --bus=BUS-NUMBER             USB bus number\n"
          "  --dev=DEV-NUMBER             USB assigned device number\n"
          "\n");
}

#define OPTCMP(value, str, COMMAND) \
  if (!strcmp(str, argv[1])) {      \
    value = COMMAND;                \
  }

int main(int argc, char **argv) {
  int command = 0;
  int busn = 0, devn = 0;
  com::sony::imaging::remote::PTPTransaction transaction;
  uint16_t device_property_code = 0;
  uint32_t handle = 0;
  char infilename[FILENAME_MAX_LEN];
  infilename[0] = 0;
  char outfilename[FILENAME_MAX_LEN];
  outfilename[0] = 0;
  char logfilename[FILENAME_MAX_LEN];
  logfilename[0] = 0;
  /* parse options */
  int option_index = 0, opt;
  static struct option loptions[] = {
      {"bus", 1, 0, 'b'}, {"dev", 1, 0, 'd'},  {"p1", 1, 0, 0},
      {"p2", 1, 0, 0},    {"p3", 1, 0, 0},     {"p4", 1, 0, 0},
      {"p5", 1, 0, 0},    {"size", 1, 0, 's'}, {"data", 1, 0, 'D'},
      {"log", 1, 0, 'l'}, {"op", 1, 0, 'O'},   {"if", 1, 0, 'i'},
      {"of", 1, 0, 'o'},  {0, 0, 0, 0}};

  if (argc < 2) {
    usage();
    return 0;
  }
  memset(&transaction, 0, sizeof(transaction));
  fprintf(stderr, "command: %s\n", argv[1]);
  OPTCMP(command, "send", SEND);
  OPTCMP(command, "recv", RECV);
  OPTCMP(command, "wait", WAIT);
  OPTCMP(command, "reset", RESET);
  OPTCMP(command, "clear", CLEARHALT);
  OPTCMP(command, "open", OPEN);
  OPTCMP(command, "close", CLOSE);
  OPTCMP(command, "auth", AUTH);
  OPTCMP(command, "get", GET);
  if (GET == command) {
    bool err = 0;
    char *endptr = NULL;
    if (argc < 3) {
      err = 1;
    } else {
      device_property_code = strtoll(argv[2], &endptr, 0);
    }
    if (err || argv[2] == endptr) {
      fprintf(stderr, "command: \"get\" needs a device property code\n");
      return -1;
    }
  }
  OPTCMP(command, "getall", GETALL);
  OPTCMP(command, "getobject", GETOBJECT);
  if (GETOBJECT == command) {
    bool err = 0;
    char *endptr;
    if (argc < 3) {
      err = 1;
    } else {
      handle = strtoll(argv[2], &endptr, 0);
    }
    if (err || argv[2] == endptr) {
      fprintf(stderr, "command: \"getobject\" needs a handle\n");
      return -1;
    }
  }
  OPTCMP(command, "getliveview", GETLIVEVIEW);

  optind = 2;
  while (1) {
    opt = getopt_long(argc, argv, "b:d:s:d:l:O:i:o:", loptions, &option_index);
    if (-1 == opt) break;

    switch (opt) {
      case 0:
        if (!(strcmp("p1", loptions[option_index].name))) {
          uint32_t param = strtoll(optarg, NULL, 0);
          fprintf(stderr, "p1: %u\n", param);
          transaction.params[0] = param;
          if (1 > transaction.nparam) {
            transaction.nparam = 1;
          }
        }
        if (!(strcmp("p2", loptions[option_index].name))) {
          uint32_t param = strtoll(optarg, NULL, 0);
          fprintf(stderr, "p2: %u\n", param);
          transaction.params[1] = param;
          if (2 > transaction.nparam) {
            transaction.nparam = 2;
          }
        }
        if (!(strcmp("p3", loptions[option_index].name))) {
          uint32_t param = strtoll(optarg, NULL, 0);
          fprintf(stderr, "p3: %u\n", param);
          transaction.params[2] = param;
          if (3 > transaction.nparam) {
            transaction.nparam = 3;
          }
        }
        if (!(strcmp("p4", loptions[option_index].name))) {
          uint32_t param = strtoll(optarg, NULL, 0);
          fprintf(stderr, "p4: %u\n", param);
          transaction.params[3] = param;
          if (4 > transaction.nparam) {
            transaction.nparam = 4;
          }
        }
        if (!(strcmp("p5", loptions[option_index].name))) {
          uint32_t param = strtoll(optarg, NULL, 0);
          fprintf(stderr, "p5: %u\n", param);
          transaction.params[4] = param;
          if (5 > transaction.nparam) {
            transaction.nparam = 5;
          }
        }
        break;
      case 'b': {
        busn = strtoll(optarg, NULL, 0);
        fprintf(stderr, "bus: %d\n", busn);
        break;
      }
      case 'd': {
        devn = strtoll(optarg, NULL, 0);
        fprintf(stderr, "dev: %d\n", devn);
        break;
      }
      case 's': {
        if (0 == strcmp("string", optarg)) {
          if (com::sony::imaging::remote::PTPTransaction::DATA_IS_STRING !=
              transaction.size) {
            optind = 2;
          }
          transaction.size =
              com::sony::imaging::remote::PTPTransaction::DATA_IS_STRING;
        } else if (0 == strcmp("file", optarg)) {
          if (com::sony::imaging::remote::PTPTransaction::DATA_IS_FILE !=
              transaction.size) {
            optind = 2;
          }
          transaction.size =
              com::sony::imaging::remote::PTPTransaction::DATA_IS_FILE;
        } else {
          uint32_t size = strtoll(optarg, NULL, 0);
          fprintf(stderr, "size: %u\n", size);
          transaction.size = size;
        }
        break;
      }
      case 'D': {
        if (com::sony::imaging::remote::PTPTransaction::DATA_IS_STRING ==
            transaction.size) {
          strncpy(transaction.data.string, optarg, OPT_STRING_MAX_LEN);
          fprintf(stderr, "data: %s\n", transaction.data.string);
        } else if (com::sony::imaging::remote::PTPTransaction::DATA_IS_FILE ==
                   transaction.size) {
          char *full_path;
          full_path = realpath(optarg, NULL);
          if (NULL == full_path) {
            fprintf(stderr, "the file(%s) is not exist?\n", optarg);
            return 1;
          }
          if (FILENAME_MAX_LEN - 1 < strnlen(full_path, PATH_MAX)) {
            fprintf(stderr, "the file(%s) path is too long\n", full_path);
            free(full_path);
            return 1;
          }
          strncpy(transaction.data.file, full_path, FILENAME_MAX_LEN);
          fprintf(stderr, "filedata: %s\n", transaction.data.file);
          free(full_path);
        } else {
          uint32_t data = strtoll(optarg, NULL, 0);
          fprintf(stderr, "data: %u\n", data);
          transaction.data.send = data;
        }
        break;
      }
      case 'l': {
        fprintf(stderr, "logfile: %s\n", optarg);
        strncpy(logfilename, optarg, FILENAME_MAX_LEN);
        break;
      }
      case 'O': {
        uint16_t op = strtoll(optarg, NULL, 0);
        fprintf(stderr, "op: 0x%04X\n", op);
        transaction.code = op;
        break;
      }
      case 'i': {
        fprintf(stderr, "infile: %s\n", optarg);
        strncpy(infilename, optarg, FILENAME_MAX_LEN);
        break;
      }
      case 'o': {
        fprintf(stderr, "outfile: %s\n", optarg);
        strncpy(outfilename, optarg, FILENAME_MAX_LEN);
        break;
      }
      default:
        fprintf(stderr, "getopt returned character code 0%o\n", opt);
        break;
    }
  }

  // offline
  if (0 != strnlen(infilename, FILENAME_MAX_LEN)) {
    return com::sony::imaging::remote::offline(infilename, outfilename, command,
                                               device_property_code);
  }

  // online
  com::sony::imaging::remote::SocketClient *server_port =
      com::sony::imaging::remote::server_create(busn, devn);
  int ret = com::sony::imaging::remote::client(
      server_port, logfilename, outfilename, command, &transaction,
      device_property_code, handle);
  delete server_port;

  return ret;
}
