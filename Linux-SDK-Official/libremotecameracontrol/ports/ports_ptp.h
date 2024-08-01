#ifndef __PORTS_PTP_H__
#define __PORTS_PTP_H__

#include <stdint.h>

#include "parser.h"
#include "socc_ptp.h"

namespace com {
namespace sony {
namespace imaging {
namespace ports {

class ports_ptp {
 public:
  virtual ~ports_ptp(){};
  virtual int send(uint16_t code, uint32_t* parameters, uint8_t num,
                   com::sony::imaging::remote::Container& response, void* data,
                   uint32_t size) = 0;
  virtual int receive(uint16_t code, uint32_t* parameters, uint8_t num,
                      com::sony::imaging::remote::Container& response,
                      void** data, uint32_t& size) = 0;
  virtual int wait_event(com::sony::imaging::remote::Container& container) = 0;
  virtual void dispose_data(void** data) = 0;
};

}  // namespace ports
}  // namespace imaging
}  // namespace sony
}  // namespace com
#endif
