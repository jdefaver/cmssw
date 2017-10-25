#ifndef EventFilter_Phase2TrackerRawToDigi_Phase2TrackerPhase2TrackerFEDRawChannelUnpacker_H // {
#define EventFilter_Phase2TrackerRawToDigi_Phase2TrackerPhase2TrackerFEDRawChannelUnpacker_H

#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerFEDDAQHeader.h"
#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerFEDDAQTrailer.h"
#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerFEDChannel.h"
#include <stdint.h>

namespace Phase2Tracker {

  // unpacker for RAW CBC data
  // each bit of the channel is related to one strip
  class Phase2TrackerFEDRawChannelUnpacker
  {
  public:
    Phase2TrackerFEDRawChannelUnpacker(const Phase2TrackerFEDChannel& channel);
    uint8_t stripIndex() const { return currentStrip_; }
    bool stripOn() const { return bool((currentWord_>>bitInWord_)&0x1); }
    bool hasData() const { return valuesLeft_; }
    Phase2TrackerFEDRawChannelUnpacker& operator ++ ();
    Phase2TrackerFEDRawChannelUnpacker& operator ++ (int);
  private:
    const uint8_t* data_;
    uint16_t currentOffset_;
    uint8_t currentStrip_;
    uint16_t valuesLeft_;
    uint8_t currentWord_;
    uint8_t bitInWord_;
  }; // end of Phase2TrackerFEDRawChannelUnpacker

} // end of Phase2Tracker namespace

#endif // } end def EventFilter_Phase2TrackerRawToDigi_Phase2TrackerPhase2TrackerFEDRawChannelUnpacker_H

