import FWCore.ParameterSet.Config as cms



# output block for alcastream Electron
OutALCARECOEcalCalElectron_specific = cms.untracked.vstring(
    'drop *EcalRecHit*_ecalRecHit_*_*',
    'drop *EcalrecHit*_*ecalPreshowerRecHit*_*EcalRecHitsES*_*',
    'drop *EcalRecHit*_reducedEcalRecHitsE*_*_*',
    'keep *EcalRecHit*_alCaIsolatedElectrons_*_*',
    'keep *EcalRecHit*_reducedEcalRecHitsES_alCaRecHitsES_*'
)

OutALCARECOEcalCalElectron_noDrop = cms.PSet(
    # put this if you have a filter
#    SelectEvents = cms.untracked.PSet(
#        SelectEvents = cms.vstring('pathALCARECOEcalCalElectron')
#    ),
    outputCommands = cms.untracked.vstring( 
    'keep *_pfMet_*_*',
    'keep *_kt6PFJetsForRhoCorrection_rho_*',
    'keep *_kt6PFJets_rho_*',
    'keep *_offlinePrimaryVertices*_*_*',
    'keep *BeamSpot_offlineBeamSpot_*_*',
    'keep *_allConversions_*_*',
    'keep *_conversions_*_*',
    'keep *GsfTrack*_*_*_*',
    'keep *_generator_*_*',
    'keep *_addPileupInfo_*_*',
    'keep *_genParticles_*_*',
    'keep recoGsfElectron*_gsfElectron*_*_*',
    'keep recoCaloClusters_*_*_*',
    'keep recoSuperClusters_*_*_*',
    'keep recoPreshowerCluster*_*_*_*',
    'drop reco*Clusters_hfEMClusters_*_*',
    'drop reco*Clusters_pfPhotonTranslator_*_*',
    'drop *_*Unclean*_*_*',
    'drop *_*unclean*_*_*',
    'drop *_*_*Unclean*_*',
    'drop *_*_*unclean*_*',
    #'keep *_*_*_HLT',
    #'keep *_generalTracks_*_*',
    #'keep reco*Track*Extra*_generalTracks_*_*',
    'keep *_alcaElectronTracksReducer_*_*',
    # for the trigger matching
    'keep *_l1extraParticles_*_*',
    'keep L1GlobalTriggerReadoutRecord_gtDigis_*_*',
    'keep *_l1L1GtObjectMap_*_*',
    'keep edmConditionsInEventBlock_conditionsInEdm_*_*',
    'keep edmConditionsInLumiBlock_conditionsInEdm_*_*',
    'keep edmConditionsInRunBlock_conditionsInEdm_*_*',
    'keep *_TriggerResults_*_*',
    'keep *_hltTriggerSummaryAOD_*_HLT',
    # pfisolation
    'keep *_elPFIsoValueCharged03PFIdPFIso_*_*',
    'keep *_elPFIsoValueGamma03PFIdPFIso_*_*',
    'keep *_elPFIsoValueNeutral03PFIdPFIso_*_*',
    'keep *_*_elPFIsoValueCharged03PFIdPFIso_*',
    'keep *_*_elPFIsoValueGamma03PFIdPFIso_*',
    'keep *_*_elPFIsoValueNeutral03PFIdPFIso_*'
    )
)

OutALCARECOEcalCalElectron_noDrop.outputCommands+=OutALCARECOEcalCalElectron_specific


import copy
OutALCARECOEcalCalElectron=copy.deepcopy(OutALCARECOEcalCalElectron_noDrop)
OutALCARECOEcalCalElectron.outputCommands.insert(0, "drop *")
