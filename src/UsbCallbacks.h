#include <cstdint>
#include <stddef.h>

class UsbCallbacks
{
public:
  virtual void onRecievedMessage(const uint8_t *buffer, const size_t bufferLength);
  virtual void onSentMessage(const uint8_t *buffer, const size_t bufferLength);
  UsbCallbacks();
  ~UsbCallbacks();
};
