#pragma once
#include <tuple>
#include <cstdint>
#include <map>
#include "CalifaParser.h"
int febex2preamp(int fbxch);
//typedef  std::tuple<uint8_t, uint8_t, uint8_t> // sfp (0-4), module (0-~18), channel (0-15)

typedef CalifaParser::module_index_t
  califa_febex_idx_t;
//typedef std::tuple<uint8_t, uint8_t, uint8_t> // petal no (0-1), theta-pos, phi-pos
typedef CalifaParser::module_index_t
  crystal_pos_idx_t;

extern std::map<califa_febex_idx_t, crystal_pos_idx_t> idx_map_febex2crystal;
extern std::map<crystal_pos_idx_t, califa_febex_idx_t> idx_map_crystal2febex;
