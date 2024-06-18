#include <usb/usb_types_ch9.h>
#include "UsbHost.h"
#include <esp_log.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include <freertos/mpu_wrappers.h>
#include <queue.h>
#include "show_desc.h"
#include <string.h>

uint8_t UsbHost::bInterfaceNumber;

typedef void (*usb_host_enum_cb_t)(const usb_config_desc_t *configDesc);

UsbCallbacks *UsbHost::callbacks = NULL;
TaskHandle_t *UsbHost::usbTaskHandle = NULL;

bool UsbHost::isConnected = false;
bool UsbHost::readyToConnect = false;
uint8_t UsbHost::supportedEndpointType = 0;

usb_transfer_t *UsbHost::OutTransfer = NULL;
usb_transfer_t *UsbHost::InTransfer = NULL;

usb_host_enum_cb_t UsbHost::_USB_host_enumerate = NULL;
usb_host_client_handle_t UsbHost::Client_Handle;
usb_device_handle_t UsbHost::Device_Handle;

void UsbHost::waitForConnect()
{
  ESP_LOGI(USBH_LOG_TAG, "Waiting for usb client to connect");
  while (!isConnected)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(USBH_LOG_TAG, "...");
  }
}

void UsbHost::startLoopTask(void *p)
{
  while (true)
    daemonTask();
}

void UsbHost::setCallbacks(UsbCallbacks *callback)
{
  callbacks = callback;
}

void UsbHost::on_usb_transfer_cb(usb_transfer_t *transfer)
{
  if (Device_Handle == transfer->device_handle)
  {
    int in_buffer = transfer->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK;
    ESP_LOGD(USBH_LOG_TAG, "transfer_cb context: %d", (int)(transfer->context));
    if ((transfer->status == 0))
    {
      if (in_buffer)
      {
        esp_err_t err = usb_host_transfer_submit(transfer);
        if (err != ESP_OK)
        {
          ESP_LOGE(USBH_LOG_TAG, "usb_host_transfer_submit In fail: %x", err);
        }
        if (callbacks != NULL) // success
          callbacks->onRecievedMessage(transfer->data_buffer, transfer->actual_num_bytes);
      }
      else if (callbacks != NULL)
        callbacks->onSentMessage(transfer->data_buffer, transfer->actual_num_bytes);
    }
  }
  else
  {
    ESP_LOGD(USBH_LOG_TAG, "transfer->status %d", transfer->status);
  }
}

void UsbHost::fix_descriptors_maximum_packet_size()
{
  const usb_config_desc_t *config_desc;
  ESP_ERROR_CHECK(usb_host_get_active_config_descriptor(Device_Handle, &config_desc));

  int offset = 0;
  uint16_t wTotalLength = config_desc->wTotalLength;
  const usb_standard_desc_t *next_desc = (const usb_standard_desc_t *)config_desc;
  if (next_desc)
  {
    do
    {
      if (next_desc->bDescriptorType == USB_B_DESCRIPTOR_TYPE_ENDPOINT)
      {
        usb_ep_desc_t *mod_desc = (usb_ep_desc_t *)next_desc;
        if (mod_desc->wMaxPacketSize > MAXIMUM_OUTPUT_TRANFER_BUFFER_LENGTH)
        {
          ESP_LOGW(USBH_LOG_TAG, "Unsupported endpoint 0x%02X with wMaxPacketSize = %d - fixed to %d", mod_desc->bEndpointAddress, mod_desc->wMaxPacketSize, MAXIMUM_INPUT_TRANFER_BUFFER_LENGTH);
          if (mod_desc->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)
            mod_desc->wMaxPacketSize = MAXIMUM_INPUT_TRANFER_BUFFER_LENGTH;
          else
            mod_desc->wMaxPacketSize = MAXIMUM_OUTPUT_TRANFER_BUFFER_LENGTH;
        }
      }

      next_desc = usb_parse_next_descriptor(next_desc, wTotalLength, &offset);

    } while (next_desc != NULL);
  }
}

void UsbHost::claim_device(const void *p)
{
  const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;

  ESP_LOGI(USBH_LOG_TAG, "Claiming a device!");
  fix_descriptors_maximum_packet_size();
  bInterfaceNumber = intf->bInterfaceNumber;
  esp_err_t err = usb_host_interface_claim(Client_Handle, Device_Handle,
                                           bInterfaceNumber, intf->bAlternateSetting);
  if (err != ESP_OK)
    ESP_LOGE(USBH_LOG_TAG, "usb_host_interface_claim failed: %x", err);
  readyToConnect = true;
}

void UsbHost::prepare_endpoint(const void *p)
{
  const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
  if (supportedEndpointType != NULL && (endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != supportedEndpointType)
  {
    ESP_LOGE(USBH_LOG_TAG, "Not supported endpoint: 0x%02x", endpoint->bmAttributes);
    return;
  }
  esp_err_t err;
  if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)
  {
    ESP_LOGI(USBH_LOG_TAG, "In endpoint %d", endpoint->bEndpointAddress);
    err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &InTransfer);
    if (err != ESP_OK)
    {
      InTransfer = NULL;
      ESP_LOGI(USBH_LOG_TAG, "usb_host_transfer_alloc In fail: %x", err);
    }
    InTransfer->device_handle = Device_Handle;
    InTransfer->bEndpointAddress = endpoint->bEndpointAddress;
    InTransfer->callback = on_usb_transfer_cb;
    InTransfer->context = NULL;
    InTransfer->num_bytes = endpoint->wMaxPacketSize;
    esp_err_t err = usb_host_transfer_submit(InTransfer);
    if (err != ESP_OK)
    {
      ESP_LOGE(USBH_LOG_TAG, "usb_host_transfer_submit In fail: %x", err);
    }
  }
  else
  {
    err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &OutTransfer);
    if (err != ESP_OK)
    {
      OutTransfer = NULL;
      ESP_LOGE(USBH_LOG_TAG, "usb_host_transfer_alloc Out fail: %x", err);
      return;
    }
    ESP_LOGI(USBH_LOG_TAG, "Out data_buffer_size: %d", OutTransfer->data_buffer_size);
    ESP_LOGI(USBH_LOG_TAG, "Out endpoint %d", endpoint->bEndpointAddress);

    OutTransfer->device_handle = Device_Handle;
    OutTransfer->bEndpointAddress = endpoint->bEndpointAddress;
    OutTransfer->callback = on_usb_transfer_cb;
    OutTransfer->context = NULL;
  }
  isConnected = ((OutTransfer != NULL) || (InTransfer != NULL)); // OR, bcs HID devices can have only that
}

void UsbHost::show_config_desc_full(const usb_config_desc_t *config_desc)
{
  // Full decode of config desc.
  const uint8_t *p = &config_desc->val[0];
  uint8_t bLength;
  for (int i = 0; i < config_desc->wTotalLength; i += bLength, p += bLength)
  {
    bLength = *p;
    if ((i + bLength) <= config_desc->wTotalLength)
    {
      const uint8_t bDescriptorType = *(p + 1);
      switch (bDescriptorType)
      {
      case USB_B_DESCRIPTOR_TYPE_DEVICE:
        ESP_LOGE(USBH_LOG_TAG, "USB Device Descriptor should not appear in config");
        break;
      case USB_B_DESCRIPTOR_TYPE_CONFIGURATION:
        show_config_desc(p);
        break;
      case USB_B_DESCRIPTOR_TYPE_STRING:
        ESP_LOGE(USBH_LOG_TAG, "USB string desc TBD");
        break;
      case USB_B_DESCRIPTOR_TYPE_INTERFACE:
        show_interface_desc(p);
        if (!readyToConnect)
          claim_device(p);
        break;

        break;
      case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
        show_endpoint_desc(p);
        if (readyToConnect && !isConnected)
          prepare_endpoint(p);
        break;
      }
    }
    else
    {
      ESP_LOGE(USBH_LOG_TAG, "USB Descriptor invalid");
      return;
    }
  }
}

void UsbHost::client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg)
{
  esp_err_t err;
  switch (event_msg->event)
  {
  // A new device has been enumerated and added to the USB Host Library
  case USB_HOST_CLIENT_EVENT_NEW_DEV:
    ESP_LOGI(USBH_LOG_TAG, "New device address: %d", event_msg->new_dev.address);
    err = usb_host_device_open(Client_Handle, event_msg->new_dev.address, &Device_Handle);
    if (err != ESP_OK)
      ESP_LOGE(USBH_LOG_TAG, "usb_host_device_open: %x", err);

    usb_device_info_t dev_info;
    err = usb_host_device_info(Device_Handle, &dev_info);
    if (err != ESP_OK)
      ESP_LOGE(USBH_LOG_TAG, "usb_host_device_info: %x", err);
    ESP_LOGD(USBH_LOG_TAG, "speed: %d dev_addr %d vMaxPacketSize0 %d bConfigurationValue %d",
             dev_info.speed, dev_info.dev_addr, dev_info.bMaxPacketSize0,
             dev_info.bConfigurationValue);

    const usb_device_desc_t *dev_desc;
    err = usb_host_get_device_descriptor(Device_Handle, &dev_desc);
    if (err != ESP_OK)
      ESP_LOGE(USBH_LOG_TAG, "usb_host_get_device_desc: %x", err);
    show_dev_desc(dev_desc);

    const usb_config_desc_t *config_desc;
    err = usb_host_get_active_config_descriptor(Device_Handle, &config_desc);
    if (err != ESP_OK)
      ESP_LOGE(USBH_LOG_TAG, "usb_host_get_config_desc: %x", err);
    (*_USB_host_enumerate)(config_desc);
    break;
  // A device opened by the client is now gone
  case USB_HOST_CLIENT_EVENT_DEV_GONE:
    ESP_LOGI(USBH_LOG_TAG, "Device Gone handle");
    isConnected = false;
    readyToConnect = false;
    freeBuffers();
    break;
  }
}

void UsbHost::init()
{
  const usb_host_config_t config = {
      .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };
  esp_err_t err = usb_host_install(&config);
  ESP_LOGI(USBH_LOG_TAG, "usb_host_install err: %x", err);

  const usb_host_client_config_t client_config = {
      .is_synchronous = false,
      .max_num_event_msg = 5,
      .async = {.client_event_callback = client_event_callback,
                .callback_arg = Client_Handle}};
  err = usb_host_client_register(&client_config, &Client_Handle);
  ESP_LOGI(USBH_LOG_TAG, "usb_host_client_register err: %x", err);

  _USB_host_enumerate = show_config_desc_full;

  xTaskCreatePinnedToCore(&startLoopTask, "usb_task", 8 * 1024, NULL, 1, usbTaskHandle, 1);
}

void UsbHost::daemonTask(void)
{
  uint32_t event_flags;
  static bool all_clients_gone = false;
  static bool all_dev_free = false;

  esp_err_t err = usb_host_lib_handle_events(HOST_EVENT_TIMEOUT, &event_flags);
  if (err == ESP_OK)
  {
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
    {
      ESP_LOGI(USBH_LOG_TAG, "No more clients");
      all_clients_gone = true;
    }
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
    {
      ESP_LOGI(USBH_LOG_TAG, "No more devices");
      all_dev_free = true;
    }
  }
  else
  {
    if (err != ESP_ERR_TIMEOUT)
    {
      ESP_LOGI(USBH_LOG_TAG, "usb_host_lib_handle_events");
    }
  }

  err = usb_host_client_handle_events(Client_Handle, CLIENT_EVENT_TIMEOUT);
  if ((err != ESP_OK) && (err != ESP_ERR_TIMEOUT))
  {
    ESP_LOGE(USBH_LOG_TAG, "usb_host_client_handle_events: %x", err);
  }
}

void UsbHost::setSupportedEndpointType(uint8_t type) { supportedEndpointType = type; }
bool UsbHost::isDeviceConnected() { return isConnected; }

esp_err_t UsbHost::sendMessage(const uint8_t *buffer, const uint16_t bufferLen)
{
  if (bufferLen > MAXIMUM_OUTPUT_TRANFER_BUFFER_LENGTH)
    return ESP_ERR_INVALID_SIZE;
  if (OutTransfer == NULL)
    return ESP_ERR_NOT_SUPPORTED;

  memcpy(OutTransfer->data_buffer, buffer, bufferLen);
  OutTransfer->num_bytes = bufferLen;

  esp_err_t err = usb_host_transfer_submit(OutTransfer);

  if (err != ESP_OK)
    ESP_LOGE(USBH_LOG_TAG, "Error in usb transporting occured");

  vTaskDelay(5 / portTICK_PERIOD_MS);
  return err;
}

UsbHost::UsbHost()
{
}

UsbHost::~UsbHost()
{
  freeBuffers();
}

void UsbHost::freeBuffers()
{
  usb_host_transfer_free(OutTransfer);
  usb_host_transfer_free(InTransfer);

  usb_host_interface_release(Client_Handle, Device_Handle, bInterfaceNumber);
  usb_host_device_close(Client_Handle, Device_Handle);
  vTaskDelay(100 / portTICK_PERIOD_MS);
}
