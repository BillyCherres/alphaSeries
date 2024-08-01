#ifndef __PORTS_USB_IMPL_H__
#define __PORTS_USB_IMPL_H__

#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include <socc_types.h>

#include "ports_usb.h"

namespace com {
namespace sony {
namespace imaging {
namespace ports {

class ports_usb_impl_target_device;

class ports_usb_impl : public ports_usb {
 public:
  ports_usb_impl(int busn, int devn);
  int open();
  int close();
  int write(void* bytes, unsigned int size);
  int read(void* bytes, unsigned int size);
  int read_interrupt(void* bytes, unsigned int size);
  int clear_halt(int what = -0);
  int reset();
  void set_hotplug_callback(socc_hotplug_callback_func_t callback_func,
                            void* vp);
  int snatch_device_handle(socc_device_handle_info_t& info);

 private:
  int busn;
  int devn;

  int inep;
  int outep;
  int intep;

  int configuration_value;
  int interface_number;
  int alternate_setting;

  usb_device_info_t current_device;
  socc_hotplug_callback_func_t user_callback_func;
  void* user_callback_data;

  libusb_context* context;
  libusb_device* device;
  libusb_device_handle* device_handle;
  libusb_hotplug_callback_handle hotplug_callback_handle;

  ports_usb_impl_target_device* target_device;

  pthread_t thread_id;
  pthread_attr_t thread_attr;

  int bulk_write(int ep, void* bytes, unsigned int size);
  int bulk_read(int ep, void* bytes, unsigned int size);

  static void* event_thread(void* vp);
  static int LIBUSB_CALL hotplug_callback_entry(libusb_context* ctx,
                                                libusb_device* device,
                                                libusb_hotplug_event event,
                                                void* user_data);
  static const int bulk_transfer_max_size;
};

class ports_usb_impl_target_device {
 public:
  ports_usb_impl_target_device(libusb_context* context);
  ~ports_usb_impl_target_device();

  libusb_device* find_device(int busn, int devn, libusb_class_code code);
  void get_settings(int& config, int& inter, int& alt);
  void get_endpoint(int& inep, int& outep, int& intep);

  usb_device_info_t current_device;

 private:
  struct libusb_config_descriptor* find_configurations(libusb_device& device,
                                                       int index = 0);
  const struct libusb_interface_descriptor* find_altsetting_by_class_code(
      libusb_config_descriptor& config_descriptor, libusb_class_code code);
  bool configure_endpoint(const struct libusb_interface_descriptor& altsetting,
                          int& inep, int& outep, int& intep);

  libusb_context* context;
  libusb_device* device;

  int configuration_value;
  int interface_number;
  int alternate_setting;
  int inep;
  int outep;
  int intep;
};

}  // namespace ports
}  // namespace imaging
}  // namespace sony
}  // namespace com
#endif
