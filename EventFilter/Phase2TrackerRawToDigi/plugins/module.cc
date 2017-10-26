#include "FWCore/Framework/interface/MakerMacros.h"

#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerDigiProducer.h"
#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerDigiProducerTestBeam.h"
#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerCommissioningDigiProducer.h"
#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerHeaderProducer.h"
#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerDigiToRawProducer.h"
#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerDebugProducer.h"
#include "EventFilter/Phase2TrackerRawToDigi/plugins/Phase2TrackerStubProducer.h"

typedef Phase2Tracker::Phase2TrackerDigiProducer Phase2TrackerDigiProducer;
typedef Phase2Tracker::Phase2TrackerDigiProducerTestBeam Phase2TrackerDigiProducerTestBeam;
typedef Phase2Tracker::Phase2TrackerDebugProducer Phase2TrackerDebugProducer;
typedef Phase2Tracker::Phase2TrackerStubProducer Phase2TrackerStubProducer;
typedef Phase2Tracker::Phase2TrackerCommissioningDigiProducer Phase2TrackerCommissioningDigiProducer;
typedef Phase2Tracker::Phase2TrackerHeaderProducer Phase2TrackerHeaderProducer;
typedef Phase2Tracker::Phase2TrackerDigiToRawProducer Phase2TrackerDigiToRawProducer;

DEFINE_FWK_MODULE(Phase2TrackerDigiProducer);
DEFINE_FWK_MODULE(Phase2TrackerDigiProducerTestBeam);
DEFINE_FWK_MODULE(Phase2TrackerDebugProducer);
DEFINE_FWK_MODULE(Phase2TrackerStubProducer);
DEFINE_FWK_MODULE(Phase2TrackerCommissioningDigiProducer);
DEFINE_FWK_MODULE(Phase2TrackerHeaderProducer);
DEFINE_FWK_MODULE(Phase2TrackerDigiToRawProducer);
