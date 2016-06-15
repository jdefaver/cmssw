#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerDigiToRaw.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"
#include "CondFormats/DataRecord/interface/Phase2TrackerCablingRcd.h"
#include "CondFormats/SiStripObjects/interface/Phase2TrackerCabling.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Common/interface/DetSetVector.h"

namespace Phase2Tracker
{
  const int MAX_NP = 31; // max P clusters per concentrator i.e. per side
  const int MAX_NS = 31; // same for S clusters

  std::pair<int,int> SortExpandAndLimitClusters(std::vector<stackedDigi> & digis, int max_ns, int max_np)
  {
    std::vector<stackedDigi> processed;
    // number of clusters allowed : P-left, P-right, S-left, S-right 
    int roomleft[4] = {max_ns,max_ns,max_np,max_np};
    // fill left and right vectors, expand big clusters
    for(auto dig = digis.begin(); dig < digis.end(); dig++)
    {
      std::vector<stackedDigi> parts = dig->splitDigi();
      for(auto id = parts.begin(); id < parts.end(); id++)
      {
        if(roomleft[id->getSideType()] > 0) 
        { 
          processed.push_back(*id); 
          roomleft[id->getSideType()] -= 1;
        }
      }
    }
    // Sort vector
    std::sort(processed.begin(),processed.end());
    // replace input vector
    digis.swap(processed);
    // return number of S and P clusters
    return std::make_pair(2*max_ns - roomleft[2] - roomleft[3], 2*max_np - roomleft[0] - roomleft[1]); 
  }

  Phase2TrackerDigiToRaw::Phase2TrackerDigiToRaw(const Phase2TrackerCabling * cabling, const TrackerGeometry* tGeom, const TrackerTopology* tTopo, std::map< int, std::pair<int,int> > stackMap, edm::Handle< edmNew::DetSetVector<Phase2TrackerCluster1D> > digis_handle, int mode):
    cabling_(cabling),
    tTopo_(tTopo),
    tGeom_(tGeom),
    stackMap_(stackMap),
    digishandle_(digis_handle),
    mode_(mode),
    FedDaqHeader_(0,0,0,DAQ_EVENT_TYPE_SIMULATED), // TODO : add L1ID
    FedDaqTrailer_(0,0)
  {
    FedHeader_.setDataFormatVersion(2);
    FedHeader_.setDebugMode(SUMMARY); 
    FedHeader_.setEventType((uint8_t)0x04);
  }

  void Phase2TrackerDigiToRaw::buildFEDBuffers(std::auto_ptr<FEDRawDataCollection>& rcollection)
  {
    // store digis for a given fedid
    std::vector<edmNew::DetSet<Phase2TrackerCluster1D>> digis_t;
    // store active connections for a given fedid
    std::vector<bool> festatus (72,false);
    // iterate on all possible channels 
    Phase2TrackerCabling::cabling conns = cabling_->orderedConnections(0);
    Phase2TrackerCabling::cabling::const_iterator iconn = conns.begin(), end = conns.end(), icon2;
    while(iconn != end)
    {
      unsigned int fedid = (*iconn)->getCh().first;
      for (icon2 = iconn; icon2 != end && (*icon2)->getCh().first == fedid; icon2++)
      {
        // detid of first plane is detid of module + 1
        // FIXME (when it is implemented) : we should get this from the topology / geometry
        unsigned int detid = stackMap_[(*icon2)->getDetid()].first;
        // FIXME (when proper cabling exists) : because we use test cabling, we have some detids set to 0 : we should ignore them
        if ( detid == 0 ) continue;
        // end of fixme
        edmNew::DetSetVector<Phase2TrackerCluster1D>::const_iterator  digis;
        digis = digishandle_->find(detid);
        if (digis != digishandle_->end())
        {
          digis_t.push_back(*digis);
          festatus[(*icon2)->getCh().second] = true;
        }
        digis = digishandle_->find(tTopo_->partnerDetId(detid));
        if (digis != digishandle_->end())
        {
          digis_t.push_back(*digis);
          festatus[(*icon2)->getCh().second] = true;
        }
      }
      // save buffer
      FedHeader_.setFrontendStatus(festatus);
      // write digis to buffer
      std::vector<uint64_t> fedbuffer = makeBuffer(digis_t);
      FEDRawData& frd = rcollection->FEDData(fedid);
      int size = fedbuffer.size()*8;
      frd.resize(size);
      memcpy(frd.data(),&fedbuffer[0],size);
      festatus.assign(72,false);
      digis_t.clear();
      // advance connections pointer
      iconn = icon2;
    }
  }

  std::vector<uint64_t> Phase2TrackerDigiToRaw::makeBuffer(std::vector<edmNew::DetSet<Phase2TrackerCluster1D>> digis)
  {
    uint64_t bitindex = 0;
    int moduletype = -1;
    std::vector<uint64_t> fedbuffer;
    // add daq header
    fedbuffer.push_back(*(uint64_t*)FedDaqHeader_.data());
    bitindex += 64;
    // add fed header
    uint8_t* feh = FedHeader_.data();
    fedbuffer.push_back(*(uint64_t*)feh);
    fedbuffer.push_back(*(uint64_t*)(feh+8));
    bitindex += 128;
    // looping on detids
    std::vector<edmNew::DetSet<Phase2TrackerCluster1D>>::const_iterator idigi;
    for (idigi = digis.begin(); idigi != digis.end(); idigi++ )
    {
      // get id of stack
      unsigned int detid = idigi->detId();
      TrackerGeometry::ModuleType det_type = tGeom_->getDetectorType(detid);
      if ( det_type == TrackerGeometry::ModuleType::Ph2PSP or det_type == TrackerGeometry::ModuleType::Ph2PSS) {
        moduletype = 1;
      } else if ( det_type == TrackerGeometry::ModuleType::Ph2SS ) {
        moduletype = 0;
      } else {
        // FIXME: raise exception : we should never be here
      } 
      // container for digis, to be sorted afterwards
      std::vector<stackedDigi> digs_mod;
      edmNew::DetSet<Phase2TrackerCluster1D>::const_iterator it;
      // pair modules if there are digis for both
      if(tTopo_->isLower(idigi->detId()) == 1)
      {  
        // digis for inner plane (P in case of PS)
        if( (idigi+1) != digis.end() and (int)(idigi+1)->detId() == (int)tTopo_->partnerDetId(detid))
        {
          // next digi is the corresponding outer plane : join them
          for (it = idigi->begin(); it != idigi->end(); it++)
          {
            digs_mod.push_back(stackedDigi(it,LAYER_INNER,moduletype));
          }
          idigi++;
          for (it = idigi->begin(); it != idigi->end(); it++)
          {
            digs_mod.push_back(stackedDigi(it,LAYER_OUTER,moduletype));
          }
        }
        else
        {
          // next digi is from another module, only use this one
          for (it = idigi->begin(); it != idigi->end(); it++)
          {
            digs_mod.push_back(stackedDigi(it,LAYER_INNER,moduletype));
          }
        }
      }
      else
      {
        // digis from outer plane (S in case of PS) 
        for (it = idigi->begin(); it != idigi->end(); it++)
        {
          digs_mod.push_back(stackedDigi(it,LAYER_OUTER,moduletype));
        }
      }
      // here we:
      // - sort all digis
      // - divide big clusters into 8-strips parts
      // - count digis on each side/layer (concentrator)
      // - remove extra digis
      std::sort(digs_mod.begin(),digs_mod.end());
      std::pair<int,int> nums = SortExpandAndLimitClusters(digs_mod, MAX_NS, MAX_NP);
      // - write appropriate header
      writeFeHeaderSparsified(fedbuffer, bitindex, moduletype, nums.second, nums.first);
      // - write the digis
      std::vector<stackedDigi>::iterator its;
      for(its = digs_mod.begin(); its != digs_mod.end(); its++)
      {
        writeCluster(fedbuffer, bitindex, *its);
      }

    } // end idigi (FE) loop
    // add daq trailer 
    fedbuffer.push_back(*(uint64_t*)FedDaqTrailer_.data());
    return fedbuffer;
  }

  void Phase2TrackerDigiToRaw::writeFeHeaderSparsified(std::vector<uint64_t> & buffer, uint64_t & bitpointer, int modtype, int np, int ns)
  {
    uint8_t  length = 0;
    uint16_t header = ((uint16_t)ns & 0x3F);
    // module type switch
    if (modtype == 1)
    {
      header |= ((uint16_t)np & 0x3F)<<6;
      header |= ((uint16_t)modtype & 0x01)<<12;
      length = 13;
    }
    else 
    {
      header |= ((uint16_t)modtype & 0x01)<<6;
      length = 7;
    }
    write_n_at_m(buffer,length,bitpointer,header);
    bitpointer += length;
  }

  // layer = 0 for inner, 1 for outer (-1 if irrelevant)
  void Phase2TrackerDigiToRaw::writeCluster(std::vector<uint64_t> & buffer, uint64_t & bitpointer, stackedDigi digi)
  {
    if(digi.getModuleType() == 0)
    {
      // 2S module
      writeSCluster(buffer,bitpointer,digi,false);
    } 
    else
    {
      // PS module
      if(digi.getLayer() == LAYER_INNER)
      {
        writePCluster(buffer,bitpointer,digi);
      }
      else
      {
        writeSCluster(buffer,bitpointer,digi,true);   
      }
    }
  }


  void Phase2TrackerDigiToRaw::writeSCluster(std::vector<uint64_t> & buffer, uint64_t & bitpointer, stackedDigi digi, bool threshold)
  {
    int csize = 15;
    uint16_t scluster = (digi.getChipId() & 0x0F) << 11;
    scluster |= (digi.getRawX() & 0xFF) << 3;
    scluster |= ((digi.getSizeX()-1) & 0x07);
    if (threshold) {
      csize += 1;
      scluster <<= 1;
      scluster |= (digi.getThreshold() & 0x01);
    } 
    write_n_at_m(buffer,csize,bitpointer,scluster);
    bitpointer += csize;
    // debug
    #ifdef EDM_ML_DEBUG
    std::ostringstream ss;
    ss << "S chip: " << digi.getChipId() << " digiX: " << digi.getDigiX() << " raw size: " << digi.getSizeX() << " digiY: " << digi.getDigiY() << " Layer: " << digi.getLayer(); 
    LogTrace("Phase2TrackerDigiProducer") << ss.str(); ss.clear(); ss.str("");
    #endif
  }

  void Phase2TrackerDigiToRaw::writePCluster(std::vector<uint64_t> & buffer, uint64_t & bitpointer, stackedDigi digi)
  {
    uint32_t pcluster = (digi.getChipId() & 0x0F) << 14;
    pcluster |= (digi.getRawX() & 0x7F) << 7;
    pcluster |= (digi.getRawY() & 0x0F) << 3;
    pcluster |= ((digi.getSizeX()-1) & 0x07);
    write_n_at_m(buffer,18,bitpointer,pcluster);
    bitpointer += 18;
    // debug 
    #ifdef EDM_ML_DEBUG
    std::ostringstream ss;
    ss << "P chip: " << digi.getChipId() << " digiX: " << digi.getDigiX() << " raw size: " << digi.getSizeX() << " digiY: " << digi.getDigiY(); 
    LogTrace("Phase2TrackerDigiProducer") << ss.str(); ss.clear(); ss.str("");
    #endif
  }
}
