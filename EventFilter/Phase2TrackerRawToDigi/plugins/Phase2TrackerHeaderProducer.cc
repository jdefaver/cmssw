// -*- C++ -*-
//
// Package:    EventFilter/Phase2TrackerRawToDigi/Phase2TrackerHeaderProducer
// Class:      Phase2TrackerHeaderProducer
// 
/**\class Phase2TrackerHeaderProducer Phase2TrackerHeaderProducer.cc EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerHeaderProducer.cc

 Description: Producer for the phase 2 tracker header digi

*/
//
// Original Author:  Jerome De Favereau De Jeneret
//         Created:  Mon, 01 Sep 2014 08:42:31 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"


//
// class declaration
//

class Phase2TrackerHeaderProducer : public edm::EDProducer {
   public:
      explicit Phase2TrackerHeaderProducer(const edm::ParameterSet&);
      ~Phase2TrackerHeaderProducer();

   private:
      edm::EDGetTokenT<FEDRawDataCollection> token_;
      const Phase2TrackerCabling * cabling_;
};

Phase2TrackerHeaderProducer::Phase2TrackerHeaderProducer(const edm::ParameterSet& iConfig)
{
   produces<edm::DetSet<Phase2TrackerHeaderDigi>>("TrackerHeader");
   token_ = consumes<FEDRawDataCollection>(pset.getParameter<edm::InputTag>("ProductLabel"));
}

Phase2TrackerHeaderProducer::~Phase2TrackerHeaderProducer() {}

void Phase2TrackerHeaderProducer::beginRun( edm::Run const& run, edm::EventSetup const& es)
{
  edm::ESHandle<Phase2TrackerCabling> c;
  es.get<Phase2TrackerCablingRcd>().get( c );
  cabling_ = c.product();
}

void Phase2TrackerHeaderProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   // Retrieve FEDRawData collection
   edm::Handle<FEDRawDataCollection> buffers;
   event.getByToken( token_, buffers );
   size_t fedIndex;
   for( fedIndex = Phase2Tracker::FED_ID_MIN; fedIndex < Phase2Tracker::CMS_FED_ID_MAX; ++fedIndex )
   {
     const FEDRawData& fed = buffers->FEDData(fedIndex);
     if(fed.size()!=0)
     {
       // construct buffer
       Phase2Tracker:: Phase2TrackerFEDBuffer * buffer = new Phase2Tracker::Phase2TrackerFEDBuffer(fed.data(),fed.size());
       Phase2TrackerHeaderDigi head_digi = Phase2TrackerHeaderDigi(buffer->trackerHeader());
       // get detid from cabling
       const Phase2TrackerModule mod = cabling_->findFedCh(std::make_pair(fedIndex, ife));
       uint32_t detid = mod.getDetid();
       // store digis
       edm::DetSet<Phase2TrackerHeaderDigi> *header_digi = new edm::DetSet<Phase2TrackerHeaderDigi>(detid);
       header_digi->push_back(header_digi);
       std::auto_ptr< edm::DetSet<Phase2TrackerHeaderDigi> > hdd(header_digi);
       event.put( hdd, "TrackerHeader" );
     }
   }
}

void Phase2TrackerHeaderProducer::beginJob() {}

void Phase2TrackerHeaderProducer::endJob() {}
 
DEFINE_FWK_MODULE(Phase2TrackerHeaderProducer);
