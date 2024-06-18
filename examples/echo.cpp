#include <esp32-hal-log.h>
#include <Arduino.h>
#include <UsbHost.h>
namespace USB::Examples
{
  // idk where you can use it, but if you can - good luch

  class GamepadController : public UsbCallbacks
  {
    void onSentMessage(const uint8_t *buffer, const uint16_t bufferLength)
    {
      ESP_LOGI("", "Sent back %i bytes:", bufferLength);
      ESP_LOG_BUFFER_HEX("", buffer, bufferLength);
    }

    void onRecievedMessage(const uint8_t *buffer, const uint16_t bufferLength)
    {
      ESP_LOGI("", "Received %i bytes", bufferLength);

      UsbHost::sendMessage(buffer, bufferLength);
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