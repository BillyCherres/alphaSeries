/**
 * @file parser.h
 * @brief Header file of the parser to parse some PTP data.
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdint.h>

#include <map>
#include <string>

#define PTP_MAXSTRLEN 255

namespace com {
namespace sony {
namespace imaging {
namespace remote {

/**
 * @brief This is the common class of SDIDevicePropInfoDataset.
 */
class SDIDevicePropInfoDataset {
 private:
  size_t mSize;

 public:
  uint16_t DevicePropertyCode;  //!< A specific DevicePropCode.
  uint16_t
      DataType;    //!< This field identifies the Datatype Code of the property.
  uint8_t GetSet;  //!< This field indicates whether the property is read-only
                   //!< or read-write.
  uint8_t IsEnable;  //!< This field indicates whether the property is valid,
                     //!< invalid or DispOnly.
  uint8_t FormFlag;  //!< This field indicates the format of the next field.

  /**
   * @brief create a suitable DataType class for the DataType
   * @param data an address of a DevicePropCode dataset.
   * @see DataTypeInteger
   * @see DataTypeArray
   * @see DataTypeSTR
   * @return the parsed result
   */
  static SDIDevicePropInfoDataset *create(void *data);

  /**
   * @brief return the total bytes
   * @return the total bytes
   */
  size_t size() { return mSize; };

  /**
   * @brief this function is called from create(). don't call directly.
   *
   * sets the properties of DevicePropertyCode and etc from \em data.
   * @param data an address of a DevicePropCode dataset.
   * @see create()
   * @return the parsed result
   */
  SDIDevicePropInfoDataset *parse(void *data);

  virtual ~SDIDevicePropInfoDataset();

  /**
   * @brief print the parsed contents to stdio
   */
  virtual void toString();

  /**
   * @brief stores the parsed contents to std::string
   * @param str a reference of std::string to store it
   */
  virtual void toString(std::string &str);

 protected:
  /**
   * @brief parse a value section
   * @params data a double pointer of an address of start of a value section
   */
  virtual void parseValues(char **data);

  /**
   * @brief parse a form section
   * @params data a double pointer of an address of start of a form section
   */
  virtual void parseForm(char **data);
  SDIDevicePropInfoDataset()
      : mSize(),
        DevicePropertyCode(),
        DataType(),
        GetSet(),
        IsEnable(),
        FormFlag() {}
};

/**
 * @brief This class supports an integer type dataset.
 */
template <typename T>
class DataTypeInteger : public SDIDevicePropInfoDataset {
  friend class SDIDevicePropInfoDataset;

 public:
  T DefaultValue;  //!< This field identifies the value of the factory default
                   //!< setting for the property.
  T CurrentValue;  //!< This field identifies the current value of the property.
  T MinimumValue;  //!< Minimum value of property supported by the device.\n If
                   //!< the FormFlag is Range-Form, this is valid.
  T MaximumValue;  //!< Maximum value of property supported by the device.\n If
                   //!< the FormFlag is Range-Form, this is valid.
  T StepSize;      //!< The steps of the supported value like MinimumValue + N x
               //!< StepSize which is less than or equal to MaximumValue.\n If
               //!< the FormFlag is Range-Form, this is valid.
  uint16_t NumOfValues;  //!< This field indicates the number of values of the
                         //!< particular property supported by the device.\n If
                         //!< the FormFlag is Enumeration-Form, this is valid.

  uint16_t NumOfValues_2nd;

  T *Values;  //!< This field is the array of the supported value.\n If the
              //!< FormFlag is Enumeration-Form, this is valid.

  virtual void toString();
  virtual void toString(std::string &str);
  virtual ~DataTypeInteger();

 protected:
  virtual void parseValues(char **data);
  virtual void parseForm(char **data);
  DataTypeInteger()
      : DefaultValue(),
        CurrentValue(),
        MinimumValue(),
        MaximumValue(),
        StepSize(),
        NumOfValues(),
        NumOfValues_2nd(),
        Values() {}
};

/**
 * @brief This class supports an array of an integer type dataset.
 */
template <typename T>
class DataTypeArray : public SDIDevicePropInfoDataset {
  friend class SDIDevicePropInfoDataset;

 public:
  uint32_t
      NumOfDefaultValues;  //!< Represents the total number of elements of the
                           //!< factory default setting contained in the array.
  uint32_t NumOfCurrentValues;  //!< Represents the total number of elements of
                                //!< the current value contained in the array.
  T *DefaultValues;  //!< This field is the array of the factory default setting
                     //!< value.
  T *CurrentValues;  //!< This field is the array of the current value.

  virtual void toString();
  virtual void toString(std::string &str);
  virtual ~DataTypeArray();

 protected:
  virtual void parseValues(char **data);

  /**
   * @brief In this parser the form section is not supported.\n If the FormFlag
   * is not None, the parsed result of the all following datasets is corrupt.
   */
  virtual void parseForm(char **data);
  DataTypeArray()
      : NumOfDefaultValues(),
        NumOfCurrentValues(),
        DefaultValues(),
        CurrentValues() {}
};

/**
 * @brief This class supports a Unicode String type dataset.
 */
class DataTypeSTR : public SDIDevicePropInfoDataset {
  friend class SDIDevicePropInfoDataset;

 public:
  char *DefaultValue;  //!< This field identifies the value of the factory
                       //!< default setting for the property.
  char *CurrentValue;  //!< This field identifies the current value of the
                       //!< property.
  uint8_t length_DefaultValue;  //!< the number of characters of DevalueValue
  uint8_t length_CurrentValue;  //!< the number of characters of CurrentValue

  virtual void toString();
  virtual void toString(std::string &str);
  virtual ~DataTypeSTR();

 protected:
  virtual void parseValues(char **data);

  /**
   * @brief In this parser the form section is not supported.\n If the FormFlag
   * is not None, the parsed result of the all following datasets is corrupt.
   */
  virtual void parseForm(char **data);
  DataTypeSTR()
      : DefaultValue(),
        CurrentValue(),
        length_DefaultValue(),
        length_CurrentValue() {}
};

/**
 * @brief This class supports a SDIDevicePropInfo Dataset array.
 */
class SDIDevicePropInfoDatasetArray {
 private:
  std::map<uint16_t, SDIDevicePropInfoDataset *> dataset;

 public:
  uint64_t num;  //<!

  /**
   * @brief parses the data gotten by GetAllExtDevicePropInfo API.
   * @param data an address of the data.
   * @return the parsed result
   */
  SDIDevicePropInfoDatasetArray(void *data);

  /**
   * @brief gets the specified SDIDevicePropInfoDataset as \em
   * device_property_code.
   * @param device_property_code a device property code.
   * @return the instance of SDIDevicePropInfoDataset.\n It should be casted to
   * a suitable DataType in accordance with the member variable of DataType.
   * @see DataTypeInteger
   * @see DataTypeArray
   * @see DataTypeSTR
   */
  SDIDevicePropInfoDataset *get(uint16_t device_property_code);

  ~SDIDevicePropInfoDatasetArray();

  /**
   * @brief print the parsed contents to stdio
   */
  void toString();

  /**
   * @brief stores the parsed contents to std::string
   * @param str a reference of std::string to store it
   */
  void toString(std::string &str);
};

/**
 * @brief LiveView parser
 */
class LiveViewImage {
 private:
  void *org_data;
  uint32_t offset;
  uint32_t _size;
  uint8_t *data;

 public:
  /**
   * @brief parses the LiveView dataset.
   * @param data an address of the LiveView dataset.
   */
  LiveViewImage(void *data);
  ~LiveViewImage();

  /**
   * @brief returns the size of the LiveView image.
   */
  uint32_t size();

  /**
   * @brief returns the pointer of the LiveView image to be able to be decoded
   * as JPEG.
   */
  uint8_t *get();
};

/**
 * @brief This class supports a simple array format.
 */
template <typename T>
class SimpleArray {
 public:
  uint32_t
      num;  //<! Represents the total number of elements contained in the array.
  T *values;  //<! This field is the array entry.

  /**
   * @brief parses a simple array format.
   * @param data an address of a simple array format.
   */
  SimpleArray(void *data);
  ~SimpleArray();

  /**
   * @brief print the parsed contents to stdio
   */
  void toString();

  /**
   * @brief stores the parsed contents to std::string
   * @param str a reference of std::string to store it
   */
  void toString(std::string &str);
};

}  // namespace remote
}  // namespace imaging
}  // namespace sony
}  // namespace com
#endif /* __PARSER_H__ */
