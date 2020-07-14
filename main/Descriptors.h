#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

// Includes
#include "LUFAConfig.h"

#include <LUFA/LUFA/Drivers/USB/USB.h>
#include <avr/pgmspace.h>

// Type Defines
// Device Configuration Descriptor Structure
typedef struct {
  USB_Descriptor_Configuration_Header_t Config;

  // Joystick HID Interface
  USB_Descriptor_Interface_t HID_Interface;
  USB_HID_Descriptor_HID_t HID_JoystickHID;
  USB_Descriptor_Endpoint_t HID_ReportOUTEndpoint;
  USB_Descriptor_Endpoint_t HID_ReportINEndpoint;

  // CDC Control Interface
  USB_Descriptor_Interface_Association_t CDC_IAD;
  USB_Descriptor_Interface_t CDC_CCI_Interface;
  USB_CDC_Descriptor_FunctionalHeader_t CDC_Functional_Header;
  USB_CDC_Descriptor_FunctionalACM_t CDC_Functional_ACM;
  USB_CDC_Descriptor_FunctionalUnion_t CDC_Functional_Union;
  USB_Descriptor_Endpoint_t CDC_NotificationEndpoint;

  // CDC Data Interface
  USB_Descriptor_Interface_t CDC_DCI_Interface;
  USB_Descriptor_Endpoint_t CDC_DataOutEndpoint;
  USB_Descriptor_Endpoint_t CDC_DataInEndpoint;

} USB_Descriptor_Configuration_t;

// Device Interface Descriptor IDs
enum InterfaceDescriptors_t {
  INTERFACE_ID_Joystick = 0, /**< Joystick interface descriptor ID */
  INTERFACE_ID_CDC_CCI = 1,  /**< CDC CCI interface descriptor ID */
  INTERFACE_ID_CDC_DCI = 2,  /**< CDC DCI interface descriptor ID */
};

// Device String Descriptor IDs
enum StringDescriptors_t {
  STRING_ID_Language =
      0,  // Supported Languages string descriptor ID (must be zero)
  STRING_ID_Manufacturer = 1,  // Manufacturer string ID
  STRING_ID_Product = 2,       // Product string ID
};

// Macros
// Endpoint Addresses
#define JOYSTICK_IN_EPADDR (ENDPOINT_DIR_IN | 1)
#define JOYSTICK_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)
// HID Endpoint Size
// The Switch -needs- this to be 64.
// The Wii U is flexible, allowing us to use the default of 8 (which did not
// match the original Hori descriptors).
#define JOYSTICK_EPSIZE 64
// Descriptor Header Type - HID Class HID Descriptor
#define DTYPE_HID 0x21
// Descriptor Header Type - HID Class HID Report Descriptor
#define DTYPE_Report 0x22
/** Endpoint address of the CDC device-to-host notification IN endpoint. */
#define CDC_NOTIFICATION_EPADDR (ENDPOINT_DIR_IN | 3)
/** Endpoint address of the CDC device-to-host data IN endpoint. */
#define CDC_TX_EPADDR (ENDPOINT_DIR_IN | 4)
/** Endpoint address of the CDC host-to-device data OUT endpoint. */
#define CDC_RX_EPADDR (ENDPOINT_DIR_OUT | 5)
/** Size in bytes of the CDC device-to-host notification IN endpoint. */
#define CDC_NOTIFICATION_EPSIZE 8
/** Size in bytes of the CDC data IN and OUT endpoints. */
#define CDC_TXRX_EPSIZE 64

/* Function Prototypes: */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void **const DescriptorAddress)
    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
