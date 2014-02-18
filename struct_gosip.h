#ifndef STRUCT_GOSIP_H_
#define STRUCT_GOSIP_H_

#include <stdint.h>

typedef struct gosip_header
{
  uint8_t header_size;
  uint8_t trigger;
  uint8_t module_id;
  uint8_t submemory_id;
  uint32_t data_size;
  uint32_t timestamp_1;
  uint16_t timestamp_2;
  uint8_t timestamp_3;
  uint8_t num_submemories;
}
  gosip_header_t;

typedef struct gosip_sub_header
{
  uint8_t header_size;
  uint8_t trigger;
  uint8_t module_id;
  uint8_t submemory_id;
  uint32_t data_size;
}
  gosip_sub_header_t;

#endif

