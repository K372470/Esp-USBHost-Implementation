#include <Arduino.h>
#include <esp32-hal-log.h>
#include <UsbHost.h>
namespace USB::Examples
{
  class GamepadController : public UsbCallbacks
  {
    void onRecievedMessage(const uint8_t *buffer, const uint16_t bufferLength)
    {
      if (bufferLength == 20)
        ESP_LOGI("GAMEPAD", "LEFT VERTICAL: %i | RIGHT VERTICAL: %i", buffer[12] - 128, buffer[8] - 128); // values from -128 to 127
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