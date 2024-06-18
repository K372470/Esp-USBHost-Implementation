#include "UsbCallbacks.h"
#include <cstdint>
#include <stddef.h>

void UsbCallbacks::onRecievedMessage(const uint8_t *buffer, const uint16_t bufferLength) {}

void UsbCallbacks::onSentMessage(const uint8_t *buffer, const uint16_t bufferLength) {}

UsbCallbacks::UsbCallbacks() {}

UsbCallbacks::~UsbCallbacks() {}
