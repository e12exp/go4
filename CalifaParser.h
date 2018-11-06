#ifndef __CalifaParser_h
#define __CalifaParser_h
#include "struct_event.h"
#include "struct_gosip.h"
#include "struct_ts.h"
//#include <vector>
#include <stdint.h>
#include "struct_eventinfo.h"
#include <map>
#include <tuple>
#include <set>
#include <assert.h>

#if 1
#include "TGo4MbsEvent.h"
#endif

#define IDX_ANY  CalifaParser::module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, 255, 255, 255)
#define IDX_NONE CalifaParser::module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, 255, 255, 254)
#define IDX_CHANNEL_WILDCARD 254
#define IDX_INVALID CalifaParser::module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, 255, 255, 253)

#define GET_TYPE(idx) std::get<0>(idx)
#define GET_SFP(idx)  std::get<1>(idx)
#define GET_MOD(idx)  std::get<2>(idx)
#define GET_CH(idx)   std::get<3>(idx) 
#define IDX_WILDCARD ((uint8_t)254)

#define GET_PETAL(idx) std::get<1>(idx)

class CalifaParser
{
 public:
  static const uint32_t FEBEX_PROC_ID=0x2;
  static const uint32_t FEBEX_EVT_TYPE=0x64;
  static const uint32_t FEBEX_SUBEVT_TYPE=0x2710;

  enum subEventIdxType
  {
    fbxChannelIdx=0,
    fbxModuleIdx=1,
    petalIdx=2
  };



  typedef std::tuple<subEventIdxType, uint8_t, uint8_t, uint8_t> module_index_t;

  //create a petal index from a channel index
  static module_index_t toIdxType(subEventIdxType st, module_index_t idx)
  {
    auto from=GET_TYPE(idx);
    module_index_t midx;
    auto sfp=GET_SFP(idx);
    auto mod=GET_MOD(idx);
    char petalIndex;
    assert(st>from);
    switch(st)
      {
      case fbxChannelIdx:
	assert(0);
	break;
      case fbxModuleIdx:
	midx=std::make_tuple(st, GET_SFP(idx), GET_MOD(idx),  IDX_WILDCARD);
	break;
      case petalIdx:
	//midx=std::make_tuple(st, GET_SFP(idx)*4+GET_MOD(idx)/4, IDX_WILDCARD,  IDX_WILDCARD);
	// per https://elog.gsi.de/land/r3b2018/180906_172916/text4757-1-1.png.png
	// and https://elog.gsi.de/land/r3b2018/35
	if (sfp==0)
	  {
	    if (mod<4)       //TUDA 1 : 1
	      petalIndex=1;
	    else if (mod<8)  //TUDA 2 : 2
	      petalIndex=2;
	    else if (mod<16) // USC 1-2: 3-4 -> 3 (double petal)
	      petalIndex=3;
	    else
	      return IDX_INVALID;
	  }
	else if (sfp==1)
	  {
	    if (mod<4)       // USC 3: 5
	      petalIndex=5;
	    if (mod<12)      // LU1-2: 6-7 -> 6
	      petalIndex=6;
	    else
	      return IDX_INVALID;
	  }
	else
	  return IDX_INVALID;
	midx=std::make_tuple(st, petalIndex, IDX_WILDCARD,  IDX_WILDCARD);
	break;
      default:
	assert(0);
	return IDX_INVALID;
     }
    return midx;
  }
  
  typedef std::map<module_index_t, eventinfo_t> eventmap_t;
  typedef std::map<uint32_t, timestamp_t>  tsmap_t;


  CalifaParser();
  virtual ~CalifaParser()
    {
    }
  virtual int parseGo4(TGo4MbsEvent *);
  virtual int parse(uint32_t* p, uint32_t len); //returns 0 on success, non-zero on error
  eventmap_t* getCalifaEvents(); //returns pointer to event map, which is reset whenever a new FEBEX event comes in
  uint32_t getSysID();         //returns the last system ID from WR, or 0 if there was none
  tsmap_t* getTimestamps();      //returns map of system IDs to latest timestamps
  uint32_t getEventCount()
  {
    return this->subevent_count;
  }
 protected:
  static const uint32_t _SYSIDS[];
  static const std::set<uint32_t> SYSIDS;
  static const uint32_t FEBEX_SYSTEM_ID=0x400;
  static const uint32_t AMS_SYSTEM_ID=  0x200;
  static const uint32_t N_CHA=16;
  eventmap_t eventmap;

  std::map<module_index_t, event_t> virtevents; // persistant allocation for non-fbx-channel events
  

  tsmap_t tsmap;
  
  uint32_t lastSysID;
  uint32_t subevent_count;
  //immediate storage:
  //  event_t evts;
  //gosip_header_t gossip;
  //gosip_sub_header_t gosip_sub;
  int parseTimestamp(uint32_t *&p, uint32_t* p_end);
  module_index_t parseGosip(uint32_t* &p, eventinfo_t*&, uint32_t*&);
  int parseEvent(uint32_t *&pl_tmp, eventinfo_t* ei, module_index_t idx);
  void reset();
};

#endif
