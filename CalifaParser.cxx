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
      linfo("ignored event with evt_type=0x%x, subevent_type=0x%x, procid=0x%x\n", evt_type, subevt_type, procid);
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
  ldbg("parsing of subevent %d completed successfully with %d good and %d / %d  bad gosip/event headers!\n",
	this->subevent_count, goodheaders, badgosipheaders, badeventheaders);
  this->multiplicity=this->eventmap.size();
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
      ts->whiterabbit_prev=this->tsmap[system_id].whiterabbit;
      this->tsmap[system_id]=*ts;

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

CalifaParser::module_index_t CalifaParser::parseGosipHeader(uint32_t *&p,
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

  uint8_t sfp_offset=70;
  if (control==90)
    {
      sfp_offset=10;
    }
  else if (control==91)
    {
      sfp_offset=20;
    }
  else
    {
      lerror("I do not know about SE_CONTROL==%d, will assign an sfp offset of %d to them.", control, sfp_offset);
    }
  //linfo("found a good event\n");
  module_index_t idx=std::make_tuple(CalifaParser::subEventIdxType::fbxChannelIdx,
				     (uint8_t)(gosip_sub->sfp_id)+sfp_offset,
				     (uint8_t)(gosip_sub->module_id),
				     gosip_sub->submemory_id);
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
			     CalifaParser::module_index_t idx)
{
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

  const std::vector<subEventIdxType> virtevent_types={};//petalIdx};

  for (auto& st: virtevent_types)
    {
      auto vidx=CalifaParser::toIdxType(st, idx);
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
  
  for (int i=0; i<ei->tracepoints-1; i++)
    {
      double s=getTracePoint(ei, i+1)-getTracePoint(ei, i);
      if (s>max_slope)
        {
          max_slope=s;
          trace_start=i;
        }
    }


  double trace_max=-INF;
  for (int i=0; i<ei->tracepoints; i++)
    {
      double s=getTracePoint(ei, i);
      if (s>trace_max)
        trace_max=s;
    }
  
  ei->trace_en=trace_max-getTracePoint(ei, 0); // cheap
  

}
