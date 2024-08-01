#include <ports_ptp.h>
#include <ports_ptp_impl.h>
#include <ports_usb.h>
#include <ports_usb_impl.h>
#include <socc_ptp.h>
#include <socc_types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

using namespace com::sony::imaging::remote;

socc_ptp::socc_ptp(int32_t busn, int32_t devn) : busn(busn), devn(devn) {
  usb = new com::sony::imaging::ports::ports_usb_impl(busn, devn);
  ptp = new com::sony::imaging::ports::ports_ptp_impl(busn, devn, 1, 0, usb);
}
socc_ptp::~socc_ptp() {
  if (ptp != NULL) {
    delete ptp;
  }
  if (usb != NULL) {
    delete usb;
  }
}

int socc_ptp::connect() { return usb->open(); }

int socc_ptp::disconnect() { return usb->close(); }

void socc_ptp::set_hotplug_callback(socc_hotplug_callback_func_t callback_func,
                                    void* vp) {
  usb->set_hotplug_callback(callback_func, vp);
}

int socc_ptp::send(uint16_t code, uint32_t* params, uint8_t nparam,
                   Container& response, void* data, uint32_t size) {
  return ptp->send(code, params, nparam, response, data, size);
}

int socc_ptp::receive(uint16_t code, uint32_t* params, uint8_t nparam,
                      Container& response, void** data, uint32_t& size) {
  return ptp->receive(code, params, nparam, response, data, size);
}

int socc_ptp::wait_event(Container& container) {
  return ptp->wait_event(container);
}

void socc_ptp::dispose_data(void** data) { ptp->dispose_data(data); }

int socc_ptp::clear_halt(int what) { return usb->clear_halt(what); }

int socc_ptp::reset() { return usb->reset(); }
