#ifndef STRUCT_GOSIP_H_
#define STRUCT_GOSIP_H_

#include <stdint.h>
#ifndef _PACKED
#define _PACKED  //__attribute__ ((packed))
#endif

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

extern "C"
{
typedef struct gosip_sub_header
{
  uint8_t header_size;
  uint8_t trigger :4 _PACKED;
  uint8_t sfp_id  :4 _PACKED;
  uint8_t module_id;
  uint8_t submemory_id;
  uint32_t data_size   _PACKED;
}  gosip_sub_header_t;
}

//C++0x only
static_assert(sizeof(gosip_sub_header_t) == 8, "bad memory alignment forgosip_sub_header_t");


#endif

