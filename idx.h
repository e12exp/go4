#pragma once
#include <cstdint>
#include <cassert>
enum subEventIdxType
  {
   fbxChannelIdx=0,
   fbxModuleIdx=1,
   fbxUniqueIdx=2
  };



struct __attribute__((__packed__)) module_index_t
{
  subEventIdxType type: 2;
  uint8_t pc_id: 3;
  uint8_t sfp: 2;
  uint8_t mod;
  uint8_t ch;
  uint8_t zero{};
  operator uint32_t() const
  {
    return *reinterpret_cast<const uint32_t*>(this);
  }
  const char* getPCName() const
  {
    const char* pcNames[]={"m0", "w0", "m1", "w1", "??", "??", "??", "??"};
    return pcNames[pc_id];
  }
  __attribute__((noinline))
  uint32_t toInt() const
  {
    return uint32_t(*this);
  }
  
  
};

static_assert(sizeof(module_index_t)==4);

const uint8_t IDX_WILDCARD = 254;

const module_index_t IDX_ANY{fbxUniqueIdx, 0, 0, 0, IDX_WILDCARD};

const module_index_t IDX_INVALID {fbxUniqueIdx, 0, 0, 0, IDX_WILDCARD};


const module_index_t IDX_NONE  {fbxUniqueIdx, 0, 0, 1, IDX_WILDCARD};
const module_index_t IDX_EVENT {fbxUniqueIdx, 0, 0, 2, IDX_WILDCARD};

//create a petal index from a channel index
module_index_t toIdxType(subEventIdxType st, module_index_t idx);
