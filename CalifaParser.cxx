#include "CalifaParser.h"
#include "struct_eventinfo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
const uint32_t CalifaParser::_SYSIDS[]= {0x100, 0x200, 0x400};
const std::set<uint32_t> CalifaParser::SYSIDS(_SYSIDS, _SYSIDS+3);

CalifaParser::CalifaParser(): eventmap(), tsmap(), subevent_count(0)
{
  //initialize stuff
  //this->eventmap();
}

void CalifaParser::reset()
{
  for (auto it=this->eventmap.begin(); it!=this->eventmap.end(); ++it)
    {
      if (it->second.evnt)
	free(it->second.evnt);
    }
  this->eventmap.clear();

}

//len in words of 4 bytes
int CalifaParser::parse(uint32_t* p, uint32_t len)
{
  //linfo("parsing %d words starting from %lx\n", len, p);
  uint32_t* p_end=p+len;
  //  if (parseTimestamp(p, p_end)<0) //ignore positive errors here.
  //  return -1; //a timestamp with a bad magic number
  if ((*p) == 0xbad00bad)
    {
      linfo("found event marked as bad, ignored it.\n");
      return 1;
    }
  int goodheaders=0, badgosipheaders=0, badeventheaders=0;
  this->reset();
  while(p<p_end)
    {
      uint32_t *next;
      //skip DMA padding
      //ldbg("pointer after skipped padding: %lx\n", p);
      while((*(p) & 0xfff00000) == 0xadd00000)
	p++;
      //ldbg("pointer after skipped padding: %lx\n", p);
      eventinfo_t* ei=NULL;
      if (this->parseGosip(p, ei, next))
	  badgosipheaders++;
	else
	  if (this->parseEvent(p, ei))
	    badeventheaders++;
	  else
	    goodheaders++;
      //continue with the next header
      p=next;
    }
  this->subevent_count++;
  linfo("parsing of subeven %d completed successfully with %d good and %d / %d  bad gosip/event headers!\n",
	this->subevent_count, goodheaders, badgosipheaders, badeventheaders);
  return goodheaders==0;
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
  if (!CalifaParser::SYSIDS.count(system_id))
    return 1;
  linfo("timestamp found with system id 0x%x\n", system_id);
  while(data < p_end && *data++ == system_id)
    {
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
	  lerror("Unknown timestamp ID 0x%02x\n", *data & 0xffff0000);
	  return -1; //unknown timestamp
	}
    }
  if (ts->whiterabbit || ts->titris)
    {
      this->tsmap[system_id]=*ts;
      this->lastSysID=system_id;
      p=data;
      return 0;
    }
  //no timestamp data
  return 1;
}

int CalifaParser::parseGosip(uint32_t *&p,
			     eventinfo_t* &ei, uint32_t* &next)
{
  //linfo("gosip first word : @%lx is %lx\n", p, *p);
  //linfo("gosip second word: @%lx is %lx\n", p+1, *(p+1));
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
      )
    {
      //ldbg("gosip: ignored bad event\n");
      return 1;
    }
  //linfo("found a good event\n");
  module_index_t idx=std::make_tuple((uint8_t)(gosip_sub->sfp_id),
				     (uint8_t)(gosip_sub->module_id),
				     gosip_sub->submemory_id);
  if (this->eventmap.count(idx))
    {
      lerror("gosip: multiple subheaders for the same channel, ignoring all but the first.\n");
      return 1;
    }
  this->eventmap.count(idx);
  ei=&(this->eventmap[idx]);
  ei->gosip=gosip_sub;
  ei->evnt=NULL;
  ei->trace=NULL;
  return 0;
}


int CalifaParser::parseEvent(uint32_t *&pl_tmp,
			     eventinfo_t* ei)
{
  event_t* evnt=(event_t*)malloc(sizeof(event_t));
  ei->evnt=evnt;
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
      evnt->overflow = *pl_tmp && 0xffffff;
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
      return 0;
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
      return 0;
    default:
      lerror(" event: Invalid event magic number: 0x%04x\n", evnt->magic);
      free(ei->evnt);
      ei->evnt=NULL;
      return 1;
    }

  if(evnt->size > processed_size && (*pl_tmp >> 16) == 0xBEEF)
    {
      //time over threshold
      evnt->tot = *pl_tmp++ & 0xffff;
      memcpy(evnt->tot_samples, pl_tmp, 8); 
      pl_tmp += 2;
      processed_size += 12;
    }

  if (evnt->size > processed_size) 
    {
      //traces
      trace_head_t * data=(trace_head_t * )&(pl_tmp);
      if (data->magic_2bad != 0x2bad) 
	  lerror("ERROR>> wrong data header ident: 0x%x "
		 "instead of 0x2bad\n", data->magic_2bad);
      else
	  ei->trace=data;
    }
}

