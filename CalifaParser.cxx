#include "CalifaParser.h"
#include "struct_eventinfo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include <list>

#include <algorithm>
#ifndef INF
#define INF INFINITY
#endif

CalifaParser::CalifaParser(): eventmap(), tsmap(), subevent_count(0), multiplicity(0)
{
  memset(&last_ts, 0, sizeof(last_ts));
  if (CHECK_EVT_TYPE)
    {
      fprintf(stderr, "CalifaParser: will IGNORE any events not matching PROC=0x%x, EVT=0x%x, SUBEVT=0x%x\n\n",
              FEBEX_PROC_ID, FEBEX_EVT_TYPE, FEBEX_SUBEVT_TYPE);
    }
  else
    {
      fprintf(stderr, "CalifaParser:Will try to parse EVERYTHING as CALIFA febex data!\n");
    }
  //initialize stuff
  //this->eventmap();
}

void CalifaParser::reset()
{
  linfo("resetting eventmap.\n");
  for (auto it=this->eventmap.begin(); it!=this->eventmap.end(); ++it)
      if (it->second.evnt)
	free(it->second.evnt);
  this->eventmap.clear();
  this->multiplicity=0;
  this->lastSysID=0;
  this->last_ts=0;
}

int CalifaParser::parseGo4(TGo4MbsEvent* fInput)
{
  linfo("CalifaParser::parseGo4 .=========================================================================================\n");
  if (fInput == 0) {
    linfo("AnlProc: no input event !\n");
    return 1;
  }
  if (fInput->GetTrigger() > 11) {
    linfo("**** CalifaProc: Skip trigger event\n" );
    return 2;
  }
  this->lasttrig=fInput->GetTrigger();
  fInput->ResetIterator();
  this->reset();
  linfo("     CalifaParser::parseGo4 Reset Iterator.\n");

  while(auto psubevt = fInput->NextSubEvent())
    {
      int r=this->parseSubEvent(psubevt) ;

      if (r)
	{
	  linfo("     CalifaProc: bad subevent, ignored.\n");
	  //a bad subevent
	  //	  return 3;
	  continue;
	}
      else
	{
	  //nothing. events are automatically added to the eventlist. 

	  /*
	  auto e=this->parser->getCalifaEvents();
	  for (auto ei=e->begin(); ei!=e->end(); ++ei)
	    linfo("found index: %d %d %d\n", 
		  );*/
	  //ldbg("calling processEvent for %d processors\n", this->subprocessors.size());
	}
    }
  
  return 0;
}



//len in words of 4 bytes
int CalifaParser::parseSubEvent(TGo4MbsSubEvent* psubevt)
{
  linfo("parsing subevent ====================\n");
  auto p=(uint32_t*)psubevt->GetDataField();
  auto len=psubevt->GetIntLen();
  if (!p)
    return -1; 
  //linfo("parsing %d words starting from %lx\n", len, p);
  uint32_t* p_end=p+len;
  auto ts_ret=parseTimestamp(p, p_end);
  if (!ts_ret)
    {
      linfo("after wrts: @%lx is %lx\n", p, *p);
    }
  else if (ts_ret<0) //ignore positive errors here.
    {
      linfo("bad WRTS.\n");
      //return -1; //a timestamp with a bad magic number
    }
  else // positive error=> no ts info.
    {
      linfo("no WRTS found. assuming califa stand-alone.\n");
    }
  uint32_t evt_type = psubevt->GetType();
  uint32_t subevt_type = psubevt->GetSubtype();
  uint32_t procid = psubevt->GetProcid();
  uint8_t control=psubevt->GetControl();

  if (CHECK_EVT_TYPE
      && (evt_type != FEBEX_EVT_TYPE || subevt_type != FEBEX_SUBEVT_TYPE || procid != FEBEX_PROC_ID))
    {
      static std::set<std::tuple<uint32_t, uint32_t, uint32_t>> ignored_warned;
      auto t=std::make_tuple(evt_type, subevt_type, procid);
      if (!ignored_warned.count(t))
        {
          lerror("ignored event with evt_type=0x%x, subevent_type=0x%x, procid=0x%x\n", evt_type, subevt_type, procid);
          ignored_warned.insert(t);
        }
      return 0; 
    }
  
  
  if ((*p) == 0xbad00bad)
    {
      linfo("found event marked as bad, ignored it.\n");
      return 1;
    }


  int goodheaders=0, badgosipheaders=0, badeventheaders=0;
  while(p<p_end)
    {
      uint32_t *next;
      //skip DMA padding
      //ldbg("pointer after skipped padding: %lx\n", p);
      while((*(p) & 0xfff00000) == 0xadd00000)
	p++;
      //ldbg("pointer after skipped padding: %lx\n", p);
      eventinfo_t* ei=NULL;
      auto idx=this->parseGosipHeader(p, ei, next, control);
      if (idx==IDX_INVALID)
	  badgosipheaders++;
	else
	  if (this->parseCalifaHit(p, ei, idx))
	    badeventheaders++;
	  else
	    goodheaders++;
      //continue with the next header
      p=next;
    }
  this->subevent_count++;
  this->multiplicity=this->eventmap.size();
  ldbg("parsing of subevent %d completed successfully with %d good and %d / %d  bad gosip/event headers, multiplicity %d.\n",
       this->subevent_count, goodheaders, badgosipheaders, badeventheaders, this->multiplicity);
 return badgosipheaders+badeventheaders;
}

CalifaParser::eventmap_t* CalifaParser::getCalifaEvents()
{
  return &(this->eventmap);
}
uint32_t CalifaParser::getSysID()
{
  return this->lastSysID;
}
CalifaParser::tsmap_t* CalifaParser::getTimestamps()
{
  return &(this->tsmap);
}
uint64_t CalifaParser::getLastTS()
{
  return this->last_ts;
}

//  event_t evts;
//  gosip_t gossip;
//  timestamp_t ts;
int CalifaParser::parseTimestamp(uint32_t *&p, uint32_t* p_end)
{
  timestamp_t tsdat;
  timestamp_t* ts=&tsdat;
  uint32_t *data = p;
  uint32_t system_id = *data;

  ts->whiterabbit = 0;
  ts->titris = 0;
  linfo("potential timestamp found with system id 0x%x\n", system_id);
  while(data < p_end && *data == system_id)
    {
      data++; //only increment if loop condition holds. 
      switch(*data & 0xffff0000)
	{
	case 0x00f70000:
	  // TITRIS
	  ts->titris = *data++ & 0xffff;
	  ts->titris |= (*data++ & 0xffff) << 16;
	  ts->titris |= (uint64_t)(*data++ & 0xffff) << 32;
	  //			printf("TITRIS TS for 0x%x: 0x%012llx\n", system_id, ts->titris);
	  break;
	case 0x03e10000:
	  // Whiterabbit
	  ts->whiterabbit = *data++ & 0xffff;
	  ts->whiterabbit |= (*data++ & 0xffff) << 16;
	  ts->whiterabbit |= (uint64_t)(*data++ & 0xffff) << 32;
	  ts->whiterabbit |= (uint64_t)(*data++ & 0xffff) << 48;

	  //			printf("WR TS for 0x%x: 0x%016llx\n", system_id, ts->whiterabbit);
	  break;
	default:
	  //lerror("Unknown timestamp ID 0x%02x\n", *data & 0xffff0000);
	  return -1; //unknown timestamp
	}
    }
  if (ts->whiterabbit || ts->titris)
    {
      if (system_id!=0xa00 && system_id != 0xb00)
        this->updateWRTS(system_id, ts->whiterabbit);
      
      uint32_t offset=0; // add to system id for specific triggers
#if 0
      if (this->lasttrig==1)
        {
          offset=0;
        }
      else if (this->lasttrig==3) // sync
        {
          offset=0x30000;
        }
      else
        {
          lerror("Unhandled trigger type %d for WRTS id %d", this->lasttrig, system_id);
          offset=0xffff0000;
        }
#endif      
      this->lastSysID=system_id+offset;
      this->last_ts=ts->whiterabbit;
      p=data;
      return 0;
    }
  //no timestamp data
  return 1;
}

void CalifaParser::updateWRTS(uint32_t system_id, uint64_t wrts)
{
  uint32_t count=0;
  
  timestamp_t ts;
  ts.whiterabbit=wrts;
  ts.whiterabbit_prev=this->tsmap[system_id].whiterabbit;
  this->tsmap[system_id]=ts;
  if (0 && !count++%1000)
    {
      printf("timestamp IDs:\n");
      for (auto& a: this->tsmap)
        printf("0x%lx ", a.first);
      printf("\n");
    }
  
}

module_index_t CalifaParser::parseGosipHeader(uint32_t *&p,
						      eventinfo_t* &ei, uint32_t* &next, uint8_t control)
{
  linfo("gosip first word : @%lx is %lx\n", p, *p);
  linfo("gosip second word: @%lx is %lx\n", p+1, *(p+1));
  gosip_sub_header_t* gosip_sub=(gosip_sub_header_t*)(p);

  p += sizeof(gosip_sub_header_t)/4;
  //ldbg("gosip: l_cha %d\n ", gosip_sub->submemory_id);
  if (gosip_sub->header_size != 0x34) {
    //0x34: 3: three bytes follow before data size
    //      4: data size is 4 bytes long. 
    // go figure. 
    lerror("gosip header size is wrong = %x \n", 
    	   gosip_sub->header_size);
  }

  //  printf("%x-> %x.%x\n", *(uint32_t*)gosip_sub, gosip_sub->sfp_id, gosip_sub->module_id);
  next=p+gosip_sub->data_size/4;
  //linfo("calculated next pointer to be")
  //on error, skip to next submodule
  uint8_t pc_id{7};
  if (gosip_sub->data_size == 0 
      //empty event marks end of stream???
      || gosip_sub->submemory_id == 0xff 
      //Special channel //max timestamp readout trigger
      || gosip_sub->submemory_id >= CalifaParser::N_CHA
      //deactivated channel
      || (gosip_sub->data_size == 4 && *p == 0xdeadda7a)
      //Data reduction: Empty channel
      || gosip_sub->sfp_id>3 // weird broken events
      )
    {
      //ldbg("gosip: ignored bad event\n");
      return IDX_INVALID;
    }

  if (90<=control && control<=93)
    {
      pc_id=control-90; // 0: m0, 1: w1, 2: m1, 3: w1
    }
  else
    {
      lerror("I do not know about SE_CONTROL==%d, will assign a pc_id of %d to them.\n", control, pc_id);
    }

  uint8_t mod=gosip_sub->module_id;
  if (NOMODULES)
    mod=0;
  //linfo("found a good event\n");
  module_index_t idx={fbxChannelIdx,
                      pc_id,
                      gosip_sub->sfp_id,
                      mod,
                      gosip_sub->submemory_id};
  
  bool duplicate=0;
  if (this->eventmap.count(idx))
    {
      lerror("gosip: multiple subheaders for the same channel (%d.%d.%d), using one with highest energy.\n",
	     (gosip_sub->sfp_id),
	     (uint8_t)(gosip_sub->module_id),
	     gosip_sub->submemory_id	     );
      //      return 1;
      duplicate=1;
    }
 
  ei=&(this->eventmap[idx]);
  memset(ei, 0, sizeof(eventinfo_t));

  ei->gosip=gosip_sub;
  if (!duplicate)
    {
      ei->evnt=NULL;
    }
  ei->trace=NULL;
  return idx;
}


int CalifaParser::parseCalifaHit(uint32_t *&pl_tmp,
			     eventinfo_t* ei,
			     module_index_t idx)
{
  // hack: only add the califa stuff to tsmap now

  if (idx.mod<16)
    {
      // regular califa range
      updateWRTS(this->lastSysID, this->last_ts);
    }
  else
    {
      updateWRTS(0xf00000+(idx.pc_id<<6)+(idx.sfp<<4)+idx.ch, this->last_ts);
    }
#define FAKE_MAIN 0
#if FAKE_MAIN
  if (idx==IDX(10,3,10))
    {
      //      printf("faking main wrts!\n");
      updateWRTS(0x1000, this->last_ts);
    }
#endif
  

  
  event_t* evnt=(event_t*)malloc(sizeof(event_t));
  uint32_t processed_size;
  switch((*pl_tmp) >> 16)
    {
      // Old event structure
    case 0xAFFE:
      evnt->size = *pl_tmp & 0xffff;
      evnt->magic = *pl_tmp++ >> 16;
      evnt->event_id = *pl_tmp++;
      evnt->timestamp = *pl_tmp++;
      evnt->timestamp |= ((uint64_t)*pl_tmp++) << 32;
      memcpy(evnt->cfd_samples, pl_tmp, 8); pl_tmp+=2;
      evnt->overflow = *pl_tmp & 0xffffff;
      evnt->self_triggered = *pl_tmp++ >> 24;
      evnt->num_pileup = *pl_tmp & 0xffff;
      evnt->num_discarded = *pl_tmp++ >> 16;
      evnt->energy = *pl_tmp++ & 0xffff;
      pl_tmp++;
      evnt->n_f = *pl_tmp & 0xffff;
      evnt->n_s = *pl_tmp++ >> 16;
      evnt->tot = 0;
      memset(evnt->tot_samples, 0, sizeof(int16_t)*4);
      processed_size = event_t_size;
      break;
      // New event structure
    case 0x115A:
      evnt->size = *pl_tmp & 0xffff;
      evnt->magic = *pl_tmp++ >> 16;
      evnt->event_id = *pl_tmp++;
      evnt->timestamp = *pl_tmp++;
      evnt->timestamp |= ((uint64_t)*pl_tmp++) << 32;
      memcpy(evnt->cfd_samples, pl_tmp, 8); pl_tmp+=2;
      evnt->overflow = *pl_tmp & 0xffffff;
      evnt->self_triggered = *pl_tmp++ >> 24;
      evnt->num_pileup = *pl_tmp & 0xffff;
      evnt->num_discarded = *pl_tmp++ >> 16;
      evnt->energy = *pl_tmp++ & 0xffff;
      evnt->n_f = *pl_tmp & 0xffff;
      evnt->n_s = *pl_tmp++ >> 16;
      evnt->tot = 0;
      memset(evnt->tot_samples, 0, sizeof(int16_t)*4);
      processed_size = event_115a_t_size;
      break;
    default:
      lerror(" event: Invalid event magic number: 0x%04x\n", evnt->magic);
      free(evnt);
      ei->evnt=NULL;
      return 10;
    }


  bool skip=0;
  if (!(ei->evnt) || evnt->energy > ei->evnt->energy)
    {
      if (ei->evnt)
	{
	  lerror("replacing event for %d.%d.%d (en %d) by an event for the same subheader with energy %d\n",
		 (ei->gosip->sfp_id), (uint8_t)(ei->gosip->module_id),  ei->gosip->submemory_id,
		 ei->evnt->energy, evnt->energy);
	  free(ei->evnt);
	}
      ei->evnt=evnt;
      ei->wrts=this->last_ts;
    }
  else
    {
      lerror("not replacing event for %d.%d.%d (en %d) by an event for the same subheader with energy %d\n",
	     (ei->gosip->sfp_id), (uint8_t)(ei->gosip->module_id),  ei->gosip->submemory_id,
	     ei->evnt->energy, evnt->energy);
      skip=1;
    }




  if(evnt->size > processed_size && (*pl_tmp >> 16) == 0xBEEF)
    {
      //lerror("found TOT data");
      //time over threshold
	  uint16_t tot = *pl_tmp++ & 0xffff;
      if (!skip)
	{
	  evnt->tot=tot;
	  memcpy(evnt->tot_samples, pl_tmp, 8); 
	}
      pl_tmp += 2;
      processed_size += 12;
    }

  if (evnt->size > processed_size) 
    {
      //traces
      trace_head_t * data=(trace_head_t * )(pl_tmp);

      if (data->magic_2bad != 0x2bad) 
	{
	  lerror("ERROR>> wrong data header ident at %p: 0x%x "
		 "instead of 0x2bad\n", 
		 &(data->magic_2bad),
		 data->magic_2bad);
	  //lerror("data=%p\n", data);
	  //lerror("pl_tmp=%p\n", pl_tmp);
	  //dumpMem(pl_tmp-4, evnt->size-processed_size);
	}
      else
	{
	  if (!skip)
	    {
	      ei->trace=data;
	      //don't ask me why we need -2 here, but the last points
	      // are often bad. 
	      ei->tracepoints=std::max(0, int(data->size/2-sizeof(trace_head_t)-2));
	      //printf("tl=%d, %d %lu", ei->tracepoints, data->size/2,sizeof(trace_head_t));
	    }
	}
    }
  else
    {
      //lerror("No traces found!\n");
    }
  traceAnalysis(ei);
  traceTrigAnalysis(ei);
  
  const std::vector<subEventIdxType> virtevent_types={};//petalIdx};

  for (auto& st: virtevent_types)
    {
      auto vidx=toIdxType(st, idx);
      if (vidx!=IDX_INVALID)
	{
	  if (!eventmap.count(vidx))
	    {
	      auto vei=&(eventmap[vidx]);
	      memset(vei, 0, sizeof(*vei));
	      vei->evnt=(event_t*)malloc(sizeof(event_t));
	      memset(vei->evnt, 0, sizeof(event_t));
	    }
	  eventmap[vidx]+=evnt;
	}
    }
  
  if (skip)
    free(evnt);

  return 0;
}

void CalifaParser::traceAnalysis(eventinfo_t* ei)
{

  if (!ei->trace)
    {
      ei->trace_start=-1; // which sample of the trace has the highest slope?
      ei->max_slope=NAN;  // what is that slope
      ei->trace_en=NAN;   // what is the difference between the maximum of the trace and the baseline before?
      return;
    }

  auto& trace_start=ei->trace_start;
  auto& max_slope=ei->max_slope;
  
  trace_start=-1;
  max_slope=-INF;

  int max=std::min(ei->tracepoints-1, 200U);
  
  for (int i=1; i<max; i++)
    {
      double s=getTracePoint(ei, i+1)-getTracePoint(ei, i);
      if (s>max_slope)
        {
          max_slope=s;
          trace_start=i;
        }
    }


  double trace_max=-INF;
  for (int i=1; i<max; i++)
    {
      double s=getTracePoint(ei, i);
      if (s>trace_max)
        trace_max=s;
    }
  
  ei->trace0=getTracePoint(ei, 1);
  ei->bl_slope=getTracePoint(ei, 11)-getTracePoint(ei, 1);

  if (std::abs(ei->bl_slope)>5)
    ei->trace0=NAN;
  ei->trace_en=trace_max-getTracePoint(ei, 1); // cheap

  
  ei->trace_en_diff=ei->trace_en-ei->evnt->energy;
  if (ei->evnt->overflow || ei->evnt->num_pileup)
    ei->trace_en_diff=NAN;
  

}

#define TRIG_GATE 

void CalifaParser::traceTrigAnalysis(eventinfo_t* ei)
{
  // we are assuming decimated 25MHz traces, so we use half the values
  // from febex.db
  const int discr_int=24/2; // how many samples to integrate?
  const int discr_gap=50/2; // gap of trapezoidal filter
  const int discr_shift_right=3; // gap of trapezoidal filter
  const int margin=5;
  auto h=ei->trace;
  if (!h)
    {
      ei->discr_amp=NAN;
      return;
    }

  double m{};
  double filt{};
  int maxPos;
  for (unsigned int i=0; i<ei->tracepoints-margin; i++)
    {
      filt+=getTracePoint(h, i          )-getTracePoint(h, i-discr_int);
      filt-=getTracePoint(h, i-discr_gap)-getTracePoint(h, i-discr_int-discr_gap);
      if (0)
      printf("%d:%f incoming: %f outgoing %f\n",
             i, filt,
             getTracePoint(h, i          )-getTracePoint(h, i-discr_int),
             getTracePoint(h, i-discr_gap)-getTracePoint(h, i-discr_int-discr_gap));
      if (m<filt && i>discr_int+discr_gap+margin)
        {
          
          //printf("new max!\n");
          m=filt;
          maxPos=i;
        }
    }
  ei->discr_amp=m/pow(2, discr_shift_right);
  ei->discr_max=maxPos;
}
