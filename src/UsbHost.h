#include <usb/usb_types_ch9.h>
#include <usb/usb_host.h>
#include "UsbCallbacks.h"
#include <freertos/task.h>
#include <esp32-hal-log.h>

#define USB_TAG "USB HOST"
#define HOST_EVENT_TIMEOUT 1
#define CLIENT_EVENT_TIMEOUT 1

class UsbHost // TODO:make class not fully static
{
private:
  static TaskHandle_t *usbTaskHandle;
  typedef void (*usb_host_enum_cb_t)(const usb_config_desc_t *configDesc);
  static usb_host_enum_cb_t _USB_host_enumerate;
  static usb_host_client_handle_t Client_Handle;
  static usb_device_handle_t Device_Handle;
  static uint8_t bInterfaceNumber;
  static void on_usb_transfer_cb(usb_transfer_t *transfer);
  static void _client_event_callback(const usb_host_client_event_msg_t *eventMsg, void *arg);
  static void fix_descriptors_packet_size();
  static void claim_hid_device(const void *p);
  static void prepare_endpoints(const void *p);
  static void show_config_desc_full(const usb_config_desc_t *configDesc);
  static void update();

public:
  static UsbCallbacks *callbacks;
  static bool readyToConnect;
  static bool isConnected;
  static usb_transfer_t *OutTransfer;
  static usb_transfer_t *InTransfer;

  static void freeBuffers();
  static bool sendMessage(uint8_t *buffer, size_t bufferLen);
  static void init();
  static void waitForConnect();
  static void setCallbacks(UsbCallbacks *callbacks);

  static void startLoopTask(void *p);

  UsbHost();
  ~UsbHost();
};
