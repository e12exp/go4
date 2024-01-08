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
#define IDX_EVENT CalifaParser::module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, 255, 255, 252)

#define IDX(A, B, C) CalifaParser::module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, A, B, C)
#define MIDX(A, B)  CalifaParser::module_index_t(CalifaParser::subEventIdxType::fbxModuleIdx, A, B, IDX_WILDCARD)

#define GET_TYPE(idx) std::get<0>(idx)
#define GET_SFP(idx)  std::get<1>(idx)
#define GET_MOD(idx)  std::get<2>(idx)
#define GET_CH(idx)   std::get<3>(idx)
#define GET_SFP_PURE(idx) (GET_SFP(idx)%10)

#define IDX_WILDCARD ((uint8_t)254)

#define GET_PETAL(idx) std::get<1>(idx)



// for trig 1:
#define WRTS_MES      0x0a00
#define WRTS_WIX      0x0b00
#define WRTS_MAIN     0x1000
#define TRIG3_OFFSET 0x30000
#define SYNC

class CalifaParser
{
 public:
  static const bool CHECK_EVT_TYPE=0;
  static const bool NOMODULES=0; // map all module indices to zero (for testing febex)
  
  // if true, skip anything not matching:
  static const uint32_t FEBEX_PROC_ID=0xd;
  static const uint32_t FEBEX_EVT_TYPE=0x64;
  static const uint32_t FEBEX_SUBEVT_TYPE=0x2710;

  enum subEventIdxType
  {
    fbxChannelIdx=0,
    fbxModuleIdx=1,
    petalIdx=2
  };



  typedef std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> module_index_t;

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
  uint32_t flattenIdx(module_index_t m)
  {
    // to avoid explicitly casting GET_ to uint32_t before shifting:
    uint32_t res=0;
    res+=GET_TYPE(m); res<<=8;
    res+=GET_SFP(m);  res<<=8;
    res+=GET_MOD(m);  res<<=8;
    res+=GET_CH(m);
    return res;
  }

  
  typedef std::map<module_index_t, eventinfo_t> eventmap_t;
  typedef std::map<uint64_t, timestamp_t>  tsmap_t;


  CalifaParser();
  virtual ~CalifaParser()
    {
    }
  virtual int parseGo4(TGo4MbsEvent *);
  virtual int  parseSubEvent(TGo4MbsSubEvent* psubevt); //returns 0 on success, non-zero on error
  eventmap_t* getCalifaEvents(); //returns pointer to event map, which is reset whenever a new FEBEX event comes in
  uint32_t getSysID();         //returns the last system ID from WR, or 0 if there was none
  tsmap_t* getTimestamps();      //returns map of system IDs to latest timestamps
  uint32_t getEventCount()
  {
    return this->subevent_count;
  }

  unsigned int getMultiplicity()
  {
    return multiplicity;
  }
  uint64_t getLastTS();
  
 protected:
  static const uint32_t _SYSIDS[];
  static const std::set<uint32_t> SYSIDS;
  static const uint32_t FEBEX_SYSTEM_ID=0x400;
  static const uint32_t AMS_SYSTEM_ID=  0x200;
  static const uint32_t N_CHA=16;
  uint16_t lasttrig;
  std::map<module_index_t, event_t> virtevents; // persistant allocation for non-fbx-channel events
  eventmap_t eventmap;
  tsmap_t tsmap;
  uint64_t last_ts;
  uint32_t lastSysID;
  uint32_t subevent_count;
  unsigned int multiplicity;

  //immediate storage:
  //  event_t evts;
  //gosip_header_t gossip;
  //gosip_sub_header_t gosip_sub;
  int parseTimestamp(uint32_t *&p, uint32_t* p_end);
  void updateWRTS(uint32_t system_id, uint64_t wrts);
  module_index_t parseGosipHeader(uint32_t* &p, eventinfo_t*&, uint32_t*&, uint8_t control);
  int parseCalifaHit(uint32_t *&pl_tmp, eventinfo_t* ei, module_index_t idx);
  void reset();
  static void traceAnalysis(eventinfo_t*);
  static void traceTrigAnalysis(eventinfo_t*);
};

#endif
