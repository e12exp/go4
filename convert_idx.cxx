#include <map>
#include "convert_idx.h"
#include <cassert>
#include <stdio.h>
//converts febex channels (0..15) to mesytec channels (1..16)
int febex2preamp(int fbxch)
{
  if (fbxch>7)
    return 8+febex2preamp(fbxch-8);
  if (fbxch<1 || fbxch==7)
    return fbxch+1;
  return 8-fbxch;
}

static crystal_pos_idx_t febex2crystal(califa_febex_idx_t i)
{
  // febex channel maps
  static std::map<uint8_t, uint8_t> phimap={
    {3, 0}, {7, 0}, {13, 0}, { 9, 0},
    {4, 1}, {0, 1}, {14, 1}, {10, 1},
    {5, 2}, {1, 2}, {15, 2}, {11, 2},
    {6, 3}, {2, 3}, { 8, 3}, {12, 3}
  };
  static std::map<uint8_t, uint8_t> thetamap={
    { 3, 0}, { 4, 0}, { 5, 0}, { 6, 0},
    { 7, 1}, { 0, 1}, { 1, 1}, { 2, 1},
    {13, 2}, {14, 2}, {15, 2}, { 8, 2},
    { 9, 3}, {10, 3}, {11, 3}, {12, 3}
  };
  uint8_t petal_idx=0;
  uint8_t theta_idx=0;
  uint8_t phi_idx  =0;
  
  uint8_t fbx_sfp=GET_SFP(i);
  assert(fbx_sfp==0);
  uint8_t fbx_mod=GET_MOD(i);
  petal_idx+=fbx_mod/4;
  theta_idx+=4*(fbx_mod%4);
  uint8_t fbx_ch =GET_CH(i);
  theta_idx+=thetamap[fbx_ch];
  phi_idx+=phimap[fbx_ch];

  return std::make_tuple(CalifaParser::subEventIdxType::petalIdx, petal_idx, phi_idx, theta_idx);
}

std::map<califa_febex_idx_t, crystal_pos_idx_t> init_febex2crystal()
{
  std::map<califa_febex_idx_t, crystal_pos_idx_t> r={};
  for (uint8_t sfp=0; sfp<1; sfp++)
    for (uint8_t mod=0; mod<8; mod++)
      for (uint8_t ch=0; ch<16; ch++)
	{
	  auto fbx_idx=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx, sfp, mod, ch);
	  r[fbx_idx]=febex2crystal(fbx_idx);
	  /*	  printf("fbx %d %d %d - > crystal %d %d %d",
		 GET_SFP(fbx_idx), GET_MOD(fbx_idx), GET_CH(fbx_idx),
		 GET_SFP(r[fbx_idx]), GET_MOD(r[fbx_idx]), GET_CH(r[fbx_idx])
		 );*/
	}
  return r;
}

std::map<califa_febex_idx_t, crystal_pos_idx_t> idx_map_febex2crystal=init_febex2crystal();

std::map<crystal_pos_idx_t, califa_febex_idx_t> init_crystal2febex()
{
  std::map<crystal_pos_idx_t, califa_febex_idx_t> r={};
  for (auto it: idx_map_febex2crystal)
    {
      r[it.second]=it.first;
    }
  return r;
}
std::map<crystal_pos_idx_t, califa_febex_idx_t> idx_map_crystal2febex=init_crystal2febex();

