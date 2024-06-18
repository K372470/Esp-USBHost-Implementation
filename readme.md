# ESP USB Host Library

| Supported Targets | ESP32-S2 | ESP32-S3 |
| ------------------ | -------- | -------- |

This is mainly a wrapper around the [api](https://github.com/espressif/esp-idf/tree/v5.2.2/examples/peripherals/usb/host/usb_host_lib) of esp-idf, writen as a C++ static class.

## How to use

### Basic Usage 

```cpp
class USBCallbacks : public UsbCallbacks
{
  void onRecievedMessage(const uint8_t *buffer, const uint16_t bufferLength) {}
  void onSentMessage(const uint8_t *buffer, const uint16_t bufferLength) {}
};


void setup()
  { 
    // automaticaly initialises all ports
    // creates background usb-handling task
    UsbHost::init();

    // sets callbacks to handle usb requests
    UsbHost::setCallbacks(new USBCallbacks());

    // if needed, stops calling thread until usb device is connected
    UsbHost::waitForConnect();

    // some other setup() stuff
  }

void loop()
{
  // some loop() stuff
}
```

This library contains examples of working code, so you should be good on figuring out its opportunities.
Once it was working BLE and USB at the same time, so it doesnt block calling thread


### Hardware Required

Basically any ESP board that supports USB-OTG.
The Library uses standart ESP-IDF USB api, so it works perfectly on both `arduino` and `esp-idf` platforms

### Connection Pins

| Esp                   | Usb port
| ---                   | --- 
| GND                   | GND 
| Vin or 5v if you have | VBus
| 19                    | D-  
| 20                    | D+  

**WARNING If you dont have enought curent on VBus (~5v) usb-device will not work**

### Configure the project

As the official documentation says:

> The USB Host Stack has a maximum supported transfer size for control transfer during device enumeration. This size is specified via the USB_HOST_CONTROL_TRANSFER_MAX_SIZE configuration option and has a default value of 256 bytes. Therefore, if devices with length config/string descriptors are used, users may want to increase the size of this configuration.

But this is only possible on `esp-idf` platform


### Ending

* If you have some good ideas, feel free to send pull requests
* Good luck in fighting against esp, lol (but its worth it)