#include <usb/usb_types_ch9.h>
#include <usb/usb_host.h>
#include "UsbCallbacks.h"
#include <freertos/task.h>

#define USBH_LOG_TAG "USBH"

#ifndef USB_HOST_CONTROL_TRANSFER_MAX_SIZE
#define USB_HOST_CONTROL_TRANSFER_MAX_SIZE 256 // on esp-idf platform can setup this in sdkconfig
#endif

class UsbHost // FIXME: all names are different style-guide. If somebody can help, please, do.
{
private:
  const static uint16_t MAXIMUM_INPUT_TRANFER_BUFFER_LENGTH = USB_HOST_CONTROL_TRANSFER_MAX_SIZE;
  const static uint16_t MAXIMUM_OUTPUT_TRANFER_BUFFER_LENGTH = 64; // TODO:Try if this can equal to USB_HOST_CONTROL_TRANSFER_MAX_SIZE in arduino
  const static uint8_t HOST_EVENT_TIMEOUT = 1;
  const static uint8_t CLIENT_EVENT_TIMEOUT = 1;

  typedef void (*usb_host_enum_cb_t)(const usb_config_desc_t *configDesc);
  static usb_host_enum_cb_t _USB_host_enumerate;
  static usb_host_client_handle_t Client_Handle;
  static usb_device_handle_t Device_Handle;

  static usb_transfer_t *OutTransfer;
  static usb_transfer_t *InTransfer;

  static uint8_t bInterfaceNumber;
  static UsbCallbacks *callbacks;

  static bool readyToConnect;
  static bool isConnected;

  static uint8_t supportedEndpointType;

  static TaskHandle_t *usbTaskHandle;

  static void on_usb_transfer_cb(usb_transfer_t *transfer);
  static void client_event_callback(const usb_host_client_event_msg_t *eventMsg, void *arg);
  static void fix_descriptors_maximum_packet_size();
  static void claim_device(const void *p);
  static void prepare_endpoint(const void *p);
  static void show_config_desc_full(const usb_config_desc_t *configDesc);
  static void daemonTask();
  static void startLoopTask(void *p);

public:
  /// @brief
  /// @return state of usb service
  static bool isDeviceConnected();

  /// @brief Disconnect and Dispose memory used for transports.
  static void freeBuffers();

  /// @brief
  /// @return ESP_OK on success, overwize error
  static esp_err_t sendMessage(const uint8_t *buffer, const uint16_t bufferLen);

  /// @brief Initialises subrelaing functions. Should be called before using
  static void init();

  /// @brief Non-async function, which blocks calling thread before device is connected and ready to use
  static void waitForConnect();

  /// @brief Use this, if needed to get only needed endpoints (ex: to remove INTERRUPT, which spam big sound packets on some sound-cards)
  /// @param type Only endpoint with this type will be used (ex: USB_BM_ATTRIBUTES_XFER_BULK)
  static void setSupportedEndpointType(uint8_t type = USB_BM_ATTRIBUTES_XFER_BULK);

  /// @brief Set callbacks to call on receiving message
  static void
  setCallbacks(UsbCallbacks *callbacks);

  UsbHost();
  ~UsbHost();
};
