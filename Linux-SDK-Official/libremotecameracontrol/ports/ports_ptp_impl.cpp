#include "ports_ptp_impl.h"

#include <socc_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ports_usb.h"
#include "ports_usb_impl.h"

using namespace com::sony::imaging::ports;

ports_ptp_impl::ports_ptp_impl(int busn, int devn, uint32_t session_id,
                               uint32_t transaction_id, ports_usb* usb)
    : session_id(session_id), transaction_id(transaction_id), usb(usb) {}
ports_ptp_impl::~ports_ptp_impl() {}

int ports_ptp_impl::send(uint16_t code, uint32_t* parameters, uint8_t num,
                         com::sony::imaging::remote::Container& response,
                         void* data, uint32_t size) {
  int rc;
  memset(&response, 0, sizeof(response));

  if (code == 0x1002) {
    session_id = 0;
    transaction_id = 0;
  }

  rc = sendreq(code, parameters, num);
  if (rc != 0) {
    return rc;
  }

  if (data != NULL && size > 0) {
    rc = senddata(code, parameters, num, data, size);
    if (rc != 0) {
      return rc;
    }
  }

  rc = getresp(response);
  if (rc != 0) {
    return rc;
  }

  transaction_id++;

  return SOCC_OK;
}

int ports_ptp_impl::receive(uint16_t code, uint32_t* parameters, uint8_t num,
                            com::sony::imaging::remote::Container& response,
                            void** data, uint32_t& size) {
  int rc;
  memset(&response, 0, sizeof(response));
  size = 0;

  rc = sendreq(code, parameters, num);

  if (rc != 0) {
    return rc;
  }

  rc = getdata(data, size);
  if (rc != 0) {
    return rc;
  }

  rc = getresp(response);
  if (rc != 0) {
    return rc;
  }

  transaction_id++;

  return SOCC_OK;
}

int ports_ptp_impl::wait_event(
    com::sony::imaging::remote::Container& container) {
  int rc;
  memset(&container, 0, sizeof(container));

  rc = getevent(container);
  if (rc != 0) {
    return rc;
  }

  return SOCC_OK;
}
void ports_ptp_impl::dispose_data(void** data) {
  if (*data != NULL) {
    free(*data);
  }
  *data = NULL;
}

int ports_ptp_impl::sendreq(uint16_t code, uint32_t* parameters, uint8_t num) {
  int ret;
  GenericBulkContainerHeader* header;
  uint32_t* payload;
  uint32_t length;
  void* vp = calloc(1, BULK_MAX_PACKET_SIZE);
  length = sizeof(GenericBulkContainerHeader) + sizeof(uint32_t) * num;

  header = (GenericBulkContainerHeader*)vp;
  header->length = length;
  header->type = 0x0001; /* Command Block */
  header->code = code;
  header->transaction_id = transaction_id;

  header++;

  payload = (uint32_t*)header;
  memcpy(payload, parameters, sizeof(uint32_t) * num);

  int actual = usb->write(vp, length);
  free(vp);

  if (actual < 0) {
    return actual;
  }
  return SOCC_OK;
}

int ports_ptp_impl::senddata(uint16_t code, uint32_t* parameters, uint8_t num,
                             void* data, unsigned int size) {
  int ret;
  GenericBulkContainerHeader* header;
  uint32_t* payload;
  uint32_t length = sizeof(GenericBulkContainerHeader) + size;
  void* vp = calloc(1, length);

  header = (GenericBulkContainerHeader*)vp;
  header->length = length;
  header->type = 0x0002; /* Data Block */
  header->code = code;
  header->transaction_id = transaction_id;

  header++;

  payload = (uint32_t*)header;
  memcpy(payload, data, size);

  int actual = usb->write(vp, length);
  free(vp);

  if (actual < 0) {
    return actual;
  }
  return SOCC_OK;
}

int ports_ptp_impl::getdata(void** data, uint32_t& size) {
  int ret;
  GenericBulkContainerHeader* header;
  uint32_t* payload;
  uint32_t length;
  void* vp = calloc(1, BULK_MAX_PACKET_SIZE);
  length = BULK_MAX_PACKET_SIZE;

  int actual = usb->read(vp, length);

  if (actual < 0) {
    free(vp);
    return actual;
  }

  header = (GenericBulkContainerHeader*)vp;

  if (header->type != 0x0002) {
    free(vp);
    return SOCC_PTP_ERROR_TRANSACTION;
  } else {
    int payload_length = header->length;
    unsigned char* cp = (unsigned char*)calloc(1, payload_length);
    *data = cp;

    header++;
    memcpy(cp, header, actual - sizeof(GenericBulkContainerHeader));
    cp += actual - sizeof(GenericBulkContainerHeader);

    while (actual < payload_length) {
      int rs = usb->read(cp, payload_length - actual);
      if (rs < 0) {
        free(vp);
        return rs;
      }
      actual += rs;
      cp += rs;
    }
    size = (actual > sizeof(GenericBulkContainerHeader))
               ? actual - sizeof(GenericBulkContainerHeader)
               : 0;
  }

  free(vp);
  return SOCC_OK;
}

int ports_ptp_impl::getresp(com::sony::imaging::remote::Container& response) {
  GenericBulkContainerHeader* header;
  uint32_t* payload;
  uint32_t length;
  void* vp = calloc(1, BULK_MAX_PACKET_SIZE);
  length = BULK_MAX_PACKET_SIZE;

  int actual = usb->read(vp, length);

  if (actual < 0) {
    free(vp);
    return actual;
  }

  header = (GenericBulkContainerHeader*)vp;

  if (header->type != 0x0003) {
    free(vp);
    return SOCC_PTP_ERROR_TRANSACTION;
  } else {
    int nparam;
    int payload_length = header->length;
    response.code = header->code;
    response.session_id = session_id;
    response.transaction_id = header->transaction_id;

    nparam = (header->length - sizeof(GenericBulkContainerHeader)) /
             sizeof(uint32_t);
    response.nparam = nparam;

    payload = (uint32_t*)(header + 1);
    memcpy(&response.param1, payload, nparam * sizeof(uint32_t));
  }

  free(vp);
  return SOCC_OK;
}

int ports_ptp_impl::getevent(com::sony::imaging::remote::Container& event) {
  GenericBulkContainerHeader* header;
  uint32_t* payload;
  uint32_t length;
  void* vp = calloc(1, BULK_MAX_PACKET_SIZE);
  length = BULK_MAX_PACKET_SIZE;

  int actual = usb->read_interrupt(vp, length);

  if (actual < 0) {
    free(vp);
    return actual;
  }

  header = (GenericBulkContainerHeader*)vp;

  if (header->type != 0x0004) {
    free(vp);
    return SOCC_PTP_ERROR_TRANSACTION;
  } else {
    int nparam;
    event.code = header->code;
    event.session_id = session_id;
    event.transaction_id = header->transaction_id;

    nparam = (header->length - sizeof(GenericBulkContainerHeader)) /
             sizeof(uint32_t);
    event.nparam = nparam;

    payload = (uint32_t*)(header + 1);
    memcpy(&event.param1, payload, nparam * sizeof(uint32_t));
  }

  free(vp);
  return SOCC_OK;
}
