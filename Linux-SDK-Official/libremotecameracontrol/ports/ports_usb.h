#ifndef __PORTS_USB_H__
#define __PORTS_USB_H__

#include <socc_types.h>

namespace com {
namespace sony {
namespace imaging {
namespace ports {

//#define BULK_MAX_PACKET_SIZE (512) for USB2.0
#define BULK_MAX_PACKET_SIZE (1024)  // USB3.0

typedef struct __usb_device_info_t {
  int idVendor;
  int idProduct;
  int bInterfaceClass;
  int iManufacturer;
  int iProduct;
  int iSerialNumber;
  int busn;
  int devn;
  int inep;
  int outep;
  int intep;
  unsigned char ascii_Manufacturer[16];
  unsigned char ascii_Product[16];
  unsigned char ascii_SerialNumber[16];
} usb_device_info_t;

class ports_usb {
 public:
  virtual ~ports_usb(){};
  virtual int open() = 0;
  virtual int close() = 0;
  virtual int write(void* bytes, unsigned int size) = 0;
  virtual int read(void* bytes, unsigned int size) = 0;
  virtual int read_interrupt(void* bytes, unsigned int size) = 0;
  virtual int clear_halt(int what = 0) = 0;
  virtual int reset() = 0;
  virtual void set_hotplug_callback(socc_hotplug_callback_func_t callback_func,
                                    void* vp) = 0;
  virtual int snatch_device_handle(socc_device_handle_info_t& info) = 0;

 protected:
};

}  // namespace ports
}  // namespace imaging
}  // namespace sony
}  // namespace com
#endif
