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
#include <idx.h>


#if 1
#include "TGo4MbsEvent.h"
#endif

//#define IDX_NONE module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, 255, 255, 254)
//#define IDX_CHANNEL_WILDCARD 254
//#define IDX_INVALID module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, 255, 255, 253)
//#define IDX_EVENT module_index_t(CalifaParser::subEventIdxType::fbxChannelIdx, 255, 255, 252)


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
