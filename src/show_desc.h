/*
 * MIT License
 *
 * Copyright (c) 2021 touchgadgetdev@gmail.com
 * (most of this file)
 */

#include <esp_log.h>
#include <usb/usb_types_ch9.h>
typedef union
{
  struct
  {
    uint8_t bLength;                  /**< Size of the descriptor in bytes */
    uint8_t bDescriptorType;          /**< Constant name specifying type of HID descriptor. */
    uint16_t bcdHID;                  /**< USB HID Specification Release Number in Binary-Coded Decimal (i.e., 2.10 is 210H) */
    uint8_t bCountryCode;             /**< Numeric expression identifying country code of the localized hardware. */
    uint8_t bNumDescriptor;           /**< Numeric expression specifying the number of class descriptors. */
    uint8_t bHIDDescriptorType;       /**< Constant name identifying type of class descriptor. See Section 7.1.2: Set_Descriptor Request for a table of class descriptor constants. */
    uint16_t wHIDDescriptorLength;    /**< Numeric expression that is the total size of the Report descriptor. */
    uint8_t bHIDDescriptorTypeOpt;    /**< Optional constant name identifying type of class descriptor. See Section 7.1.2: Set_Descriptor Request for a table of class descriptor constants. */
    uint16_t wHIDDescriptorLengthOpt; /**< Optional numeric expression that is the total size of the Report descriptor. */
  } USB_DESC_ATTR;
  uint8_t val[9];
} usb_hid_desc_t;

void show_dev_desc(const usb_device_desc_t *dev_desc)
{
  ESP_LOGD("", "bLength: %d", dev_desc->bLength);
  ESP_LOGD("", "bDescriptorType(device): %d", dev_desc->bDescriptorType);
  ESP_LOGD("", "bcdUSB: 0x%x", dev_desc->bcdUSB);
  ESP_LOGD("", "bDeviceClass: 0x%02x", dev_desc->bDeviceClass);
  ESP_LOGD("", "bDeviceSubClass: 0x%02x", dev_desc->bDeviceSubClass);
  ESP_LOGD("", "bDeviceProtocol: 0x%02x", dev_desc->bDeviceProtocol);
  ESP_LOGD("", "bMaxPacketSize0: %d", dev_desc->bMaxPacketSize0);
  ESP_LOGD("", "idVendor: 0x%x", dev_desc->idVendor);
  ESP_LOGD("", "idProduct: 0x%x", dev_desc->idProduct);
  ESP_LOGD("", "bcdDevice: 0x%x", dev_desc->bcdDevice);
  ESP_LOGD("", "iManufacturer: %d", dev_desc->iManufacturer);
  ESP_LOGD("", "iProduct: %d", dev_desc->iProduct);
  ESP_LOGD("", "iSerialNumber: %d", dev_desc->iSerialNumber);
  ESP_LOGD("", "bNumConfigurations: %d", dev_desc->bNumConfigurations);
}

void show_config_desc(const void *p)
{
  const usb_config_desc_t *config_desc = (const usb_config_desc_t *)p;

  ESP_LOGD("", "bLength: %d", config_desc->bLength);
  ESP_LOGD("", "bDescriptorType(config): %d", config_desc->bDescriptorType);
  ESP_LOGD("", "wTotalLength: %d", config_desc->wTotalLength);
  ESP_LOGD("", "bNumInterfaces: %d", config_desc->bNumInterfaces);
  ESP_LOGD("", "bConfigurationValue: %d", config_desc->bConfigurationValue);
  ESP_LOGD("", "iConfiguration: %d", config_desc->iConfiguration);
  ESP_LOGD("", "bmAttributes(%s%s%s): 0x%02x",
           (config_desc->bmAttributes & USB_BM_ATTRIBUTES_SELFPOWER) ? "Self Powered" : "",
           (config_desc->bmAttributes & USB_BM_ATTRIBUTES_WAKEUP) ? ", Remote Wakeup" : "",
           (config_desc->bmAttributes & USB_BM_ATTRIBUTES_BATTERY) ? ", Battery Powered" : "",
           config_desc->bmAttributes);
  ESP_LOGD("", "bMaxPower: %d = %d mA", config_desc->bMaxPower, config_desc->bMaxPower * 2);
}

uint8_t show_interface_desc(const void *p)
{
  const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;

  ESP_LOGD("", "bLength: %d", intf->bLength);
  ESP_LOGD("", "bDescriptorType (interface): %d", intf->bDescriptorType);
  ESP_LOGD("", "bInterfaceNumber: %d", intf->bInterfaceNumber);
  ESP_LOGD("", "bAlternateSetting: %d", intf->bAlternateSetting);
  ESP_LOGD("", "bNumEndpoints: %d", intf->bNumEndpoints);
  ESP_LOGD("", "bInterfaceClass: 0x%02x", intf->bInterfaceClass);
  ESP_LOGD("", "bInterfaceSubClass: 0x%02x", intf->bInterfaceSubClass);
  ESP_LOGD("", "bInterfaceProtocol: 0x%02x", intf->bInterfaceProtocol);
  ESP_LOGD("", "iInterface: %d", intf->iInterface);
  return intf->bInterfaceClass;
}

void show_endpoint_desc(const void *p)
{
  const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
  const char *XFER_TYPE_NAMES[] = {
      "Control", "Isochronous", "Bulk", "Interrupt"};
  ESP_LOGD("", "bLength: %d", endpoint->bLength);
  ESP_LOGD("", "bDescriptorType (endpoint): %d", endpoint->bDescriptorType);
  ESP_LOGD("", "bEndpointAddress(%s): 0x%02x",
           (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) ? "In" : "Out",
           endpoint->bEndpointAddress);
  ESP_LOGD("", "bmAttributes(%s): 0x%02x",
           XFER_TYPE_NAMES[endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK],
           endpoint->bmAttributes);
  ESP_LOGD("", "wMaxPacketSize: %d", endpoint->wMaxPacketSize);
  ESP_LOGD("", "bInterval: %d", endpoint->bInterval);
}

void show_hid_desc(const void *p)
{
  usb_hid_desc_t *hid = (usb_hid_desc_t *)p;
  ESP_LOGD("", "bLength: %d", hid->bLength);
  ESP_LOGD("", "bDescriptorType (HID): %d", hid->bDescriptorType);
  ESP_LOGD("", "bcdHID: 0x%04x", hid->bcdHID);
  ESP_LOGD("", "bCountryCode: %d", hid->bCountryCode);
  ESP_LOGD("", "bNumDescriptor: %d", hid->bNumDescriptor);
  ESP_LOGD("", "bDescriptorType: %d", hid->bHIDDescriptorType);
  ESP_LOGD("", "wDescriptorLength: %d", hid->wHIDDescriptorLength);
  if (hid->bNumDescriptor > 1)
  {
    ESP_LOGD("", "bDescriptorTypeOpt: %d", hid->bHIDDescriptorTypeOpt);
    ESP_LOGD("", "wDescriptorLengthOpt: %d", hid->wHIDDescriptorLengthOpt);
  }
}

void show_interface_assoc(const void *p)
{
  usb_iad_desc_t *iad = (usb_iad_desc_t *)p;
  ESP_LOGD("", "bLength: %d", iad->bLength);
  ESP_LOGD("", "bDescriptorType: %d", iad->bDescriptorType);
  ESP_LOGD("", "bFirstInterface: %d", iad->bFirstInterface);
  ESP_LOGD("", "bInterfaceCount: %d", iad->bInterfaceCount);
  ESP_LOGD("", "bFunctionClass: 0x%02x", iad->bFunctionClass);
  ESP_LOGD("", "bFunctionSubClass: 0x%02x", iad->bFunctionSubClass);
  ESP_LOGD("", "bFunctionProtocol: 0x%02x", iad->bFunctionProtocol);
  ESP_LOGD("", "iFunction: %d", iad->iFunction);
}