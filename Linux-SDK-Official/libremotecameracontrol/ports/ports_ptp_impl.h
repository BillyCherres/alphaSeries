#ifndef __PORTS_PTP_IMPL_H__
#define __PORTS_PTP_IMPL_H__

#include <socc_types.h>

#include "ports_ptp.h"

namespace com {
namespace sony {
namespace imaging {
namespace ports {

class ports_usb;

typedef struct _GenericBulkContainerHeader {
  uint32_t length;
  uint16_t type;
  uint16_t code;
  uint32_t transaction_id;
} GenericBulkContainerHeader;

class GenericBulkContainer {
 public:
  GenericBulkContainer(uint32_t length, uint16_t type, uint16_t code,
                       uint32_t transaction_id, uint32_t size) {}
  GenericBulkContainer(void* raw) {}
  uint32_t length;
  uint16_t type;
  uint16_t code;
  uint32_t transaction_id;
  uint32_t size;
  uint32_t param1;
  uint32_t param2;
  uint32_t param3;
  uint32_t param4;
  uint32_t param5;
  void* payload;
  int nparam;
};

class ports_ptp_impl : public ports_ptp {
 public:
  ports_ptp_impl(int busn, int devn, uint32_t session_id,
                 uint32_t transaction_id, ports_usb* usb);
  ~ports_ptp_impl();

  int send(uint16_t code, uint32_t* parameters, uint8_t num,
           com::sony::imaging::remote::Container& response, void* data,
           uint32_t size);
  int receive(uint16_t code, uint32_t* parameters, uint8_t num,
              com::sony::imaging::remote::Container& response, void** data,
              uint32_t& size);
  int wait_event(com::sony::imaging::remote::Container& container);
  void dispose_data(void** data);

 private:
  uint32_t session_id;
  uint32_t transaction_id;
  ports_usb* usb;

  int sendreq(uint16_t code, uint32_t* parameters, uint8_t num);
  int senddata(uint16_t code, uint32_t* parameters, uint8_t num, void* data,
               unsigned int size);
  int getdata(void** data, uint32_t& size);
  int getresp(com::sony::imaging::remote::Container& response);
  int getevent(com::sony::imaging::remote::Container& event);
};

}  // namespace ports
}  // namespace imaging
}  // namespace sony
}  // namespace com
#endif
