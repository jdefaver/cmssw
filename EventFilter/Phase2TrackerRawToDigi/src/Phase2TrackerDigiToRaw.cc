#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerDigiToRaw.h"
#include "CondFormats/DataRecord/interface/Phase2TrackerCablingRcd.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Common/interface/DetSetVector.h"

namespace Phase2Tracker
{
  Phase2TrackerDigiToRaw::Phase2TrackerDigiToRaw(FEDDAQHeader daq_header, FEDDAQTrailer daq_trailer, Phase2TrackerFEDHeader fed_header, int mode):
    daq_header_(daq_header), 
    daq_trailer_(daq_trailer), 
    fed_header_(fed_header),
    mode_(mode),
    payload_size_(0)
  {
  }

  uint8_t* Phase2TrackerDigiToRaw::buildBuffer()
  {
    // TODO : get these from the headers themselves
    int daq_header_size  = 8;
    int daq_trailer_size = 8;
    int fed_header_size  = 16;
    int position = 0;
    const int buffer_size = daq_header_size + fed_header_size + payload_size_ + daq_trailer_size;
    uint8_t buffer[buffer_size];

    memcpy(buffer+position,daq_header_.data(),daq_header_size);
    position += daq_header_size;

    memcpy(buffer+position,fed_header_.data(),fed_header_size);
    position += fed_header_size;

    position += payload_size_; 

    memcpy(buffer+position,daq_trailer_.data(),daq_trailer_size);
    buffer_ = buffer;
    return buffer_;
  }

  void Phase2TrackerDigiToRaw::buildPayload(edm::Event& event, edm::EventSetup const& es, const Phase2TrackerCabling*& c, edm::Handle< edmNew::DetSetVector<SiPixelCluster> >& handle)
  {
    cabling_ = c;
    // get 'raw' digis
    event.getByLabel("siPixelClusters","", handle);
    const edmNew::DetSetVector<SiPixelCluster>* digs = handle.product();
    for (edmNew::DetSetVector<SiPixelCluster>::const_iterator it = digs->begin(); it != digs->end(); it++)
    {
        std::cout << std::endl << "Digis for detId : " << it->detId() << std::endl;
        for (edmNew::DetSet<SiPixelCluster>::const_iterator it2 = it->begin(); it2 != it->end(); it2++)
        {
            std::cout << it2->x() << " " << it2->y() << " " << it2->sizeX() << " " << it2->sizeY() << std::endl;
        }
    }
  }
}
