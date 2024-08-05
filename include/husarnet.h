#pragma once

#if defined(ARDUINO)
  #if !defined(ARDUINO_ARCH_ESP32)
    #error "This library only supports boards from the ESP32 family."
  #elif ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 3)
    #error "This library requires ESP32 Arduino Core version 3.0.3 or newer. Please upgrade your board/platform."
  #endif
#endif

// User-facing API is defined in the
// "husarnet/port/esp32/user_interface.h" header
// in the included Husarnet submodule.
#include "husarnet/ports/esp32/user_interface.h"
