#include <Arduino.h>
#include <UsbHost.h>

UsbHost USBH;

class UsbController : public UsbCallbacks
{
  void onSentMessage(const uint8_t *buffer, const size_t bufferLength) override {}

  void onRecievedMessage(const uint8_t *buffer, const size_t bufferLength) override
  {
    ESP_LOGW(USB_TAG, "LEFT: %i RIGHT: %i", buffer[8] - 128, buffer[12] - 128);
  }
};

void setup()
{
  USBH.init();
  USBH.setCallbacks(new UsbController());
  USBH.waitForConnect();
}

void loop()
{
}
