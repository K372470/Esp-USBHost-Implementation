#include <esp32-hal-log.h>
#include <UsbHost.h>
namespace USB::Examples
{
  class GamepadController : public UsbCallbacks
  {
    void onRecievedMessage(const uint8_t *buffer, const uint16_t bufferLength)
    {
      ESP_LOG_BUFFER_HEX(USBH_LOG_TAG, buffer, bufferLength);
    }
  };

  void setup()
  {
    UsbHost::init();
    UsbHost::setCallbacks(new GamepadController());
    UsbHost::waitForConnect();
  }

  void loop()
  {
    vTaskDelete(NULL);
  }
}