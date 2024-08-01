/**
 * @file socc_types.h
 * @brief Header file of libremotecameracontrol
 */
#ifndef __SOCC_TYPES_H__
#define __SOCC_TYPES_H__
#include <stdint.h>

/**
 * \enum Hot plug event
 */
typedef enum {
  SOCC_HOTPLUG_EVENT_UNKNOWN = 0,
  SOCC_HOTPLUG_EVENT_ARRIVED = 1,  //!<  Claimed PTP device has arrived
  SOCC_HOTPLUG_EVENT_REMOVED = 2   //!<  Claimed PTP device has removed
} socc_hotplug_event_t;

/**
 * \enum error code
 */
enum {
  SOCC_OK = 0,
  SOCC_ERROR_NOT_SUPPORT = -1,
  SOCC_ERROR_INVALID_PARAMETER = -2,

  SOCC_ERROR_USB_INIT = -101,
  SOCC_ERROR_USB_DEVICE_NOT_FOUND = -102,
  SOCC_ERROR_USB_OPEN = -103,
  SOCC_ERROR_USB_TIMEOUT = -104,
  SOCC_ERROR_USB_ENDPOINT_HALTED = -105,
  SOCC_ERROR_USB_OVERFLOW = -106,
  SOCC_ERROR_USB_DISCONNECTED = -107,
  SOCC_ERROR_USB_OTHER = -108,

  SOCC_ERROR_THREAD_INIT = -201,
  SOCC_ERROR_THREAD_CREATE = -202,

  SOCC_PTP_ERROR_TRANSACTION = -301,
};

/**
 * \typedef type of hotplug callback function
 */
typedef void (*socc_hotplug_callback_func_t)(socc_hotplug_event_t, void*);

typedef struct __socc_device_handle_info_t {
  void* device_handle;
  const char* device_handle_description;
  void* option;
  const char* option_description;
} socc_device_handle_info_t;

namespace com {
namespace sony {
namespace imaging {
namespace remote {

/**
 * \struct Container structur for Request Dataset, Response Dataset, Event
 * Dataset
 */
typedef struct Container {
  uint16_t code;  //!< OperationCode for  Request Dataset, ResponseCode for
                  //!< Response Dataset, EventCode for Event Dataset
  uint32_t session_id;  //!< Session ID for Request Dataset, Response Dataset,
                        //!< Event Dataset
  uint32_t transaction_id;  //!< TransactionID for Request Dataset, Response
                            //!< Dataset, Event Dataset
  uint32_t param1;          //!< Parameter1
  uint32_t param2;          //!< Parameter2
  uint32_t param3;          //!< Parameter3
  uint32_t param4;          //!< Parameter4
  uint32_t param5;          //!< Parameter5
  uint8_t nparam;           //!< number of valid parameters above
} Container;

}  // namespace remote
}  // namespace imaging
}  // namespace sony
}  // namespace com

#endif
