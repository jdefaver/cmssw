import FWCore.ParameterSet.Config as cms
import sys

process = cms.Process("RawToDigi")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger = cms.Service("MessageLogger",
        destinations  = cms.untracked.vstring('logtrace' ),
        logtrace      = cms.untracked.PSet( threshold  = cms.untracked.string('DEBUG') ),
        debugModules  = cms.untracked.vstring( 'Phase2TrackerDigiProducer', 'Phase2TrackerFEDBuffer' )
)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1))


process.source = cms.Source("PoolSource",
# use this to read testbeam .dat files
# process.source = cms.Source("NewEventStreamFileReader",
    fileNames = cms.untracked.vstring( 'file:'+sys.argv[-1])
)


# use this to use hand-made testbeam cabling
# process.load('TestbeamCabling_cfi')
process.load('DummyCablingTxt_cfi')
process.load('Configuration.Geometry.GeometryExtended2023simReco_cff')

process.load('EventFilter.Phase2TrackerRawToDigi.Phase2TrackerCommissioningDigiProducer_cfi')
process.load('EventFilter.Phase2TrackerRawToDigi.Phase2TrackerDigiProducer_cfi')

# imported from runthematrix
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

# use these labels instead to run on raw data
# process.Phase2TrackerDigiProducer.ProductLabel = cms.InputTag("rawDataCollector")
# process.Phase2TrackerCommissioningDigiProducer.ProductLabel = cms.InputTag("rawDataCollector")
process.Phase2TrackerDigiProducer.ProductLabel = cms.InputTag("Phase2TrackerDigiToRawProducer")
process.Phase2TrackerCommissioningDigiProducer.ProductLabel = cms.InputTag("Phase2TrackerDigiToRawProducer")


process.out = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string('rawtodigi.root'),
)

# process.p = cms.Path(process.Phase2TrackerDigiProducer*process.Phase2TrackerCommissioningDigiProducer)
process.p = cms.Path(process.Phase2TrackerDigiProducer)

process.e = cms.EndPath(process.out)

# customisation of the process.
# Automatic addition of the customisation function from SLHCUpgradeSimulations.Configuration.combinedCustoms
from SLHCUpgradeSimulations.Configuration.combinedCustoms import cust_2023GReco
#call to customisation function cust_2023GReco imported from SLHCUpgradeSimulations.Configuration.combinedCustoms
process = cust_2023GReco(process)
