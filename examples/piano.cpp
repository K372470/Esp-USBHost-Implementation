#include <Arduino.h>
#include <esp32-hal-log.h>
#include <UsbHost.h>

namespace USB::Examples
{
  const int MIDI_MESSAGE_LENGHT = 4;

  enum Commands
  {
    SwitchPreset,
    ChangeVolume
  };

  /// @brief Example class of some usb piano with midi keys(BULK) and sound sender(INTERRUPT)
  class PianoController : public UsbCallbacks
  {
    void onRecievedMessage(const uint8_t *buffer, const uint16_t bufferLength)
    {
      if (bufferLength == MIDI_MESSAGE_LENGHT && buffer[0] == ChangeVolume)
        ESP_LOGI("", "Volume changed to: %i", buffer[1]);
    }
    void onSentMessage(const uint8_t *buffer, const uint16_t bufferLength)
    {
      if (bufferLength == MIDI_MESSAGE_LENGHT && buffer[0] == ChangeVolume)
        ESP_LOGI("", "Sending Change volume command. New value: %i", buffer[1]);
    }
  };

  void setup()
  {
    UsbHost::init();
    UsbHost::setCallbacks(new PianoController());
    UsbHost::setSupportedEndpointType(USB_BM_ATTRIBUTES_XFER_BULK);
    UsbHost::waitForConnect();
  }

  void loop()
  {
    vTaskDelete(NULL);
  }
}