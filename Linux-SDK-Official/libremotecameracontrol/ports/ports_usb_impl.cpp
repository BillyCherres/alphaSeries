#include "ports_usb_impl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstddef>

using namespace com::sony::imaging::ports;

const int ports_usb_impl::bulk_transfer_max_size = 10 * 1024 * 1024;

ports_usb_impl::ports_usb_impl(int _busn, int _devn)
    : busn(_busn),
      devn(_devn),
      inep(-1),
      outep(-1),
      intep(-1),
      configuration_value(-1),
      interface_number(-1),
      alternate_setting(-1),
      user_callback_func(NULL),
      user_callback_data(NULL),
      device(NULL),
      device_handle(NULL),
      context(NULL),
      hotplug_callback_handle(0),
      thread_id(0),
      target_device(NULL) {
  memset(&current_device, 0, sizeof(current_device));
}
int ports_usb_impl::open() {
  int ret = SOCC_OK;
  int count;
  libusb_device** devs = NULL;

  ret = libusb_init(&context);
  if (ret != 0) {
    close();
    return SOCC_ERROR_USB_INIT;
  }
    
//  libusb_set_option(context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);
    libusb_set_debug(context, LIBUSB_LOG_LEVEL_ERROR);
    
  ret = pthread_attr_init(&thread_attr);
  if (ret != 0) {
    close();
    return SOCC_ERROR_THREAD_INIT;
  }

  ret = pthread_create(&thread_id, &thread_attr, &event_thread, this);
  if (ret != 0) {
    close();
    return SOCC_ERROR_THREAD_CREATE;
  }

  target_device = new ports_usb_impl_target_device(context);

  device = target_device->find_device(busn, devn, LIBUSB_CLASS_PTP);
  if (device == NULL) {
    close();
    return SOCC_ERROR_USB_DEVICE_NOT_FOUND;
  }

  target_device->get_settings(configuration_value, interface_number,
                              alternate_setting);
  target_device->get_endpoint(inep, outep, intep);

  current_device = target_device->current_device;

  ret = libusb_open(device, &device_handle);
  if (ret != 0) {
    close();
    return SOCC_ERROR_USB_OPEN;
  }

  if (ret == LIBUSB_SUCCESS) {
    libusb_get_string_descriptor_ascii(
        device_handle, current_device.iManufacturer,
        current_device.ascii_Manufacturer,
        sizeof(current_device.ascii_Manufacturer));
    libusb_get_string_descriptor_ascii(device_handle, current_device.iProduct,
                                       current_device.ascii_Product,
                                       sizeof(current_device.ascii_Product));
    libusb_get_string_descriptor_ascii(
        device_handle, current_device.iSerialNumber,
        current_device.ascii_SerialNumber,
        sizeof(current_device.ascii_SerialNumber));
  }

  if (libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0) {
    libusb_hotplug_register_callback(
        context,
        (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                               LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
        LIBUSB_HOTPLUG_ENUMERATE, current_device.idVendor,
        current_device.idProduct, LIBUSB_HOTPLUG_MATCH_ANY,
        hotplug_callback_entry, this, &hotplug_callback_handle);
  }

  if (libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER) != 0) {
    ret = libusb_kernel_driver_active(device_handle, 0);
    if (ret == 1) {
      if (libusb_detach_kernel_driver(device_handle, 0) == 0) {
      }
    }
  }

  ret = libusb_set_configuration(device_handle, configuration_value);
  if (ret != 0) {
    close();
    return SOCC_ERROR_USB_DEVICE_NOT_FOUND;
  }

  ret = libusb_claim_interface(device_handle, interface_number);
  if (ret != 0) {
    close();
    return SOCC_ERROR_USB_DEVICE_NOT_FOUND;
  }

#if 0
    ret = libusb_set_interface_alt_setting(device_handle, interface_number, alternate_setting);
    if(ret != 0){
        close();
        return SOCC_ERROR_USB_DEVICE_NOT_FOUND;
    }
#endif

  return SOCC_OK;
}

int ports_usb_impl::close() {
  pthread_cancel(thread_id);
  pthread_join(thread_id, NULL);
  pthread_attr_destroy(&thread_attr);

  libusb_hotplug_deregister_callback(context, hotplug_callback_handle);

  delete target_device;

  libusb_close(device_handle);
  libusb_exit(context);

  return SOCC_OK;
}

int ports_usb_impl::write(void* bytes, unsigned int size) {
  int ret = bulk_write(outep, bytes, size);

  if (ret == LIBUSB_ERROR_TIMEOUT) {
    ret = SOCC_ERROR_USB_TIMEOUT;
  } else if (ret == LIBUSB_ERROR_PIPE) {
    ret = SOCC_ERROR_USB_ENDPOINT_HALTED;
  } else if (ret == LIBUSB_ERROR_OVERFLOW) {
    ret = SOCC_ERROR_USB_OVERFLOW;
  } else if (ret == LIBUSB_ERROR_NO_DEVICE) {
    ret = SOCC_ERROR_USB_DISCONNECTED;
  } else if (ret < 0) {
    ret = SOCC_ERROR_USB_OTHER;
  }

  return ret;
}

int ports_usb_impl::read(void* bytes, unsigned int size) {
  if (size > bulk_transfer_max_size) {
    size = bulk_transfer_max_size;
  }
  int ret = bulk_read(inep, bytes, size);

  if (ret == LIBUSB_ERROR_TIMEOUT) {
    ret = SOCC_ERROR_USB_TIMEOUT;
  } else if (ret == LIBUSB_ERROR_PIPE) {
    ret = SOCC_ERROR_USB_ENDPOINT_HALTED;
  } else if (ret == LIBUSB_ERROR_OVERFLOW) {
    ret = SOCC_ERROR_USB_OVERFLOW;
  } else if (ret == LIBUSB_ERROR_NO_DEVICE) {
    ret = SOCC_ERROR_USB_DISCONNECTED;
  } else if (ret < 0) {
    ret = SOCC_ERROR_USB_OTHER;
  }

  return ret;
}

int ports_usb_impl::read_interrupt(void* bytes, unsigned int size) {
  int ret = bulk_read(intep, bytes, size);

  if (ret == LIBUSB_ERROR_TIMEOUT) {
    ret = SOCC_ERROR_USB_TIMEOUT;
  } else if (ret == LIBUSB_ERROR_PIPE) {
    ret = SOCC_ERROR_USB_ENDPOINT_HALTED;
  } else if (ret == LIBUSB_ERROR_OVERFLOW) {
    ret = SOCC_ERROR_USB_OVERFLOW;
  } else if (ret == LIBUSB_ERROR_NO_DEVICE) {
    ret = SOCC_ERROR_USB_DISCONNECTED;
  } else if (ret < 0) {
    ret = SOCC_ERROR_USB_OTHER;
  }

  return ret;
}

int ports_usb_impl::clear_halt(int what) {
  int ret = SOCC_OK;
  if (what != 0) {
    return SOCC_ERROR_INVALID_PARAMETER;
  }

  do {
    ret = libusb_clear_halt(device_handle, inep);
    if (ret != 0) {
      break;
    }

    ret = libusb_clear_halt(device_handle, outep);
    if (ret != 0) {
      break;
    }

    ret = libusb_clear_halt(device_handle, intep);
    if (ret != 0) {
      break;
    }
  } while (0);

  if (ret == LIBUSB_ERROR_NO_DEVICE) {
    ret = SOCC_ERROR_USB_DISCONNECTED;
  } else if (ret == LIBUSB_ERROR_NOT_FOUND) {
    ret = SOCC_ERROR_USB_DISCONNECTED;
  } else if (ret != 0) {
    ret = SOCC_ERROR_USB_OTHER;
  }

  return ret;
}

int ports_usb_impl::reset() {
  int ret = SOCC_OK;
  ret = libusb_reset_device(device_handle);

  if (ret == LIBUSB_ERROR_NO_DEVICE) {
    ret = SOCC_ERROR_USB_DISCONNECTED;
  } else if (ret == LIBUSB_ERROR_NOT_FOUND) {
    ret = SOCC_ERROR_USB_DISCONNECTED;
  } else if (ret != 0) {
    ret = SOCC_ERROR_USB_OTHER;
  }
  return ret;
}

void ports_usb_impl::set_hotplug_callback(
    socc_hotplug_callback_func_t callback_func, void* vp) {
  user_callback_func = callback_func;
  user_callback_data = vp;
}

int ports_usb_impl::snatch_device_handle(socc_device_handle_info_t& info) {
  if (device_handle == NULL) {
    return SOCC_ERROR_NOT_SUPPORT;
  }

  info.device_handle = device_handle;
  info.device_handle_description = "libusb_device_handle";
  info.option = &current_device;
  info.option_description = "usb_device_info_t";

  return SOCC_OK;
}

int ports_usb_impl::bulk_write(int ep, void* bytes, unsigned int size) {
  int ret = 0;
  int actual = 0;
  do {
    int transferred = 0;

    ret = libusb_bulk_transfer(device_handle, ep, (unsigned char*)bytes, size,
                               &transferred, 5000);

    if (ret < 0) {
      return (ret);
    }
    actual += transferred;
    bytes = (unsigned char*)bytes + transferred;
  } while (actual < size);
  return actual;
}

int ports_usb_impl::bulk_read(int ep, void* bytes, unsigned int size) {
  int ret = 0;
  int transferred = 0;
  do {
    ret = libusb_bulk_transfer(device_handle, ep, (unsigned char*)bytes, size,
                               &transferred, 5000);
    if (ret < 0) {
      return (ret);
    }
  } while (transferred == 0);
  return transferred;
}

void* ports_usb_impl::event_thread(void* vp) {
  int ret;
  ports_usb_impl* o = static_cast<ports_usb_impl*>(vp);
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  while (1) {
    pthread_testcancel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    libusb_handle_events_timeout_completed(o->context, &tv, NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  }
  pthread_exit(NULL);
}
int LIBUSB_CALL ports_usb_impl::hotplug_callback_entry(
    libusb_context* ctx, libusb_device* device, libusb_hotplug_event event,
    void* user_data) {
  ports_usb_impl* o = static_cast<ports_usb_impl*>(user_data);

  if (device == o->device) {
    socc_hotplug_event_t hotplug_event;
    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
      hotplug_event = SOCC_HOTPLUG_EVENT_ARRIVED;
    } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
      hotplug_event = SOCC_HOTPLUG_EVENT_REMOVED;
    } else {
      hotplug_event = SOCC_HOTPLUG_EVENT_UNKNOWN;
    }

    if (o->user_callback_func != NULL) {
      o->user_callback_func(hotplug_event, o->user_callback_data);
    }
  }

  return SOCC_OK;
}

ports_usb_impl_target_device::ports_usb_impl_target_device(
    libusb_context* context)
    : context(context),
      device(NULL),
      configuration_value(-1),
      interface_number(-1),
      alternate_setting(-1),
      inep(-1),
      outep(-1),
      intep(-1) {
  memset(&current_device, 0, sizeof(current_device));
}

ports_usb_impl_target_device::~ports_usb_impl_target_device() {
  if (device != NULL) {
    libusb_unref_device(device);
  }
}

libusb_device* ports_usb_impl_target_device::find_device(
    int busn, int devn, libusb_class_code code) {
  int count;
  libusb_device** devs = NULL;
  bool found = false;

  device = NULL;

  count = libusb_get_device_list(context, &devs);

  for (int i = 0; i < count && found == false; i++) {
    int index = 0;
    while (found == false) {
      struct libusb_config_descriptor* config_descriptor = NULL;
      const struct libusb_interface_descriptor* altsetting = NULL;

      config_descriptor = find_configurations(*(devs[i]), index++);
      if (config_descriptor == NULL) {
        break;
      }

      configuration_value = config_descriptor->bConfigurationValue;

      if ((busn != 0 && devn != 0) &&
          (busn != current_device.busn || devn != current_device.devn)) {
        continue;
      }

      altsetting =
          find_altsetting_by_class_code(*config_descriptor, LIBUSB_CLASS_PTP);
      if (altsetting != NULL) {
        interface_number = altsetting->bInterfaceNumber;
        alternate_setting = altsetting->bAlternateSetting;

        if (configure_endpoint(*altsetting, inep, outep, intep)) {
          found = true;
          device = devs[i];
        }
      }
      libusb_free_config_descriptor(config_descriptor);
    }
  }

  if (found == true) {
    libusb_ref_device(device);
  }

  libusb_free_device_list(devs, 1);
  return device;
}

void ports_usb_impl_target_device::get_settings(int& config, int& inter,
                                                int& alt) {
  config = configuration_value;
  inter = interface_number;
  alt = alternate_setting;
}
void ports_usb_impl_target_device::get_endpoint(int& inep, int& outep,
                                                int& intep) {
  inep = this->inep;
  outep = this->outep;
  intep = this->intep;
}

const struct libusb_interface_descriptor*
ports_usb_impl_target_device::find_altsetting_by_class_code(
    libusb_config_descriptor& config_descriptor, libusb_class_code code) {
  const struct libusb_interface_descriptor* altsetting = NULL;
  for (int i = 0; i < config_descriptor.bNumInterfaces; i++) {
    for (int j = 0; j < config_descriptor.interface[i].num_altsetting; j++) {
      altsetting = &config_descriptor.interface[i].altsetting[j];
      if (altsetting->bInterfaceClass == code) {
        current_device.bInterfaceClass = code;
        return altsetting;
      }
    }
  }
  return NULL;
}

struct libusb_config_descriptor*
ports_usb_impl_target_device::find_configurations(libusb_device& device,
                                                  int index) {
  struct libusb_config_descriptor* config_descriptor = NULL;
  struct libusb_device_descriptor device_descriptor;

  if (libusb_get_device_descriptor(&device, &device_descriptor) != 0) {
    return NULL;
  }

  current_device.idVendor = device_descriptor.idVendor;
  current_device.idProduct = device_descriptor.idProduct;
  current_device.iManufacturer = device_descriptor.iManufacturer;
  current_device.iProduct = device_descriptor.iProduct;
  current_device.iSerialNumber = device_descriptor.iSerialNumber;
  current_device.busn = libusb_get_bus_number(&device);
  current_device.devn = libusb_get_device_address(&device);

  if (index >= device_descriptor.bNumConfigurations) {
    return NULL;
  }
  libusb_get_config_descriptor(&device, index, &config_descriptor);
  return config_descriptor;
}

bool ports_usb_impl_target_device::configure_endpoint(
    const struct libusb_interface_descriptor& altsetting, int& inep, int& outep,
    int& intep) {
  inep = -1;
  outep = -1;
  intep = -1;
  for (int i = 0; i < altsetting.bNumEndpoints; i++) {
    if ((altsetting.endpoint[i].bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) ==
        LIBUSB_TRANSFER_TYPE_BULK) {
      if (altsetting.endpoint[i].bEndpointAddress & LIBUSB_ENDPOINT_IN) {
        inep = altsetting.endpoint[i].bEndpointAddress;
      } else {
        outep = altsetting.endpoint[i].bEndpointAddress;
      }
    } else if ((altsetting.endpoint[i].bmAttributes &
                LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_INTERRUPT) {
      if (altsetting.endpoint[i].bEndpointAddress & LIBUSB_ENDPOINT_IN) {
        intep = altsetting.endpoint[i].bEndpointAddress;
      }
    }
  }

  current_device.inep = inep;
  current_device.outep = outep;
  current_device.intep = intep;

  if (inep < 0 && outep < 0 && intep < 0) {
    return false;
  }
  return true;
}
