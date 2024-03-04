#include <stdint.h>

uint32_t static sun_8x8[] = {
  0xFFFF00, 0x000000, 0x000000, 0xFFFF00, 0x000000, 0x000000, 0x000000, 0xFFFF00, 
  0x000000, 0xFFFF00, 0x000000, 0x000000, 0x000000, 0x000000, 0xFFFF00, 0x000000, 
  0x000000, 0x000000, 0x000000, 0xFFFF00, 0xFFFF00, 0x000000, 0x000000, 0x000000, 
  0x000000, 0x000000, 0xFFFF00, 0xFFFF00, 0xFFFF00, 0xFFFF00, 0x000000, 0xFFFF00, 
  0xFFFF00, 0x000000, 0xFFFF00, 0xFFFF00, 0xFFFF00, 0xFFFF00, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000000, 0xFFFF00, 0xFFFF00, 0x000000, 0x000000, 0x000000, 
  0x000000, 0xFFFF00, 0x000000, 0x000000, 0x000000, 0x000000, 0xFFFF00, 0x000000, 
  0xFFFF00, 0x000000, 0x000000, 0x000000, 0xFFFF00, 0x000000, 0x000000, 0xFFFF00,
};

uint32_t static cloud_8x8[] = {
  0x000000, 0x00FFFF, 0x00FFFF, 0x00FFFF, 0x000000, 0x000000, 0x000000, 0x000000, 
  0x00FFFF, 0x000000, 0x000000, 0x000000, 0x00FFFF, 0x00FFFF, 0x00FFFF, 0x000000, 
  0x00FFFF, 0x000000, 0x000000, 0x000000, 0x00FFFF, 0x000000, 0x000000, 0x00FFFF, 
  0x00FFFF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x00FFFF, 
  0x00FFFF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x00FFFF, 
  0x000000, 0x00FFFF, 0x00FFFF, 0x00FFFF, 0x00FFFF, 0x00FFFF, 0x00FFFF, 0x000000, 
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 
};

uint32_t static showers_8x8[] = {
  0x000000, 0x8000FF, 0x8000FF, 0x8000FF, 0x000000, 0x000000, 0x000000, 0x000000, 
  0x8000FF, 0x000000, 0x000000, 0x000000, 0x8000FF, 0x8000FF, 0x8000FF, 0x000000, 
  0x8000FF, 0x000000, 0x000000, 0x000000, 0x8000FF, 0x000000, 0x000000, 0x8000FF, 
  0x8000FF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x8000FF, 
  0x8000FF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x8000FF, 
  0x000000, 0x8000FF, 0x8000FF, 0x8000FF, 0x8000FF, 0x8000FF, 0x8000FF, 0x000000, 
  0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 
};

uint32_t static rain_8x8[] = {
  0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 
  0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 
  0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 
  0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 
  0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 
  0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 
  0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 
  0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000, 0x0000FF, 0x000000
};

uint32_t static storm_8x8[] = {
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x0000FF, 0x0000FF, 0x000000, 
  0x000000, 0x000000, 0x000000, 0x000000, 0x0000FF, 0x0000FF, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000000, 0x0000FF, 0x0000FF, 0x000000, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000000, 0x000000, 0x0000FF, 0x0000FF, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000000, 0x0000FF, 0x0000FF, 0x000000, 0x000000, 0x000000, 
  0x0000FF, 0x000000, 0x0000FF, 0x0000FF, 0x000000, 0x000000, 0x000000, 0x000000, 
  0x0000FF, 0x0000FF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 
  0x0000FF, 0x0000FF, 0x0000FF, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 
};

uint32_t static snow_8x8[] = {
  0x8080FF, 0x000000, 0x8080FF, 0x000000, 0x000000, 0x8080FF, 0x000000, 0x8080FF, 
  0x000000, 0x8080FF, 0x000000, 0x000000, 0x000000, 0x000000, 0x8080FF, 0x000000, 
  0x8080FF, 0x000000, 0x8080FF, 0x000000, 0x000000, 0x8080FF, 0x000000, 0x8080FF, 
  0x000000, 0x000000, 0x000000, 0x8080FF, 0x8080FF, 0x000000, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000000, 0x8080FF, 0x8080FF, 0x000000, 0x000000, 0x000000, 
  0x8080FF, 0x000000, 0x8080FF, 0x000000, 0x000000, 0x8080FF, 0x000000, 0x8080FF, 
  0x000000, 0x8080FF, 0x000000, 0x000000, 0x000000, 0x000000, 0x8080FF, 0x000000, 
  0x8080FF, 0x000000, 0x8080FF, 0x000000, 0x000000, 0x8080FF, 0x000000, 0x8080FF, 
};