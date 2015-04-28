#include "FWCore/Framework/interface/MakerMacros.h"

#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerDigiProducer.h"
#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerCommissioningDigiProducer.h"
#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerDigiToRawProducer.h"

typedef Phase2Tracker::Phase2TrackerDigiProducer Phase2TrackerDigiProducer;
typedef Phase2Tracker::Phase2TrackerCommissioningDigiProducer Phase2TrackerCommissioningDigiProducer;
typedef Phase2Tracker::Phase2TrackerDigiToRawProducer Phase2TrackerDigiToRawProducer;

DEFINE_FWK_MODULE(Phase2TrackerDigiProducer);
DEFINE_FWK_MODULE(Phase2TrackerCommissioningDigiProducer);
DEFINE_FWK_MODULE(Phase2TrackerDigiToRawProducer);
