#include "parser.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define le16atoh(x) ((uint16_t)(((x)[1] << 8) | (x)[0]))
#define FORMAT(__v, __t) \
  ((__v) & (0xFFFFFFFFFFFFFFFF >> (64 - sizeof(__t) * 8)))

using namespace com::sony::imaging::remote;

static void strsprintf(std::string &str, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  char *alloc;
  if (vasprintf(&alloc, format, ap) == -1) {
    str.append("");
  }
  va_end(ap);
  str.append(alloc);
  free(alloc);
}

SDIDevicePropInfoDataset *SDIDevicePropInfoDataset::create(void *data) {
  char *_data = (char *)data;
  _data += sizeof(uint16_t);

  uint16_t DataType = *(uint16_t *)_data;
  // printf("DataType=0x%zu\n", DataType);
  // printf("DataType=0x%x\n", DataType);

  SDIDevicePropInfoDataset *ret = NULL;

  switch (DataType) {
    case 0x0001:  // INT8
      ret = new DataTypeInteger<int8_t>();
      break;
    case 0x0002:  // UINT8
      ret = new DataTypeInteger<uint8_t>();
      break;
    case 0x0003:  // INT16
      ret = new DataTypeInteger<int16_t>();
      break;
    case 0x0004:  // UINT16
      ret = new DataTypeInteger<uint16_t>();
      break;
    case 0x0005:  // INT32
      ret = new DataTypeInteger<int32_t>();
      break;
    case 0x0006:  // UINT32
      ret = new DataTypeInteger<uint32_t>();
      break;
    case 0x0007:  // INT64
      ret = new DataTypeInteger<int64_t>();
      break;
    case 0x0008:  // UINT64
      ret = new DataTypeInteger<uint64_t>();
      break;
    case 0x0009:  // INT128
      ret = new DataTypeInteger<int64_t>();
      break;
    case 0x000A:  // UINT128
      ret = new DataTypeInteger<uint64_t>();
      break;
    case 0x4001:  // AINT8
      ret = new DataTypeArray<int8_t>();
      break;
    case 0x4002:  // AUINT8
      ret = new DataTypeArray<uint8_t>();
      break;
    case 0x4003:  // AINT16
      ret = new DataTypeArray<int16_t>();
      break;
    case 0x4004:  // AUINT16
      ret = new DataTypeArray<uint16_t>();
      break;
    case 0x4005:  // AINT32
      ret = new DataTypeArray<int32_t>();
      break;
    case 0x4006:  // AUINT32
      ret = new DataTypeArray<uint32_t>();
      break;
    case 0x4007:  // AINT64
      ret = new DataTypeArray<int64_t>();
      break;
    case 0x4008:  // AUINT64
      ret = new DataTypeArray<uint64_t>();
      break;
    case 0x4009:  // AINT128
      ret = new DataTypeArray<int64_t>();
      break;
    case 0x400A:  // AUINT128
      ret = new DataTypeArray<uint64_t>();
      break;
    case 0xFFFF:  // STR
      ret = new DataTypeSTR();
      break;

    default:
      return NULL;
  }

  return ret->parse(data);
}

SDIDevicePropInfoDataset::~SDIDevicePropInfoDataset() {}

void SDIDevicePropInfoDataset::parseValues(char **data) {}

void SDIDevicePropInfoDataset::parseForm(char **data) {}

void SDIDevicePropInfoDataset::toString(std::string &str) {
  strsprintf(str, "  dataset DevicePropertyCode: %04X\n", DevicePropertyCode);
  strsprintf(str, "  dataset DataType: %04X\n", DataType);
  strsprintf(str, "  dataset GetSet: %02X\n", GetSet);
  strsprintf(str, "  dataset IsEnable: %02X\n", IsEnable);
  strsprintf(str, "  dataset FormFlag: %02X\n", FormFlag);
}

void SDIDevicePropInfoDataset::toString() {
  std::string str;
  toString(str);
  printf("%s", str.c_str());
}

SDIDevicePropInfoDataset *SDIDevicePropInfoDataset::parse(void *data) {
  char *_data = (char *)data;
  DevicePropertyCode = *(uint16_t *)_data;
  _data += sizeof(uint16_t);
  DataType = *(uint16_t *)_data;
  _data += sizeof(uint16_t);
  GetSet = *(uint8_t *)_data;
  _data += sizeof(uint8_t);
  IsEnable = *(uint8_t *)_data;
  _data += sizeof(uint8_t);
  parseValues(&_data);
  FormFlag = *(uint8_t *)_data;
  _data += sizeof(uint8_t);
  parseForm(&_data);
  mSize = _data - (char *)data;

  return this;
}

template <typename T>
DataTypeInteger<T>::~DataTypeInteger() {
  if (Values) {
    delete[] Values;
    Values = NULL;
  }
}

template <typename T>
void DataTypeInteger<T>::parseValues(char **data) {
  DefaultValue = *(T *)*data;
  *data += sizeof(T);
  CurrentValue = *(T *)*data;
  *data += sizeof(T);
}

template <typename T>
void DataTypeInteger<T>::parseForm(char **data) {
  MinimumValue = 0;
  MaximumValue = 0;
  StepSize = 0;
  NumOfValues = 0;
  NumOfValues_2nd = 0;
  Values = NULL;
  switch (FormFlag) {
    case 0x00:  // None
      break;
    case 0x01:  // Range-Form
      MinimumValue = *(T *)*data;
      *data += sizeof(T);
      MaximumValue = *(T *)*data;
      *data += sizeof(T);
      StepSize = *(T *)*data;
      *data += sizeof(T);
      break;
    case 0x02:  // Enumeration-Form
      NumOfValues = *(uint16_t *)*data;
      *data += sizeof(uint16_t);
      Values = new T[NumOfValues];
      for (uint16_t i = 0; i < NumOfValues; i++) {
        Values[i] = *(T *)*data;
        *data += sizeof(T);
      }

      // Added to support ver300 06/18/2020 Koki
      NumOfValues_2nd = *(uint16_t *)*data;
      *data += sizeof(uint16_t);
      Values = new T[NumOfValues_2nd];
      for (uint16_t i = 0; i < NumOfValues_2nd; i++) {
        Values[i] = *(T *)*data;
        *data += sizeof(T);
      }
  }
}

template <typename T>
void DataTypeInteger<T>::toString(std::string &str) {
  SDIDevicePropInfoDataset::toString(str);
  strsprintf(str, "  dataset DefaultValue: %0*llX\n", sizeof(T) * 2,
             FORMAT(DefaultValue, T));
  strsprintf(str, "  dataset CurrentValue: %0*llX\n", sizeof(T) * 2,
             FORMAT(CurrentValue, T));

  switch (FormFlag) {
    case 0x00:  // None
      break;
    case 0x01:  // Range-Form
      strsprintf(str, "  dataset Range-Form\n");
      strsprintf(str, "    dataset MinimumValue: %0*llX\n", sizeof(T) * 2,
                 FORMAT(MinimumValue, T));
      strsprintf(str, "    dataset MaximumValue: %0*llX\n", sizeof(T) * 2,
                 FORMAT(MaximumValue, T));
      strsprintf(str, "    dataset StepSize: %0*llX\n", sizeof(T) * 2,
                 FORMAT(StepSize, T));
      break;
    case 0x02:  // Enumeration-Form
      strsprintf(str, "  dataset Enumeration-Form\n");
      strsprintf(str, "    dataset NumOfValues: %u\n", NumOfValues);

      for (uint16_t i = 0; i < NumOfValues; i++) {
        strsprintf(str, "    dataset Value: %0*llX\n", sizeof(T) * 2,
                   FORMAT(Values[i], T));
      }
  }
}

template <typename T>
void DataTypeInteger<T>::toString() {
  std::string str;
  toString(str);
  printf("%s", str.c_str());
}

template <typename T>
DataTypeArray<T>::~DataTypeArray() {
  if (DefaultValues) {
    delete[] DefaultValues;
    DefaultValues = NULL;
  }
  if (CurrentValues) {
    delete[] CurrentValues;
    CurrentValues = NULL;
  }
}

template <typename T>
void DataTypeArray<T>::parseValues(char **data) {
  char **_data = (char **)data;
  NumOfDefaultValues = *(uint32_t *)*_data;
  *_data += sizeof(uint32_t);
  DefaultValues = new T[NumOfDefaultValues];
  for (uint32_t i = 0; i < NumOfDefaultValues; i++) {
    DefaultValues[i] = *(T *)*_data;
    *_data += sizeof(T);
  }

  NumOfCurrentValues = *(uint32_t *)*_data;
  *_data += sizeof(uint32_t);
  CurrentValues = new T[NumOfCurrentValues];
  for (uint32_t i = 0; i < NumOfCurrentValues; i++) {
    CurrentValues[i] = *(T *)*_data;
    *_data += sizeof(T);
  }
}

template <typename T>
void DataTypeArray<T>::parseForm(char **data) {
  if (0x00 != FormFlag) {
    printf(
        "DevicePropertyCode: 0x%04X: FormFlag is not 0, this is not "
        "supported\n",
        DevicePropertyCode);
  }
}

template <typename T>
void DataTypeArray<T>::toString(std::string &str) {
  SDIDevicePropInfoDataset::toString(str);
  strsprintf(str, "  dataset NumOfDefaultValues: %u\n", NumOfDefaultValues);
  for (uint32_t i = 0; i < NumOfDefaultValues; i++) {
    strsprintf(str, "  dataset   DefaultValue: %0*llX\n", sizeof(T) * 2,
               FORMAT(DefaultValues[i], T));
  }
  strsprintf(str, "  dataset NumOfCurrentValues: %u\n", NumOfCurrentValues);
  for (uint32_t i = 0; i < NumOfCurrentValues; i++) {
    strsprintf(str, "  dataset   CurrentValue: %0*llX\n", sizeof(T) * 2,
               FORMAT(CurrentValues[i], T));
  }
}

template <typename T>
void DataTypeArray<T>::toString() {
  std::string str;
  toString(str);
  printf("%s", str.c_str());
}

DataTypeSTR::~DataTypeSTR() {
  delete DefaultValue;
  delete CurrentValue;
}

void DataTypeSTR::parseValues(char **data) {
  uint8_t len;
  // DefaultValue
  length_DefaultValue = len = *(uint8_t *)*data;
  *data += sizeof(uint8_t);
  DefaultValue = new char[len];
  memset(DefaultValue, 0, len);
  for (int i = 0; i < len && i < PTP_MAXSTRLEN; i++) {
    DefaultValue[i] = (char)le16atoh(*data + i * 2);
  }
  *data += sizeof(uint16_t) * len;

  // CurrentValue
  length_CurrentValue = len = *(uint8_t *)*data;
  *data += sizeof(uint8_t);
  CurrentValue = new char[len];
  memset(CurrentValue, 0, len);
  for (int i = 0; i < len && i < PTP_MAXSTRLEN; i++) {
    CurrentValue[i] = (char)le16atoh(*data + i * 2);
  }
  *data += sizeof(uint16_t) * len;
}

void DataTypeSTR::parseForm(char **data) {
  if (0x00 != FormFlag) {
    printf(
        "DevicePropertyCode: 0x%04X: FormFlag is not 0, this is not "
        "supported\n",
        DevicePropertyCode);
  }
}

void DataTypeSTR::toString(std::string &str) {
  SDIDevicePropInfoDataset::toString(str);
  strsprintf(str, "  dataset DefaultValue: \"%s\"\n", DefaultValue);
  strsprintf(str, "  dataset CurrentValue: \"%s\"\n", CurrentValue);
}

void DataTypeSTR::toString() {
  std::string str;
  toString(str);
  printf("%s", str.c_str());
}

SDIDevicePropInfoDatasetArray::SDIDevicePropInfoDatasetArray(void *data) {
  char *_data = (char *)data;

  num = 0;

  if (NULL == data) {
    goto bail;
  }

  num = *(uint64_t *)_data;
  _data += sizeof(uint64_t);

  // printf("num=%lu\n", num);

  for (uint64_t i = 0; i < num; i++) {
    SDIDevicePropInfoDataset *d =
        SDIDevicePropInfoDataset::create((void *)_data);
    dataset[d->DevicePropertyCode] = d;
    _data += d->size();
  }

bail:
  return;
}

SDIDevicePropInfoDataset *SDIDevicePropInfoDatasetArray::get(
    uint16_t device_property_code) {
  SDIDevicePropInfoDataset *ret = NULL;
  if (dataset.find(device_property_code) != dataset.end()) {
    ret = dataset[device_property_code];
  }
  return ret;
}

SDIDevicePropInfoDatasetArray::~SDIDevicePropInfoDatasetArray() {
  SDIDevicePropInfoDataset *data;
  std::map<uint16_t, SDIDevicePropInfoDataset *>::iterator it;

  for (it = dataset.begin(); it != dataset.end(); it++) {
    data = (*it).second;
    delete data;
  }
  dataset.clear();
}

void SDIDevicePropInfoDatasetArray::toString(std::string &str) {
  SDIDevicePropInfoDatasetArray *info = this;
  SDIDevicePropInfoDataset *data;
  strsprintf(str, "SDIDevicePropInfoDataset num: %lld\n", info->num);

  std::map<uint16_t, SDIDevicePropInfoDataset *>::iterator it;
  for (it = dataset.begin(); it != dataset.end(); it++) {
    data = (*it).second;
    data->toString(str);
  }
}

void SDIDevicePropInfoDatasetArray::toString() {
  std::string str;
  toString(str);
  printf("%s", str.c_str());
}

LiveViewImage::LiveViewImage(void *data) {
  uint8_t *_data = (uint8_t *)data;
  org_data = data;
  offset = *(uint32_t *)_data;
  _data += sizeof(uint32_t);
  _size = *(uint32_t *)_data;
  _data += sizeof(uint32_t);
  this->data = (uint8_t *)data + offset;
}

LiveViewImage::~LiveViewImage() {}

uint32_t LiveViewImage::size() { return _size; }

uint8_t *LiveViewImage::get() { return data; }

template <typename T>
SimpleArray<T>::SimpleArray(void *data) {
  char *_data = (char *)data;
  num = *(uint32_t *)_data;
  _data += sizeof(uint32_t);
  values = new T[num];
  for (uint32_t i = 0; i < num; i++) {
    values[i] = *(T *)_data;
    _data += sizeof(T);
  }
}

template <typename T>
SimpleArray<T>::~SimpleArray() {
  delete[] values;
}

template <typename T>
void SimpleArray<T>::toString(std::string &str) {
  strsprintf(str, "  array: num: %u\n", num);
  for (uint32_t i = 0; i < num; i++) {
    strsprintf(str, "    value: %0*llX\n", sizeof(T) * 2, FORMAT(values[i], T));
  }
}

template <typename T>
void SimpleArray<T>::toString() {
  std::string str;
  toString(str);
  printf("%s", str.c_str());
}

template class SimpleArray<int8_t>;
template class SimpleArray<uint8_t>;
template class SimpleArray<int16_t>;
template class SimpleArray<uint16_t>;
template class SimpleArray<int32_t>;
template class SimpleArray<uint32_t>;
template class SimpleArray<int64_t>;
template class SimpleArray<uint64_t>;
