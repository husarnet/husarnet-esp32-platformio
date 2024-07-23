#pragma once

// User-facing API is defined in the
// "husarnet/port/esp32/user_interface.h" header
// in the included Husarnet submodule.
#include "husarnet/ports/esp32/user_interface.h"

#if defined(ARDUINO) && !defined(ARDUINO_ARCH_ESP32)
  #error "This library only supports boards from the ESP32 family."
#endif
