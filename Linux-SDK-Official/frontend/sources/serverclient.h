/**
 * @file serverclient.h
 * @brief Header file of the Server and Client.
 */

#ifndef __SERVER_CLIENT__H__
#define __SERVER_CLIENT__H__

#include "command.h"

#define SEND 1
#define RECV 2
#define WAIT 3
#define RESET 4
#define CLEARHALT 5
#define OPEN 10
#define CLOSE 11
#define AUTH 12
#define GET 13
#define GETALL 14
#define GETOBJECT 15
#define GETLIVEVIEW 16

namespace com {
namespace sony {
namespace imaging {
namespace remote {

class SocketClient;
class SocketServer;

com::sony::imaging::remote::SocketClient *server_create(int busn, int devn);
void server(int busn, int devn,
            com::sony::imaging::remote::SocketServer *serverport);
int client(com::sony::imaging::remote::SocketClient *serverport, char *logfile,
           char *outfile, int command,
           com::sony::imaging::remote::PTPTransaction *transaction,
           uint16_t device_property_code, uint32_t handle);
int offline(char *infile, char *outfile, int command,
            uint16_t device_property_code);

}  // namespace remote
}  // namespace imaging
}  // namespace sony
}  // namespace com

#endif  // __SERVER_CLIENT__H__
