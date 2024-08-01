/**
 * @page libremotecameracontrol The document of the libremotecameracontrol part
 * @section abstract Abstract
 * libremotecameracontrol is a sample library to perform PTP/USB connection and PTP
transfer.\n
 * This library is used from remote cammera controls frontend and examples.\n
 * This library is designed for libusb-1.0 as USB backend.
 * If your system has your own USB backend or PTP backend, please port
libremotecameracontrol with your backends.
 *
 * @section guide Porting guide
 * libremotecameracontrol has device handling, synchronous transaction, hotplug detection
methods.\n
 * \n
 * @par Mandatory and optional method
 * [MANDATORIES] connect(), disconnect(), send(), receive()
,set_hotplug_callback(), dispose_data() and wait_event()\n
 * [OPTIONALS] clear_halt() and reset()
 * \n
 * @par Return codes
 * If methods has return value, 0 on success and other value on failure\n
 * \n
 * @par Device handling
 * com::sony::imaging::remote::socc_ptp::connect()\n
 * connect() should connect to the target device, and make it possible to start
PTP Transaction. \n
 * Bus number and device address of target device should be given with the
constructor's parameters.\n
 * Our implementation try to connect automatically for the first PTP device
founded in enumeration, if both bus number and device address are zero.\n
 * \n
 * com::sony::imaging::remote::socc_ptp::disconnect()\n
 * disconnect() should disconnect from the target device, and release every
resources if needed\n
 * \n
 * @par Synchronous transaction
 * com::sony::imaging::remote::socc_ptp::send(uint16_t code, uint32_t* params,
uint8_t nparam, Container& response, void* data, uint32_t size)\n
 * com::sony::imaging::remote::socc_ptp::receive(uint16_t code, uint32_t*
params, uint8_t nparam, Container& response, void** data, uint32_t& size)\n
 * send() and receive() are method to perfom PTP transactions.\n
 * \n
 * Both methods have some common paramters.\n
 *   - uint16_t code : OpperationCode in OperationRequest Dataset
 *   - uint32_t* params : uint32_t array for parameter fields in request phase
 *   - uint8_t nparam : count of parameters
 *   - Container& reponse : Response Dataset of response phase\n
 *     Please set the result of response phase in Response Dataset format, when
transaction complete successfully\n
 *     Container is defind in .socc_types.h as follow.
\code
    typedef struct _Container{
        uint16_t code; // OperationCode for  Request Dataset, ResponseCode for
Response Dataset, EventCode for Event Dataset uint32_t session_id; // Session ID
for Request Dataset, Response Dataset, Event Dataset uint32_t transaction_id;
//!< TransactionID for Request Dataset, Response Dataset, Event Dataset uint32_t
param1; // Parameter1 uint32_t param2; // Parameter2 uint32_t param3; //
Parameter3 uint32_t param4; // Parameter4 uint32_t param5; // Parameter5 uint8_t
nparam; // number of valid parameters above } Container; \endcode
 *   .
 * \n
 * send() should perform sending transaction with/without data phase.\n
 * If the transacation has a data phase, data should be the pointer of buffer to
send and size should be the length of buffer in byte.\n
 * If the transcation doesn't have any data phase, data should be NULL and size
should be 0;\n
 * \n
 * receive() should perform receiving transaction with data phase.\n
 * Please allocate buffer needed to receive from device in this method, and
implement dispose() method to dispose buffer allocated by receive() method.\n
 * Actual transferred size in byte should be set in size paramter on success.\n
 * \n
 * com::sony::imaging::remote::socc_ptp::wait_event(Container& container)\n
 * wait_event() should wait PTP event, and copy acquired PTP event container to
Container& container when your backend detects PTP event.\n
 * Format of Container is different from PTP Event Dataset format. Please
translate each field.
 * \n
 * \n
 * com::sony::imaging::remote::socc_ptp::dispose_data(void** data)\n
 * dispose_data() should free allocated buffer by receive(), and set it's
pointer to NULL.\n
 * @par Hotplug
 * com::sony::imaging::remote::socc_ptp::set_hotplug_callback(socc_hotplug_callback_func_t callback_func, void* vp)\n
 * set_hotplug_callback() registers callback function for USB hot plug
detection.\n
 * Type of callback function is socc_hotplug_callback_func_t defined in
.socc_types.h\n
 * First parameter of callback stands for USB connection status.
 * Call this callback with socc_hotplug_event_t::SOCC_HOTPLUG_EVENT_ARRIVED when
your backend detects USB bus connection,
socc_hotplug_event_t::SOCC_HOTPLUG_EVENT_REMOVED for disconnection.\n
 * These enum value are defined in .socc_types.h as a type.
 * Second parmeter of callback is user data. Set a same pointer given by
set_hotplug_callback() method.\n
 * socc_hotplug_callback_func_t and socc_hotplug_event_ are defined in
.socc_types.h
 * In our sample, libremotecameracontrol detects hotplug event after calling connect() method
succesfully, only for a device claimed to use. \code typedef enum {
    SOCC_HOTPLUG_EVENT_UNKNOWN = 0,
    SOCC_HOTPLUG_EVENT_ARRIVED = 1,
    SOCC_HOTPLUG_EVENT_REMOVED = 2
  } socc_hotplug_event_t;

  typedef void(*socc_hotplug_callback_func_t)(socc_hotplug_event_t, void*);
\endcode
 * \n
 * @par Optionals
 * com::sony::imaging::remote::socc_ptp::clear_halt(int what = 0) [OPTIONAL]\n
 * If your backend needs to clear HALT/STALL status of USB endpoints from
frontend, implement this method. \n
 * In our sample, libremotecameracontrol clears BULK IN, BULK OUT and Interrupt IN endpoints
at sametime, if paramter "what" is zero.\n
 * \n
 * com::sony::imaging::remote::socc_ptp::reset() [OPTIONAL]\n
 * If your backend needs to reset USB bus from frotend, implement this. method\n
 * In our sample, libremotecameracontrol just call reset function of backend.\n
 * \n
 */

/**
 * @file socc_ptp.h
 * @brief libremotecameracontrol class
 */

#ifndef __SOCC_PTP_H__
#define __SOCC_PTP_H__
#include <socc_types.h>
#include <stdio.h>

namespace com {
namespace sony {
namespace imaging {
namespace ports {
class ports_usb;
class ports_ptp;
}  // namespace ports
}  // namespace imaging
}  // namespace sony
}  // namespace com

namespace com {
namespace sony {
namespace imaging {
namespace remote {

/**
 * @class socc_ptp
 * @brief Class for USB/PTP connection and PTP transfer
 */
class socc_ptp {
 public:
  /**
   * @brief Constructor
   *
   * @param [in]bus bus number of target device
   * @param [in]dev device address of target device
   */
  socc_ptp(int32_t bus = 0, int32_t dev = 0);

  /**
   * @brief Destructor
   */
  ~socc_ptp();

  /**
   * @brief [MANDATORY] Connect to target device and set configuration as PTP
   * device
   * @return 0 on success, other on failure
   */
  int connect();

  /**
   * @brief [MANDATORY] Diconnect target device
   * @return 0 on success, other on failure
   */
  int disconnect();

  /**
   * @brief [MANDATORY] Register callback function for USB hot plug detection
   * @param [in]callback_func the function to be invoked on USB connection or
   * disconnection
   * @param [in]vp user data
   */
  void set_hotplug_callback(socc_hotplug_callback_func_t callback_func,
                            void* vp);

  /**
   * @brief [MANDATORY] Perform sending transaction with/without data phase
   * @param [in]code OperationCode
   * @param [in]params  uint32_t array of parameters
   * @param [in]nparam number of parameters
   * @param [out]response Response Dataset
   * @param [in]*data data to transfer in data phase. Set NULL if data is absent
   * @param [in]size size in byte of data. Set 0 if data is absent
   * @return 0 on success, other on failure
   */
  int send(uint16_t code, uint32_t* params, uint8_t nparam, Container& response,
           void* data, uint32_t size);

  /**
   * @brief [MANDATORY] Perform receiving transaction with data phase
   * @param [in]code OperationCode
   * @param [in]params  uint32_t array of parameters
   * @param [in]nparam number of parameters
   * @param [out]response Response Dataset
   * @param [out]**data If this method returns on success, transferrd data will
   * be set in this buffer. This buffer is allocated in this method. This buffer
   * should be disposed with dispose() method.
   * @param [out]size actual transfferd data size in byte. This value shoud be
   * set in this method.
   * @return 0 on success, other on failure
   */
  int receive(uint16_t code, uint32_t* params, uint8_t nparam,
              Container& response, void** data, uint32_t& size);

  /**
   * @brief [MANDATORY] Wait for event
   * @param [out]container acquired container in Event Dataset format
   * @return 0 on success, other on failure
   */
  int wait_event(Container& container);

  /**
   * @brief [MANDATORY] Dispose data buffer allocated in receive method, and set
   * the pointer to NULL.
   * @param [in]**data data buffer to dispose.
   */
  void dispose_data(void** data);

  /**
   * @brief [OPTIONAL] Clear the halt or stall condition for endpoints
   * @param [in]what the USB endpoint to clear halt. 0 for all USB endpoint
   * @return 0 on success, other on failure
   */
  int clear_halt(int what = 0);

  /**
   * @brief [OPTIONAL] Reset USB port
   * @return 0 on success, other on failure
   */
  int reset();

 private:
  int32_t busn;
  int32_t devn;
  com::sony::imaging::ports::ports_usb* usb;
  com::sony::imaging::ports::ports_ptp* ptp;
};

}  // namespace remote
}  // namespace imaging
}  // namespace sony
}  // namespace com
#endif
