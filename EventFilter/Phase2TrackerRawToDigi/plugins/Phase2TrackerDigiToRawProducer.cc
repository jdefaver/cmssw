#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerDigiToRawProducer.h"
#include "DataFormats/Common/interface/DetSet.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/src/fed_header.h"
#include "DataFormats/FEDRawData/src/fed_trailer.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"
#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerFEDHeader.h"
#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerDigiToRaw.h"
#include "EventFilter/Phase2TrackerRawToDigi/interface/utils.h"
#include "CondFormats/DataRecord/interface/Phase2TrackerCablingRcd.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ext/algorithm>

using namespace std;

namespace Phase2Tracker {

  Phase2TrackerDigiToRawProducer::Phase2TrackerDigiToRawProducer( const edm::ParameterSet& pset ) :
    cabling_(0)
  {
    token_ = consumes<edmNew::DetSetVector<SiPixelCluster>>(pset.getParameter<edm::InputTag>("ProductLabel"));
  }
  
  Phase2TrackerDigiToRawProducer::~Phase2TrackerDigiToRawProducer()
  {
  }
  
  void Phase2TrackerDigiToRawProducer::beginJob( )
  {
  }
  
  void Phase2TrackerDigiToRawProducer::beginRun( edm::Run const& run, edm::EventSetup const& es)
  {
    // fetch cabling from event setup
    edm::ESHandle<Phase2TrackerCabling> c;
    es.get<Phase2TrackerCablingRcd>().get( c );
    cabling_ = c.product();
  }
  
  void Phase2TrackerDigiToRawProducer::endJob()
  {
  }
  
  void Phase2TrackerDigiToRawProducer::produce( edm::Event& event, const edm::EventSetup& es)
  {
    edm::Handle< edmNew::DetSetVector<SiPixelCluster> > handle;
    Phase2TrackerDigiToRaw converter;
    converter.buildPayload(event,es,cabling_,handle);
  }
}
