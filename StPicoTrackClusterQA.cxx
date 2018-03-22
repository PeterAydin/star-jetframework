// ################################################################
// Author:  Joel Mazer for the STAR Collaboration
// Affiliation: Rutgers University
//
// ################################################################
// $Id$

#include "StPicoTrackClusterQA.h"

// root classes
#include <TChain.h>
#include <TClonesArray.h>
#include "TH1F.h"
#include "TH2F.h"
#include <TList.h>
#include <TLorentzVector.h>
#include <TParticle.h>
#include "TFile.h"
#include <THnSparse.h>

// general StRoot classes
#include "StThreeVectorF.hh"

// StRoot classes
#include "StRoot/StPicoDstMaker/StPicoDst.h"
#include "StRoot/StPicoDstMaker/StPicoDstMaker.h"
#include "StRoot/StPicoDstMaker/StPicoArrays.h"
#include "StRoot/StPicoEvent/StPicoEvent.h"
#include "StRoot/StPicoEvent/StPicoTrack.h"
#include "StPicoConstants.h"

// test for clusters:
#include "StMuDSTMaker/COMMON/StMuTrack.h"
// StEmc:
#include "StEmcADCtoEMaker/StBemcData.h"
#include "StEmcADCtoEMaker/StEmcADCtoEMaker.h"
#include "StEmcRawMaker/StBemcRaw.h"
#include "StEmcRawMaker/StBemcTables.h"
#include "StEmcRawMaker/StEmcRawMaker.h"
#include "StEmcRawMaker/defines.h"
#include "tables/St_emcStatus_Table.h"
#include "tables/St_smdStatus_Table.h"
#include "StMuDSTMaker/COMMON/StMuEmcCollection.h"
#include "StEmcClusterCollection.h"
#include "StEmcCollection.h"
#include "StEmcCluster.h"
#include "StEmcPoint.h"
#include "StEmcUtil/geometry/StEmcGeom.h"
//#include "StEmcUtil/others/emcDetectorName.h"
#include "StEmcUtil/projection/StEmcPosition.h"
#include "StEmcRawHit.h"
#include "StEmcModule.h"
#include "StEmcDetector.h"

// extra includes
#include "StPicoEvent/StPicoEmcTrigger.h"
#include "StPicoEvent/StPicoBTowHit.h"
#include "StPicoEvent/StPicoBEmcPidTraits.h"
#include "StJetFrameworkPicoBase.h"

// centrality
#include "StRoot/StRefMultCorr/StRefMultCorr.h"
#include "StRoot/StRefMultCorr/CentralityMaker.h"

// towers
#include "StJetPicoTower.h"

ClassImp(StPicoTrackClusterQA)

//________________________________________________________________________
StPicoTrackClusterQA::StPicoTrackClusterQA() : 
//  StJetFrameworkPicoBase(),
  StMaker(),
  doWriteHistos(kFALSE),
  doUsePrimTracks(kFALSE), 
  fDebugLevel(0),
  fRunFlag(0),       // see StJetFrameworkPicoBase::fRunFlagEnum
  fCentralityDef(4), // see StJetFrameworkPicoBase::fCentralityDefEnum
  fDoEffCorr(kFALSE),
  fEventZVtxMinCut(-40.0), 
  fEventZVtxMaxCut(40.0),
  mOutName(""),
  fTracksName(""),
  fCaloName(""),
  fTrackPtMinCut(0.2),
  fTrackPtMaxCut(20.0),
  fClusterPtMinCut(0.2),
  fClusterPtMaxCut(1000.0),
  fTrackPhiMinCut(0.0),
  fTrackPhiMaxCut(2.0*TMath::Pi()),
  fTrackEtaMinCut(-1.0),
  fTrackEtaMaxCut(1.0),
  fTrackDCAcut(3.0),
  fTracknHitsFit(15),
  fTracknHitsRatio(0.52),
  fTrackEfficiency(1.),
  fGoodTrackCounter(0),
  fCentralityScaled(0.),
  ref16(-99), ref9(-99),
  Bfield(0.0),
  mVertex(0x0),
  zVtx(0.0),
  fRunNumber(0),
  fTriggerEventType(0),
  mGeom(StEmcGeom::instance("bemc")),
  mEmcCol(0),
  mBemcTables(0x0),
  mBemcMatchedTracks(),
  mTowerStatusMode(AcceptAllTowers),
  mTowerEnergyMin(0.2),
  mHadronicCorrFrac(1.0),
  mMuDstMaker(0x0),
  mMuDst(0x0),
  mMuInputEvent(0x0),
  mPicoDstMaker(0x0),
  mPicoDst(0x0),
  mPicoEvent(0x0),
  grefmultCorr(0x0),
  fhnTrackQA(0x0),
  fhnTowerQA(0x0)
{
  // Default constructor.
  for(int i=0; i<7; i++) { fEmcTriggerArr[i] = 0; }
  for(int i=0; i<4801; i++) { 
    fTowerToTriggerTypeHT1[i] = kFALSE;
    fTowerToTriggerTypeHT2[i] = kFALSE;
    fTowerToTriggerTypeHT3[i] = kFALSE; 
  }
}

//________________________________________________________________________
StPicoTrackClusterQA::StPicoTrackClusterQA(const char *name, bool doHistos = kFALSE, const char* outName = "") : 
//  StJetFrameworkPicoBase(name),
  StMaker(name),
  doWriteHistos(doHistos),
  doUsePrimTracks(kFALSE),
  fDebugLevel(0),
  fRunFlag(0),       // see StJetFrameworkPicoBase::fRunFlagEnum
  fCentralityDef(4), // see StJetFrameworkPicoBase::fCentralityDefEnum
  fDoEffCorr(kFALSE),
  fEventZVtxMinCut(-40.0), 
  fEventZVtxMaxCut(40.0),
  mOutName(outName),
  fTracksName("Tracks"),
  fCaloName("Clusters"),
  fTrackPtMinCut(0.2), //0.20
  fTrackPtMaxCut(20.0), 
  fClusterPtMinCut(0.2),
  fClusterPtMaxCut(1000.0),
  fTrackPhiMinCut(0.0),
  fTrackPhiMaxCut(2.0*TMath::Pi()),
  fTrackEtaMinCut(-1.0), 
  fTrackEtaMaxCut(1.0),
  fTrackDCAcut(3.0),
  fTracknHitsFit(15),
  fTracknHitsRatio(0.52),
  fTrackEfficiency(1.),
  fGoodTrackCounter(0),
  fCentralityScaled(0.),
  ref16(-99), ref9(-99),
  Bfield(0.0),
  mVertex(0x0),
  zVtx(0.0),
  fRunNumber(0),
  fTriggerEventType(0),
  mGeom(StEmcGeom::instance("bemc")),
  mEmcCol(0),
  mBemcTables(0x0),
  mBemcMatchedTracks(),
  mTowerStatusMode(AcceptAllTowers),
  mTowerEnergyMin(0.2),
  mHadronicCorrFrac(1.0),
  mMuDstMaker(0x0),
  mMuDst(0x0),
  mMuInputEvent(0x0),
  mPicoDstMaker(0x0),
  mPicoDst(0x0),
  mPicoEvent(0x0),
  grefmultCorr(0x0),
  fhnTrackQA(0x0),
  fhnTowerQA(0x0)
{
  // Standard constructor.
  if (!name) return;
  SetName(name);

  for(int i=0; i<7; i++) { fEmcTriggerArr[i] = 0; }
  for(int i=0; i<4801; i++) { 
    fTowerToTriggerTypeHT1[i] = kFALSE;
    fTowerToTriggerTypeHT2[i] = kFALSE;
    fTowerToTriggerTypeHT3[i] = kFALSE;
  }
}

//________________________________________________________________________
StPicoTrackClusterQA::~StPicoTrackClusterQA()
{
  // free up histogram objects if they exist

  // Destructor
  if(fHistNTrackvsPt)   delete fHistNTrackvsPt;
  if(fHistNTrackvsPhi)  delete fHistNTrackvsPhi;
  if(fHistNTrackvsEta)  delete fHistNTrackvsEta;
  if(fHistNTrackvsPhivsEta) delete fHistNTrackvsPhivsEta;
  if(fHistNTowervsE)    delete fHistNTowervsE;
  if(fHistNTowervsPhi)  delete fHistNTowervsPhi;
  if(fHistNTowervsEta)  delete fHistNTowervsEta;
  if(fHistNTowervsPhivsEta) delete fHistNTowervsPhivsEta;

  delete fHistEventSelectionQA;
  delete fHistEventSelectionQAafterCuts;
  delete hTriggerIds;
  delete hEmcTriggers;

  delete fhnTrackQA;
  delete fhnTowerQA;
}

//-----------------------------------------------------------------------------
Int_t StPicoTrackClusterQA::Init() {
  DeclareHistograms();

  // test placement
  mBemcTables = new StBemcTables();

  // may not need, used for old RUNS
  // StRefMultCorr* getgRefMultCorr() ; // For grefmult //Run14 AuAu200GeV
  if(fRunFlag == StJetFrameworkPicoBase::Run14_AuAu200) { grefmultCorr = CentralityMaker::instance()->getgRefMultCorr(); }
  if(fRunFlag == StJetFrameworkPicoBase::Run16_AuAu200) {
    if(fCentralityDef == StJetFrameworkPicoBase::kgrefmult) { grefmultCorr = CentralityMaker::instance()->getgRefMultCorr(); }
    if(fCentralityDef == StJetFrameworkPicoBase::kgrefmult_P16id) { grefmultCorr = CentralityMaker::instance()->getgRefMultCorr_P16id(); }
    if(fCentralityDef == StJetFrameworkPicoBase::kgrefmult_VpdMBnoVtx) { grefmultCorr = CentralityMaker::instance()->getgRefMultCorr_VpdMBnoVtx(); }
    if(fCentralityDef == StJetFrameworkPicoBase::kgrefmult_VpdMB30) { grefmultCorr = CentralityMaker::instance()->getgRefMultCorr_VpdMB30(); }
  }

  return kStOK;
}

//-----------------------------------------------------------------------------
Int_t StPicoTrackClusterQA::Finish() {
  //  Summarize the run.
  cout << "StPicoTrackClusterQA::Finish()\n";

  if(doWriteHistos && mOutName!="") {
    TFile *fout = new TFile(mOutName.Data(), "RECREATE");
    fout->cd();
    fout->mkdir(GetName());
    fout->cd(GetName());

    WriteHistograms();
    fout->cd();
    fout->Write();
    fout->Close();
  }

  cout<<"End of StPicoTrackClusterQA::Finish"<<endl;

  return kStOK;
}

//________________________________________________________________________
void StPicoTrackClusterQA::DeclareHistograms() {
    // declare histograms
    double pi = 1.0*TMath::Pi();

    // track histograms
    fHistNTrackvsPt = new TH1F("fHistNTrackvsPt", "Ntracks vs p_{T}", 100, 0., 20.);
    fHistNTrackvsPhi = new TH1F("fHistNTrackvsPhi", "Ntracks vs #phi", 72, 0., 2*pi);
    fHistNTrackvsEta = new TH1F("fHistNTrackvsEta", "Ntracks vs #eta", 40, -1.0, 1.0);
    fHistNTrackvsPhivsEta = new TH2F("fHistNTrackvsPhivsEta", "Ntrackvs #phi vs #eta", 144, 0, 2*pi, 20, -1.0, 1.0);

    // Tower histograms
    fHistNTowervsE = new TH1F("fHistNTowervsE", "Ntowers vs energy", 100, 0., 20.0);
    fHistNTowervsPhi = new TH1F("fHistNTowervsPhi", "Ntowers vs #phi", 144, 0., 2*pi);
    fHistNTowervsEta = new TH1F("fHistNTowervsEta", "Ntowers vs #eta", 40, -1.0, 1.0);
    fHistNTowervsPhivsEta = new TH2F("fHistNTowervsPhivsEta", "Ntowers vs #phi vs #eta", 144, 0, 2*pi, 20, -1.0, 1.0);

    // Event Selection QA histo
    fHistEventSelectionQA = new TH1F("fHistEventSelectionQA", "Trigger Selection Counter", 20, 0.5, 20.5);
    fHistEventSelectionQAafterCuts = new TH1F("fHistEventSelectionQAafterCuts", "Trigger Selection Counter after Cuts", 20, 0.5, 20.5);
    hTriggerIds = new TH1F("hTriggerIds", "Trigger Id distribution", 100, 0.5, 100.5);
    hEmcTriggers = new TH1F("hEmcTriggers", "Emcal Trigger counter", 10, 0.5, 10.5);

    // set up track and tower sparse
    UInt_t bitcodeTrack = 0; // bit coded, see GetDimParams() below
    bitcodeTrack = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4; 
    fhnTrackQA = NewTHnSparseFTracks("fhnTrackQA", bitcodeTrack);

    // set up track and tower sparse
    UInt_t bitcodeTower = 0; // bit coded, see GetDimParams() below
    bitcodeTower = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4;                      
    fhnTowerQA = NewTHnSparseFTowers("fhnTowerQA", bitcodeTower);

    // Switch on Sumw2 for all histos - (except profiles)
    SetSumw2();
}

//________________________________________________________________________

void StPicoTrackClusterQA::WriteHistograms() {
  // write histograms
  fHistNTrackvsPt->Write();
  fHistNTrackvsPhi->Write();
  fHistNTrackvsEta->Write();
  fHistNTrackvsPhivsEta->Write();
  fHistNTowervsE->Write();
  fHistNTowervsPhi->Write();
  fHistNTowervsEta->Write();
  fHistNTowervsPhivsEta->Write();

  // QA histos
  fHistEventSelectionQA->Write();
  fHistEventSelectionQAafterCuts->Write();
  hTriggerIds->Write();
  hEmcTriggers->Write();

  // sparses
  fhnTrackQA->Write();
  fhnTowerQA->Write();

}

//-----------------------------------------------------------------------------
void StPicoTrackClusterQA::Clear(Option_t *opt) {
}

//________________________________________________________________________
int StPicoTrackClusterQA::Make()
{  // Main loop, called for each event.
  bool fHaveEmcTrigger = kFALSE;
  bool fHaveMBevent = kFALSE;
  fGoodTrackCounter = 0;

  // ===========================================================================
/*
  mMuDstMaker = (StMuDstMaker*)GetMaker("muDst");
  if(!mMuDstMaker) {
    LOG_WARN << " No MuDstMaker! Skip! " << endm;
    return kStWarn;
  }

  // get muDst 
  mMuDst = (StMuDst*) GetInputDS("MuDst");         
  if(!mMuDst) {
    LOG_WARN << " No muDst! Skip! " << endm;
    return kStWarn;
  }

  // get EmcCollection
  mEmcCol = (StEmcCollection*)mMuDst->emcCollection();
  if(!mEmcCol) return kStWarn;
*/
  // ===========================================================================

  // Get PicoDstMaker
  mPicoDstMaker = (StPicoDstMaker*)GetMaker("picoDst");
  if(!mPicoDstMaker) {
    LOG_WARN << " No PicoDstMaker! Skip! " << endm;
    return kStWarn;
  }

  // construct PicoDst object from maker
  mPicoDst = mPicoDstMaker->picoDst();
  if(!mPicoDst) {
    LOG_WARN << " No PicoDst! Skip! " << endm;
    return kStWarn;
  }

  // create pointer to PicoEvent
  mPicoEvent = mPicoDst->event();
  if(!mPicoEvent) {
    LOG_WARN << " No PicoEvent! Skip! " << endm;
    return kStWarn;
  }

  // get event B (magnetic) field
  Bfield = mPicoEvent->bField();

  // get vertex 3-vector and declare variables
  mVertex = mPicoEvent->primaryVertex();
  zVtx = mVertex.z();

  // Z-vertex cut - per the Aj analysis (-40, 40)
  if((zVtx < fEventZVtxMinCut) || (zVtx > fEventZVtxMaxCut)) return kStOk;

  // ============================ CENTRALITY ============================== //
  // 10 14 21 29 40 54 71 92 116 145 179 218 263 315 373 441  // RUN 14 AuAu binning
  int RunId = mPicoEvent->runId();
  float fBBCCoincidenceRate = mPicoEvent->BBCx();
  int grefMult = mPicoEvent->grefMult();
  grefmultCorr->init(RunId);
  grefmultCorr->initEvent(grefMult, zVtx, fBBCCoincidenceRate);
  grefmultCorr->getRefMultCorr(grefMult, zVtx, fBBCCoincidenceRate, 2);
  int cent16 = grefmultCorr->getCentralityBin16();
  if(cent16 == -1) return kStOk; // maybe kStOk; - this is for lowest multiplicity events 80%+ centrality, cut on them
  int centbin = GetCentBin(cent16, 16);
  fCentralityScaled = centbin*5.0;

  // cut on centrality for analysis before doing anything
  if(fRequireCentSelection) { if(!SelectAnalysisCentralityBin(centbin, fCentralitySelectionCut)) return kStOk; }

  // ========================= Trigger Info =============================== //
  // fill Event Trigger QA
  FillEventTriggerQA(fHistEventSelectionQA);

  // looking at the EMCal triggers - used for QA and deciding on HT triggers
  // trigger information:  // cout<<"istrigger = "<<mPicoEvent->isTrigger(450021)<<endl; // NEW
  FillEmcTriggersHist(hEmcTriggers);

  // Run16 triggers:
  int arrBHT1[] = {7, 15, 520201, 520211, 520221, 520231, 520241, 520251, 520261, 520605, 520615, 520625, 520635, 520645, 520655, 550201, 560201, 560202, 530201, 540201};
  int arrBHT2[] = {4, 16, 17, 530202, 540203};
  int arrBHT3[] = {17, 520203, 530213};
  int arrMB[] = {520021};
  int arrMB5[] = {1, 43, 45, 520001, 520002, 520003, 520011, 520012, 520013, 520021, 520022, 520023, 520031, 520033, 520041, 520042, 520043, 520051, 520822, 520832, 520842, 570702};
  int arrMB10[] = {7, 8, 56, 520007, 520017, 520027, 520037, 520201, 520211, 520221, 520231, 520241, 520251, 520261, 520601, 520611, 520621, 520631, 520641};

  // get trigger IDs from PicoEvent class and loop over them
  vector<unsigned int> mytriggers = mPicoEvent->triggerIds();
  if(fDebugLevel == kDebugEmcTrigger) cout<<"EventTriggers: n = "<<mytriggers.size()<<"  ";
  for(unsigned int i=0; i<mytriggers.size(); i++) {
    if(fDebugLevel == kDebugEmcTrigger) cout<<"i = "<<i<<": "<<mytriggers[i] << ", ";
    if((fRunFlag == StJetFrameworkPicoBase::Run14_AuAu200) && (mytriggers[i] == 450014)) { fHaveMBevent = kTRUE; }
    // FIXME Hard-coded for now
    if((fRunFlag == StJetFrameworkPicoBase::Run16_AuAu200) && (DoComparison(arrBHT1, sizeof(arrBHT1)/sizeof(*arrBHT1)))) { fHaveEmcTrigger = kTRUE; }
    if((fRunFlag == StJetFrameworkPicoBase::Run16_AuAu200) && (DoComparison(arrMB5, sizeof(arrMB5)/sizeof(*arrMB5)))) { fHaveMBevent = kTRUE; }
  }

  if(fDebugLevel == kDebugEmcTrigger) cout<<endl;
  // ======================== end of Triggers ============================= //

  // Event QA print-out
  // printing available information from the PicoDst objects
/*
  StPicoTrack* t = mPicoDst->track(1);
  if(t) t->Print();
  StPicoEmcTrigger *emcTrig = mPicoDst->emcTrigger(0);
  if(emcTrig) emcTrig->Print();
  StPicoMtdTrigger *mtdTrig = mPicoDst->mtdTrigger(0);
  if(mtdTrig) mtdTrig->Print();
  StPicoBTowHit *btowhit = mPicoDst->btowHit(0); 
  if(btowhit) btowhit->Print();
  StPicoBTofHit *btofhit = mPicoDst->btofHit(0);
  if(btofhit) btofhit->Print();
  //StPicoMtdHit *mtdhit = mPicoDst->mtdHit(0);
  //mtdhit->Print();
  StPicoBEmcPidTraits *emcpid = mPicoDst->bemcPidTraits(0); // OLD NAME (StPicoEmcPidTraits, emcPidTraits) now its StPicoBEmcPidTraits
  if(emcpid) emcpid->Print();
  StPicoBTofPidTraits *tofpid = mPicoDst->btofPidTraits(0);
  if(tofpid) tofpid->Print();
  StPicoMtdPidTraits *mtdpid = mPicoDst->mtdPidTraits(0);
  if(mtdpid) mtdpid->Print();
*/

  // Run - Trigger Selection to process jets from
  if((fRunFlag == StJetFrameworkPicoBase::Run14_AuAu200) && (!fEmcTriggerArr[fTriggerEventType])) return kStOK;
  if((fRunFlag == StJetFrameworkPicoBase::Run16_AuAu200) && (fHaveEmcTrigger)) return kStOK; //FIXME//

  int nTracks = mPicoDst->numberOfTracks();
  int nTrigs = mPicoDst->numberOfEmcTriggers();
  int nBTowHits = mPicoDst->numberOfBTOWHits();
  int nBEmcPidTraits = mPicoDst->numberOfBEmcPidTraits();

  cout<<"nTracks = "<<nTracks<<"  nTrigs = "<<nTrigs<<"  nBTowHits = "<<nBTowHits<<"  nBEmcPidTraits = "<<nBEmcPidTraits<<endl;
  //cout<<"highTowerThreshold 0 = "<<mPicoEvent->highTowerThreshold(0)<<endl;
  //cout<<"highTowerThreshold 1 = "<<mPicoEvent->highTowerThreshold(1)<<endl;
  //cout<<"highTowerThreshold 2 = "<<mPicoEvent->highTowerThreshold(2)<<endl;
  //cout<<"highTowerThreshold 3 = "<<mPicoEvent->highTowerThreshold(3)<<endl;

  // Event / object PRINT INFO!!
  //mPicoDst->printTracks();
  //mPicoDst->printTriggers();
  ////mPicoDst->printBTOWHits();
  ////mPicoDst->printBEmcPidTraits();

  // function for track and tower QA
  RunQA();
  RunTowerTest();

  // fill Event QA after cuts
  //FillEventTriggerQA(fHistEventSelectionQAafterCuts);

  return kStOK;
}

//________________________________________________________________________
void StPicoTrackClusterQA::RunQA()
{
  // assume neutral pion mass
  double pi = 1.0*TMath::Pi();
  double pi0mass = Pico::mMass[0]; // GeV

  // track variables
  unsigned int ntracks = mPicoDst->numberOfTracks();

  // loop over ALL tracks in PicoDst 
  for(unsigned short iTracks = 0; iTracks < ntracks; iTracks++){
    StPicoTrack* trk = (StPicoTrack*)mPicoDst->track(iTracks);
    if(!trk){ continue; }

    // TODO - double check this, was commenting out for test before (Feb21, 2018)
    // acceptance and kinematic quality cuts
    if(!AcceptTrack(trk, Bfield, mVertex)) { continue; }

    // primary track switch
    // get momentum vector of track - global or primary track
    StThreeVectorF mTrkMom;
    if(doUsePrimTracks) {
      // get primary track vector
      mTrkMom = trk->pMom();
    } else {
      // get global track vector
      mTrkMom = trk->gMom(mVertex, Bfield);
    }

    // track variables
    double pt = mTrkMom.perp();
    double phi = mTrkMom.phi();
    double eta = mTrkMom.pseudoRapidity();
    double p = mTrkMom.mag();
    double energy = 1.0*TMath::Sqrt(p*p + pi0mass*pi0mass);
    short charge = trk->charge();         
    int bemcIndex = trk->bemcPidTraitsIndex();

    // shift track phi (0, 2*pi)
    if(phi<0)    phi += 2*pi;
    if(phi>2*pi) phi -= 2*pi;

    if(fDebugLevel == 8) cout<<"iTracks = "<<iTracks<<"  p = "<<pt<<"  charge = "<<charge<<"  eta = "<<eta<<"  phi = "<<phi;
    if(fDebugLevel == 8) cout<<"  nHitsFit = "<<trk->nHitsFit()<<"  BEmc Index = "<<bemcIndex<<endl;

    // fill some QA histograms
    fHistNTrackvsPt->Fill(pt);
    fHistNTrackvsPhi->Fill(phi);
    fHistNTrackvsEta->Fill(eta);
    fHistNTrackvsPhivsEta->Fill(phi, eta);

    // fill track sparse
    Double_t trackEntries[5] = {fCentralityScaled, pt, eta, phi, zVtx};
    Double_t trefficiency = 1.0;
    fhnTrackQA->Fill(trackEntries, 1.0/trefficiency);

    fGoodTrackCounter++;
  } // track loop

  // looping over clusters - STAR: matching already done
  // get # of clusters and set variables
  unsigned int nclus = mPicoDst->numberOfBEmcPidTraits();
  StThreeVectorF  towPosition, clusPosition;
  StEmcPosition *mPosition = new StEmcPosition();
  StEmcPosition *mPosition2 = new StEmcPosition();

  // print EMCal cluster info
  if(fDebugLevel == 7) mPicoDst->printBEmcPidTraits();

  // loop over ALL clusters in PicoDst and add to jet //TODO
  for(unsigned short iClus = 0; iClus < nclus; iClus++){
    StPicoBEmcPidTraits* cluster = mPicoDst->bemcPidTraits(iClus);
    if(!cluster){ continue; }

    // print index of associated track in the event (debug = 2)
    if(fDebugLevel == 8) cout<<"iClus = "<<iClus<<"  trackIndex = "<<cluster->trackIndex()<<"  nclus = "<<nclus<<endl;

    // ===========================================================================
    // use StEmcDetector to get position information
    // need mMuDst in order to get mEmcCol which is an EMC collection
    //StEmcDetector* detector;
    //detector=mEmcCol->detector(kBarrelEmcTowerId);
    //if(!detector) cout<<"don't have detector object"<<endl;
    // ===========================================================================

    // cluster and tower ID
    // ID's are calculated as such:
    // mBtowId       = (ntow[0] <= 0 || ntow[0] > 4800) ? -1 : (Short_t)ntow[0];
    // mBtowId23 = (ntow[1] < 0 || ntow[1] >= 9 || ntow[2] < 0 || ntow[2] >= 9) ? -1 : (Char_t)(ntow[1] * 10 + ntow[2]);
    int clusID = cluster->bemcId();  // index in bemc point array
    int towID = cluster->btowId();   // projected tower Id: 1 - 4800
    int towID2 = cluster->btowId2(); // emc 2nd and 3rd closest tower local id  ( 2nd X 10 + 3rd), each id 0-8
    int towID3 = cluster->btowId3(); // emc 2nd and 3rd closest tower local id  ( 2nd X 10 + 3rd), each id 0-8
    if(towID < 0) continue;

/*
    // Feb21, 2018: this will work need to change object name - weird error conflicting with name in StJetMakerTask
    // get tower location - from ID
    float towerEta, towerPhi;    // need to be floats
    mGeom->getEtaPhi(towID,towerEta,towerPhi);
    if(towerPhi < 0)    towerPhi += 2*pi;
    if(towerPhi > 2*pi) towerPhi -= 2*pi;
    if(fDebugLevel == 8) {
      cout<<"towID1 = "<<towID<<"  towID2 = "<<towID2<<"  towID3 = "<<towID3;
      cout<<"   towerEta = "<<towerEta<<"  towerPhi = "<<towerPhi<<endl;
    }
*/

    // cluster and tower position - from vertex and ID
    towPosition = mPosition->getPosFromVertex(mVertex, towID);
    clusPosition = mPosition2->getPosFromVertex(mVertex, clusID);

    // index of associated track in the event
    int trackIndex = cluster->trackIndex();

    // get track corresponding to EMC pidTraits in question from its index
    StPicoTrack* trk = (StPicoTrack*)mPicoDst->track(trackIndex);
    StMuTrack* trkMu = (StMuTrack*)mPicoDst->track(trackIndex);
    if(!trk) continue;
    //if(doUsePrimTracks) { if(!(trk->isPrimary())) continue; } // check if primary 

    if(fDebugLevel == 8) cout<<"cluster bemcId = "<<cluster->bemcId()<<"  cluster btowId = "<<cluster->btowId();
    if(fDebugLevel == 8) cout<<"  cluster trackIndex = "<<cluster->trackIndex()<<"  trkId = "<<trk->id()<<endl;

    StThreeVectorD position, momentum;
    double kilogauss = 1000;
    double tesla = 0.00001;
    double magneticField = Bfield * kilogauss  / tesla; // in Tesla
    bool ok = kFALSE;
    if(mPosition) { 
      ok = mPosition->projTrack(&position, &momentum, trkMu, (Double_t)Bfield); 
    }

    // get position vector from projection
    // get eta, phi and z-vtx from position of track projection
    double eta = position.pseudoRapidity(); 
    double phi = position.phi();
    double z   = position.z();
    double theta = 2*TMath::ATan(exp(-1.0*eta));

    // TEST comparing track position with cluster and tower
    double towPhi = towPosition.phi();
    double towEta = towPosition.pseudoRapidity();
    double pmatchPhi = trk->pMom().phi();
    double gmatchPhi = trk->gMom().phi();
    double pmatchEta = trk->pMom().pseudoRapidity();
    double gmatchEta = trk->gMom().pseudoRapidity();
    double clusPhi = clusPosition.phi();
    double clusEta = clusPosition.pseudoRapidity();
    if(towPhi < 0)    towPhi += 2*pi;
    if(towPhi > 2*pi) towPhi -= 2*pi;
    if(gmatchPhi < 0)    gmatchPhi += 2*pi;
    if(gmatchPhi > 2*pi) gmatchPhi -= 2*pi;
    if(clusPhi < 0)    clusPhi += 2*pi;
    if(clusPhi > 2*pi) clusPhi -= 2*pi;

/*
    // print some tower / cluster / track debug info
    //if(fDebugLevel == 8) {
    if(fDebugLevel == 8 && cluster->bemcE0() > 1.0) {
      cout<<"tID = "<<towID<<" cID = "<<clusID<<" iTrk = "<<trackIndex<<" TrkID = "<<trk->id();
      cout<<" tEta = "<<towEta<<" tPhi = "<<towPhi<<"  towE = "<<cluster->bemcE0();
      cout<<" mTowE = "<<cluster->btowE()<<"  mTowE2 = "<<cluster->btowE2()<<"  mTowE3 = "<<cluster->btowE3()<<endl;
      cout<<"Track: -> trkPhi = "<<gmatchPhi<<" trkEta = "<<gmatchEta<<"  trkP = "<<trk->gMom().mag()<<endl;
      cout<<"Project trk -> eta = "<<eta<<"  phi = "<<phi<<"  z = "<<z;
      cout<<"  etaDist = "<<cluster->btowEtaDist()<<"  phiDist = "<<cluster->btowPhiDist()<<endl;
      cout<<"Cluster: cID = "<<clusID<<"  iClus = "<<iClus<<"  cEta = "<<clusEta<<"  cPhi = "<<clusPhi<<"  clusE = "<<cluster->bemcE()<<endl<<endl;
    } // debug statement
*/

    // fill tower sparse
    Double_t towerEntries[5] = {fCentralityScaled, cluster->bemcE0(), towEta, towPhi, zVtx};
    fhnTowerQA->Fill(towerEntries);

  } // cluster loop

}  // track/cluster QA

void StPicoTrackClusterQA::SetSumw2() {
  // Set sum weights
  fHistNTrackvsPt->Sumw2();
  fHistNTrackvsPhi->Sumw2();
  fHistNTrackvsEta->Sumw2();
  fHistNTrackvsPhivsEta->Sumw2();

  fHistNTowervsE->Sumw2();
  fHistNTowervsPhi->Sumw2();
  fHistNTowervsEta->Sumw2();
  fHistNTowervsPhivsEta->Sumw2();

  // QA histos
  fHistEventSelectionQA->Sumw2();
  fHistEventSelectionQAafterCuts->Sumw2();
  hTriggerIds->Sumw2();
  hEmcTriggers->Sumw2();

  fhnTrackQA->Sumw2();
  fhnTowerQA->Sumw2();
}

//________________________________________________________________________
Int_t StPicoTrackClusterQA::GetCentBin(Int_t cent, Int_t nBin) const
{  // Get centrality bin.
  Int_t centbin = -1;

  if(nBin == 16) {
    centbin = nBin - 1 - cent;
  }
  if(nBin == 9) {
    centbin = nBin - 1 - cent;
  }

  return centbin;
}

//__________________________________________________________________________________________
Bool_t StPicoTrackClusterQA::SelectAnalysisCentralityBin(Int_t centbin, Int_t fCentralitySelectionCut) {
  // this function is written to cut on centrality in a task for a given range
  // STAR centrality is written in re-verse binning (0,15) where lowest bin is highest centrality
  // -- this is a very poor approach
  // -- Solution, Use:  Int_t StJetFrameworkPicoBase::GetCentBin(Int_t cent, Int_t nBin) const
  //    in order remap the centrality to a more appropriate binning scheme
  //    (0, 15) where 0 will correspond to the 0-5% bin
  // -- NOTE: Use the 16 bin scheme instead of 9, its more flexible
  // -- Example usage:
  //    Int_t cent16 = grefmultCorr->getCentralityBin16();
  //    Int_t centbin = GetCentBin(cent16, 16);
  //    if(!SelectAnalysisCentralityBin(centbin, StJetFrameworkPicoBase::kCent3050)) return kStWarn; (or StOk to suppress warnings)
  //
  // other bins can be added if needed...

  bool doAnalysis;
  doAnalysis = kFALSE; // set false by default, to make sure user chooses an available bin

  // switch on bin selection
  switch(fCentralitySelectionCut) {
    case StJetFrameworkPicoBase::kCent010 :  // 0-10%
      if((centbin>-1) && (centbin<2)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent020 :  // 0-20%
      if((centbin>-1) && (centbin<4)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent1020 : // 10-20%
      if((centbin>1) && (centbin<4)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent1030 : // 10-30%
      if((centbin>1) && (centbin<6)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent1040 : // 10-40%
      if((centbin>1) && (centbin<8)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent2030 : // 20-30%
      if((centbin>3) && (centbin<6)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent2040 : // 20-40%
      if((centbin>3) && (centbin<8)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent2050 : // 20-50%
      if((centbin>3) && (centbin<10)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent2060 : // 20-60%
      if((centbin>3) && (centbin<12)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent3050 : // 30-50%
      if((centbin>5) && (centbin<10)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent3060 : // 30-60%
      if((centbin>5) && (centbin<12)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent4060 : // 40-60%
      if((centbin>7) && (centbin<12)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent4070 : // 40-70%
      if((centbin>7) && (centbin<14)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent4080 : // 40-80%
      if((centbin>7) && (centbin<16)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent5080 : // 50-80%
      if((centbin>9) && (centbin<16)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    case StJetFrameworkPicoBase::kCent6080 : // 60-80%
      if((centbin>11) && (centbin<16)) { doAnalysis = kTRUE; }
      else { doAnalysis = kFALSE; }
      break;

    default : // wrong entry
      doAnalysis = kFALSE;

  }

  return doAnalysis;
}

//________________________________________________________________________
Bool_t StPicoTrackClusterQA::AcceptTrack(StPicoTrack *trk, Float_t B, StThreeVectorF Vert) {
  // constants: assume neutral pion mass
  double pi0mass = Pico::mMass[0]; // GeV
  double pi = 1.0*TMath::Pi();

  // primary track switch
  // get momentum vector of track - global or primary track
  StThreeVectorF mTrkMom;
  if(doUsePrimTracks) {
    if(!(trk->isPrimary())) return kFALSE; // check if primary

    // get primary track vector
    mTrkMom = trk->pMom();
  } else {
    // get global track vector
    mTrkMom = trk->gMom(Vert, B);
  }

  // track variables
  double pt = mTrkMom.perp();
  double phi = mTrkMom.phi();
  double eta = mTrkMom.pseudoRapidity();
  //double px = mTrkMom.x();
  //double py = mTrkMom.y();
  //double pz = mTrkMom.z();
  //double p = mTrkMom.mag();
  //double energy = 1.0*TMath::Sqrt(p*p + pi0mass*pi0mass);
  //short charge = trk->charge();
  double dca = (trk->dcaPoint() - mPicoEvent->primaryVertex()).mag();
  int nHitsFit = trk->nHitsFit();
  int nHitsMax = trk->nHitsMax();
  double nHitsRatio = 1.0*nHitsFit/nHitsMax;

  // do pt cut here to accommadate either type of track
  if(doUsePrimTracks) { // primary  track
    if(pt < fTrackPtMinCut) return kFALSE;
  } else { // global track
    if(pt < fTrackPtMinCut) return kFALSE;
  }

  // track pt, eta, phi cuts
  if(pt > fTrackPtMaxCut) return kFALSE; // 20.0 STAR, 100.0 ALICE
  if((eta < fTrackEtaMinCut) || (eta > fTrackEtaMaxCut)) return kFALSE;
  if(phi < 0)    phi += 2*pi;
  if(phi > 2*pi) phi -= 2*pi;
  if((phi < fTrackPhiMinCut) || (phi > fTrackPhiMaxCut)) return kFALSE;

  // additional quality cuts for tracks
  if(dca > fTrackDCAcut) return kFALSE;
  if(nHitsFit < fTracknHitsFit) return kFALSE;
  if(nHitsRatio < fTracknHitsRatio) return kFALSE;

  // passed all above cuts - keep track and fill input vector to fastjet
  return kTRUE;
}

//_________________________________________________________________________
TH1* StPicoTrackClusterQA::FillEmcTriggersHist(TH1* h) {
  // zero out trigger array and get number of Emcal Triggers
  for(int i = 0; i < 7; i++) { fEmcTriggerArr[i] = 0; }
  int nEmcTrigger = mPicoDst->numberOfEmcTriggers();
  //if(fDebugLevel == kDebugEmcTrigger) { cout<<"nEmcTrigger = "<<nEmcTrigger<<endl; }

  // set kAny true to use of 'all' triggers
  fEmcTriggerArr[StJetFrameworkPicoBase::kAny] = 1;  // always TRUE, so can select on all event (when needed/wanted) 

  // tower - HT trigger types array
  // zero these out - so they are refreshed for each event
  for(int i = 0; i < 4801; i++) {
    fTowerToTriggerTypeHT1[i] = kFALSE;
    fTowerToTriggerTypeHT2[i] = kFALSE;
    fTowerToTriggerTypeHT3[i] = kFALSE;
  }

  // loop over valid EmcalTriggers
  for(int i = 0; i < nEmcTrigger; i++) {
    StPicoEmcTrigger *emcTrig = mPicoDst->emcTrigger(i);
    if(!emcTrig) continue;

    // emc trigger parameters
    int emcTrigID = emcTrig->id();
    //fTowerToTriggerType[i] = -1;

    // check if i'th trigger fired HT triggers by meeting threshold
    bool isHT0 = emcTrig->isHT0();
    bool isHT1 = emcTrig->isHT1();
    bool isHT2 = emcTrig->isHT2();
    bool isHT3 = emcTrig->isHT3();
    if(isHT1) fTowerToTriggerTypeHT1[emcTrigID] = kTRUE;
    if(isHT2) fTowerToTriggerTypeHT2[emcTrigID] = kTRUE;
    if(isHT3) fTowerToTriggerTypeHT3[emcTrigID] = kTRUE;

    // print some EMCal Trigger info
    if(fDebugLevel == kDebugEmcTrigger) {
      cout<<"i = "<<i<<"  id = "<<emcTrigID<<"  flag = "<<emcTrig->flag()<<"  adc = "<<emcTrig->adc();
      cout<<"  isHT0: "<<isHT0<<"  isHT1: "<<isHT1<<"  isHT2: "<<isHT2<<"  isHT3: "<<isHT3;
      cout<<"  isJP0: "<<emcTrig->isJP0()<<"  isJP1: "<<emcTrig->isJP1()<<"  isJP2: "<<emcTrig->isJP2()<<endl;
    }

    // fill for valid triggers
    if(isHT0) { h->Fill(1); fEmcTriggerArr[StJetFrameworkPicoBase::kIsHT0] = 1; }
    if(isHT1) { h->Fill(2); fEmcTriggerArr[StJetFrameworkPicoBase::kIsHT1] = 1; }
    if(isHT2) { h->Fill(3); fEmcTriggerArr[StJetFrameworkPicoBase::kIsHT2] = 1; }
    if(isHT3) { h->Fill(4); fEmcTriggerArr[StJetFrameworkPicoBase::kIsHT3] = 1; }
    if(emcTrig->isJP0()) { h->Fill(5); fEmcTriggerArr[StJetFrameworkPicoBase::kIsJP0] = 1; }
    if(emcTrig->isJP1()) { h->Fill(6); fEmcTriggerArr[StJetFrameworkPicoBase::kIsJP1] = 1; }
    if(emcTrig->isJP2()) { h->Fill(7); fEmcTriggerArr[StJetFrameworkPicoBase::kIsJP2] = 1; }
  }
  // kAny trigger - filled once per event
  h->Fill(10);

  h->GetXaxis()->SetBinLabel(1, "HT0");
  h->GetXaxis()->SetBinLabel(2, "HT1");
  h->GetXaxis()->SetBinLabel(3, "HT2");
  h->GetXaxis()->SetBinLabel(4, "HT3");
  h->GetXaxis()->SetBinLabel(5, "JP0");
  h->GetXaxis()->SetBinLabel(6, "JP1");
  h->GetXaxis()->SetBinLabel(7, "JP2");
  h->GetXaxis()->SetBinLabel(10, "Any");

  // set x-axis labels vertically
  h->LabelsOption("v");
  //h->LabelsDeflate("X");

  return h;
}

//_____________________________________________________________________________
// Trigger QA histogram, label bins 
TH1* StPicoTrackClusterQA::FillEventTriggerQA(TH1* h) {
  // check and fill a Event Selection QA histogram for different trigger selections after cuts

  // Run14 AuAu 200 GeV
  if(fRunFlag == StJetFrameworkPicoBase::Run14_AuAu200) {
    int arrBHT1[] = {450201, 450211, 460201};
    int arrBHT2[] = {450202, 450212};
    int arrBHT3[] = {460203, 6, 10, 14, 31, 450213};
    int arrMB[] = {450014};
    int arrMB30[] = {20, 450010, 450020};
    int arrCentral5[] = {20, 450010, 450020};
    int arrCentral[] = {15, 460101, 460111};
    int arrMB5[] = {1, 4, 16, 32, 450005, 450008, 450009, 450014, 450015, 450018, 450024, 450025, 450050, 450060};

    vector<unsigned int> mytriggers = mPicoEvent->triggerIds();
    int bin = 0;
    if(DoComparison(arrBHT1, sizeof(arrBHT1)/sizeof(*arrBHT1))) { bin = 2; h->Fill(bin); } // HT1
    if(DoComparison(arrBHT2, sizeof(arrBHT2)/sizeof(*arrBHT2))) { bin = 3; h->Fill(bin); } // HT2
    if(DoComparison(arrBHT3, sizeof(arrBHT3)/sizeof(*arrBHT3))) { bin = 4; h->Fill(bin); } // HT3 
    if(DoComparison(arrMB, sizeof(arrMB)/sizeof(*arrMB))) { bin = 5; h->Fill(bin); } // MB 
    //if() { bin = 6; h->Fill(bin); } 
    if(DoComparison(arrCentral5, sizeof(arrCentral5)/sizeof(*arrCentral5))) { bin = 7; h->Fill(bin); }// Central-5
    if(DoComparison(arrCentral, sizeof(arrCentral)/sizeof(*arrCentral))) { bin = 8; h->Fill(bin); } // Central & Central-mon
    if(DoComparison(arrMB5, sizeof(arrMB5)/sizeof(*arrMB5))) { bin = 10; h->Fill(bin); }// VPDMB-5 
    if(DoComparison(arrMB30, sizeof(arrMB30)/sizeof(*arrMB30))) { bin = 11; h->Fill(bin); } // VPDMB-30

    // label bins of the analysis trigger selection summary
    h->GetXaxis()->SetBinLabel(1, "un-identified trigger");
    h->GetXaxis()->SetBinLabel(2, "BHT1*VPDMB-30");
    h->GetXaxis()->SetBinLabel(3, "BHT2*VPDMB-30");
    h->GetXaxis()->SetBinLabel(4, "BHT3");
    h->GetXaxis()->SetBinLabel(5, "VPDMB-5-nobsmd");
    h->GetXaxis()->SetBinLabel(6, "");
    h->GetXaxis()->SetBinLabel(7, "Central-5");
    h->GetXaxis()->SetBinLabel(8, "Central or Central-mon");
    h->GetXaxis()->SetBinLabel(10, "VPDMB-5");
    h->GetXaxis()->SetBinLabel(11, "VPDMB-30");
  }

  // Run16 AuAu
  if(fRunFlag == StJetFrameworkPicoBase::Run16_AuAu200) {
    // get vector of triggers
    vector<unsigned int> mytriggers = mPicoEvent->triggerIds();
    int bin = 0;

    // hard-coded trigger Ids for run16
    int arrBHT0[] = {520606, 520616, 520626, 520636, 520646, 520656};
    int arrBHT1[] = {7, 15, 520201, 520211, 520221, 520231, 520241, 520251, 520261, 520605, 520615, 520625, 520635, 520645, 520655, 550201, 560201, 560202, 530201, 540201};
    int arrBHT2[] = {4, 16, 17, 530202, 540203};
    int arrBHT3[] = {17, 520203, 530213};
    int arrMB[] = {520021};
    int arrMB5[] = {1, 43, 45, 520001, 520002, 520003, 520011, 520012, 520013, 520021, 520022, 520023, 520031, 520033, 520041, 520042, 520043, 520051, 520822, 520832, 520842, 570702};
    int arrMB10[] = {7, 8, 56, 520007, 520017, 520027, 520037, 520201, 520211, 520221, 520231, 520241, 520251, 520261, 520601, 520611, 520621, 520631, 520641};
    int arrCentral[] = {6, 520101, 520111, 520121, 520131, 520141, 520103, 520113, 520123};

    // fill for kAny
    bin = 1; h->Fill(bin);

    // check if event triggers meet certain criteria and fill histos
    if(DoComparison(arrBHT1, sizeof(arrBHT1)/sizeof(*arrBHT1))) { bin = 2; h->Fill(bin); } // HT1
    if(DoComparison(arrBHT2, sizeof(arrBHT2)/sizeof(*arrBHT2))) { bin = 3; h->Fill(bin); } // HT2
    if(DoComparison(arrBHT3, sizeof(arrBHT3)/sizeof(*arrBHT3))) { bin = 4; h->Fill(bin); } // HT3
    if(DoComparison(arrMB, sizeof(arrMB)/sizeof(*arrMB))) { bin = 5; h->Fill(bin); }  // MB
    //if(mytriggers[i] == 999999) { bin = 6; h->Fill(bin); }
    if(DoComparison(arrCentral, sizeof(arrCentral)/sizeof(*arrCentral))) { bin = 7; h->Fill(bin); }// Central-5 & Central-novtx
    //if(mytriggers[i] == 999999) { bin = 8; h->Fill(bin); } 
    if(DoComparison(arrMB5, sizeof(arrMB5)/sizeof(*arrMB5))) { bin = 10; h->Fill(bin); } // VPDMB-5 
    if(DoComparison(arrMB10, sizeof(arrMB10)/sizeof(*arrMB10))) { bin = 11; h->Fill(bin); }// VPDMB-10

    // label bins of the analysis trigger selection summary
    h->GetXaxis()->SetBinLabel(1, "un-identified trigger");
    h->GetXaxis()->SetBinLabel(2, "BHT1");
    h->GetXaxis()->SetBinLabel(3, "BHT2");
    h->GetXaxis()->SetBinLabel(4, "BHT3");
    h->GetXaxis()->SetBinLabel(5, "VPDMB-5-p-sst");
    h->GetXaxis()->SetBinLabel(6, "");
    h->GetXaxis()->SetBinLabel(7, "Central");
    h->GetXaxis()->SetBinLabel(8, "");
    h->GetXaxis()->SetBinLabel(10, "VPDMB-5");
    h->GetXaxis()->SetBinLabel(11, "VPDMB-10");
  }

  // set x-axis labels vertically
  h->LabelsOption("v");
  //h->LabelsDeflate("X");

  return h;
}

// elems: sizeof(myarr)/sizeof(*myarr) prior to passing to function
// upon passing the array collapses to a pointer and can not get size anymore
//________________________________________________________________________
Bool_t StPicoTrackClusterQA::DoComparison(int myarr[], int elems) {
  //std::cout << "Length of array = " << (sizeof(myarr)/sizeof(*myarr)) << std::endl;
  bool match = kFALSE;

  // loop over specific physics selection array and compare to specific event trigger
  for(int i=0; i<elems; i++) {
    if(mPicoEvent->isTrigger(myarr[i])) match = kTRUE;
    if(match) break;
  }
  //cout<<"elems: "<<elems<<"  match: "<<match<<endl;

  return match;
}


//______________________________________________________________________
THnSparse* StPicoTrackClusterQA::NewTHnSparseFTracks(const char* name, UInt_t entries) {
  // generate new THnSparseD, axes are defined in GetDimParamsD()
  Int_t count = 0;
  UInt_t tmp = entries;
  while(tmp!=0){
    count++;
    tmp = tmp &~ -tmp;  // clear lowest bit
  }

  TString hnTitle(name);
  const Int_t dim = count;
  Int_t nbins[dim];
  Double_t xmin[dim];
  Double_t xmax[dim];

  Int_t i=0;
  Int_t c=0;
  while(c<dim && i<32){
    if(entries&(1<<i)){
      TString label("");
      GetDimParamsTracks(i, label, nbins[c], xmin[c], xmax[c]);
      hnTitle += Form(";%s",label.Data());
      c++;
    }

    i++;
  }
  hnTitle += ";";

  return new THnSparseF(name, hnTitle.Data(), dim, nbins, xmin, xmax);
} // end of NewTHnSparseF

//______________________________________________________________________________________________
void StPicoTrackClusterQA::GetDimParamsTracks(Int_t iEntry, TString &label, Int_t &nbins, Double_t &xmin, Double_t &xmax)
{
   //stores label and binning of axis for THnSparse
   const Double_t pi = TMath::Pi();

   switch(iEntry){

    case 0:
      label = "centrality 5% bin";
      nbins = 20; //16;
      xmin = 0.;
      xmax = 100.; //16.;
      break;

    case 1:
      label = "track p_{T}";
      nbins = 200;
      xmin = 0.;
      xmax = 20.;
      break;

    case 2:
      label = "track #eta";
      nbins = 24;
      xmin = -1.2;
      xmax =  1.2;
      break;

    case 3:
      label = "track #phi";
      nbins = 144;
      xmin = 0.0;
      xmax = 2.0*pi;
      break;

    case 4:
      label = "Z-vertex";
      nbins = 20;
      xmin = -40.;
      xmax = 40.;
      break;

   }// end of switch
} // end of track sparse

//______________________________________________________________________
THnSparse* StPicoTrackClusterQA::NewTHnSparseFTowers(const char* name, UInt_t entries) {
  // generate new THnSparseD, axes are defined in GetDimParamsD()
  Int_t count = 0;
  UInt_t tmp = entries;
  while(tmp!=0){
    count++;
    tmp = tmp &~ -tmp;  // clear lowest bit
  }

  TString hnTitle(name);
  const Int_t dim = count;
  Int_t nbins[dim];
  Double_t xmin[dim];
  Double_t xmax[dim];

  Int_t i=0;
  Int_t c=0;
  while(c<dim && i<32){
    if(entries&(1<<i)){
      TString label("");
      GetDimParamsTowers(i, label, nbins[c], xmin[c], xmax[c]);
      hnTitle += Form(";%s",label.Data());
      c++;
    }

    i++;
  }
  hnTitle += ";";

  return new THnSparseF(name, hnTitle.Data(), dim, nbins, xmin, xmax);
} // end of NewTHnSparseF

//______________________________________________________________________________________________
void StPicoTrackClusterQA::GetDimParamsTowers(Int_t iEntry, TString &label, Int_t &nbins, Double_t &xmin, Double_t &xmax)
{
   //stores label and binning of axis for THnSparse
   const Double_t pi = TMath::Pi();

   switch(iEntry){

    case 0:
      label = "centrality 5% bin";
      nbins = 20; //16;
      xmin = 0.;
      xmax = 100.; //16.;
      break;

    case 1:
      label = "tower E";
      nbins = 200;
      xmin = 0.;
      xmax = 20.;
      break;

    case 2:
      label = "tower #eta";
      nbins = 24;
      xmin = -1.2;
      xmax =  1.2;
      break;

    case 3:
      label = "tower #phi";
      nbins = 144;
      xmin = 0.0;
      xmax = 2.0*pi;
      break;

    case 4:
      label = "Z-vertex";
      nbins = 20;
      xmin = -40.;
      xmax = 40.;
      break;

   }// end of switch
} // end of tower sparse

//============================================================================
// 
// this is for a test... for now.. Feb12, 2018
//____________________________________________________________________________
Bool_t StPicoTrackClusterQA::MuProcessBEMC() {
  /* this is reworked, but a simple reimplementation
     of the old Maker code. It should be updated for
     future use - currently a track is matched to the
     first tower within dR of its BEMC projection
     where it should be matched to the tower
     with the minimum dR. This might require
     a rework of MuProcessPrimaryTracks as well.
   */
  /* define the number of modules in the BEMC */
  UInt_t   mBEMCModules         = 120;
  UInt_t   mBEMCTowPerModule    = 20;
  Int_t    mBEMCSubSections     = 2;
  
  /* and we will count the # of towers/tracks matched, as this
     is saved in the event header. Not sure why 
   */
  Int_t nMatchedTowers = 0;
  Int_t nMatchedTracks = 0;
 
  // are collections used in AuAu??? FIXME
  // not according to: http://www.star.bnl.gov/public/comp/meet/RM200311/MuDstTutorial.pdf 
  StEmcCollection* mEmcCollection = (StEmcCollection*) mMuDst->emcCollection();
  StMuEmcCollection* mMuEmcCollection = (StMuEmcCollection*) mMuDst->muEmcCollection();
  mBemcTables->loadTables((StMaker*)this);
  
  /* if no collections are found, exit assuming error */
  if (!mEmcCollection || !mMuEmcCollection) return kFALSE;

  StEmcDetector* mBEMC = mEmcCollection->detector(kBarrelEmcTowerId);
  StSPtrVecEmcPoint& mEmcContainer =  mEmcCollection->barrelPoints();
  
  /* if we can't get the detector exit assuming error */
  if (!mBEMC) return kFALSE;

  /* if there are no hits, continue assuming everything
     is working */
  if(mEmcContainer.size() == 0) return kTRUE;

  ////TStarJetPicoTower jetTower;
  
  /* loop over all modules */
  for (UInt_t i = 1; i <= mBEMCModules; ++i) {
    StSPtrVecEmcRawHit& mEmcTowerHits = mBEMC->module(i)->hits();
    
    /* loop over all hits in the module */
    for (UInt_t j = 0; j < mEmcTowerHits.size(); ++j) {
      StEmcRawHit* tow = mEmcTowerHits[j];
      
      if (abs(tow->module()) > mBEMCModules  || abs(tow->eta()) > mBEMCTowPerModule || tow->sub() >  mBEMCSubSections) continue;
      
      Int_t towerID, towerStatus;
      mGeom->getId((int)tow->module(), (int)tow->eta(), (int)tow->sub(), towerID);
      mBemcTables->getStatus(BTOW, towerID, towerStatus);
      
      if (mTowerStatusMode == RejectBadTowerStatus && towerStatus != 1) continue;
      
      Float_t towerEnergy = tow->energy();
      Float_t towerADC = tow->adc();
     
      // mTowerEnergyMin = 0.2
      if(towerEnergy < 0.2) continue;
      
      Float_t towerEta, towerPhi;
      mGeom->getEtaPhi(towerID, towerEta, towerPhi);
      
      /* check for SMD hits in the tower */
      Int_t ehits = MuFindSMDClusterHits(mEmcCollection, towerEta, towerPhi, 2);
      Int_t phits = MuFindSMDClusterHits(mEmcCollection, towerEta, towerPhi, 3);
      
      /* correct eta for Vz position */
      Float_t theta;
      theta = 2 * atan(exp(-towerEta)); /* getting theta from eta */
      Double_t z = 0;
      if(towerEta != 0) z = 231.0 / tan(theta);  /* 231 cm = radius of SMD */
      //Double_t zNominal = z - mMuDst->event()->primaryVertexPosition().z(); /* shifted z */
      Double_t zNominal = z - mVertex.z();       /* shifted z*/    // should be fixed
      Double_t thetaCorr = atan2(231.0, zNominal); /* theta with respect to primary vertex */
      Float_t etaCorr =-log(tan(thetaCorr / 2.0)); /* eta with respect to primary vertex */
      
      /* now match tracks to towers */
      for (unsigned k = 0; k < mBemcMatchedTracks.size(); ++k) {
        BemcMatch match = mBemcMatchedTracks[k];
        if (match.globalId == -1) continue;
        
        Double_t halfTowerWidth = 0.025;
        Double_t dEta = match.matchEta - towerEta;
        Double_t dPhi = match.matchPhi - towerPhi;
        while (dPhi > TMath::Pi())  dPhi -= TMath::Pi();
        while (dPhi < -TMath::Pi()) dPhi += TMath::Pi();
        if (fabs(dEta) > halfTowerWidth || fabs(dPhi) > halfTowerWidth) continue;
        nMatchedTracks++;
        mBemcMatchedTracks[k].globalId = -1;
        ////jetTower.AddMatchedTrack(match.trackId);
        
        /* set dEta & dPhi for the matched track */
        ////TStarJetPicoPrimaryTrack* matchedTrack = (TStarJetPicoPrimaryTrack*) mEvent->GetPrimaryTracks()->At(match.trackId);
        ////matchedTrack->SetEtaDiffHitProjected(dEta);
        ////matchedTrack->SetPhiDiffHitProjected(dPhi);
      }
      
      ////if (jetTower.GetNAssocTracks() > 0) nMatchedTowers++;
      ////mEvent->AddTower(&jetTower);
    }
    
  }
  ////mEvent->GetHeader()->SetNOfMatchedTracks(nMatchedTracks);
  ////mEvent->GetHeader()->SetNOfMatchedTowers(nMatchedTowers);
  
  return kTRUE;
}

//
//
//___________________________________________________________________________________________________
Int_t StPicoTrackClusterQA::MuFindSMDClusterHits(StEmcCollection* coll, Double_t eta, Double_t phi, Int_t detectorID) {
  StEmcCluster* smdCluster = nullptr;
  Float_t dRmin = 9999;
  Float_t dRmin_cut = 0.05;
  StDetectorId id = static_cast<StDetectorId>(detectorID + kBarrelEmcTowerId);
  
  StEmcDetector* detector = coll->detector(id);
  if (!detector) return 0;
  StEmcClusterCollection* clusters = detector->cluster();
  if (!clusters) return 0;
  StSPtrVecEmcCluster& cl=clusters->clusters();
  
  for (unsigned i = 0; i < cl.size(); ++i) {
    Float_t clEta = cl[i]->eta();
    Float_t clPhi = cl[i]->phi();
    Float_t dR = sqrt((eta - clEta) * (eta - clEta) + (phi - clPhi) * (phi - clPhi));
    
    if (dR < dRmin && dR < dRmin_cut) {
      dRmin = dR;
      smdCluster = cl[i];
    }
    
  }
  
  if (smdCluster) return smdCluster->nHits();
  else return 0;
}

//________________________________________________________________________
void StPicoTrackClusterQA::RunTowerTest()
{
  // set / initialize some variables
  double pi = 1.0*TMath::Pi();
  double pi0mass = Pico::mMass[0]; // GeV
  StEmcPosition *mPosition = new StEmcPosition();

  // towerStatus array
  float mTowerMatchTrkIndex[4801] = { 0 };
  bool mTowerStatusArr[4801] = { 0 };
  int matchedTowerTrackCounter = 0;

  // print
  int nTracks = mPicoDst->numberOfTracks();
  int nTrigs = mPicoDst->numberOfEmcTriggers();
  int nBTowHits = mPicoDst->numberOfBTOWHits();
  int nBEmcPidTraits = mPicoDst->numberOfBEmcPidTraits();
//  cout<<"nTracks = "<<nTracks<<"  nTrigs = "<<nTrigs<<"  nBTowHits = "<<nBTowHits<<"  nBEmcPidTraits = "<<nBEmcPidTraits<<endl;
  
  // loop over ALL clusters in PicoDst and add to jet //TODO
  for(unsigned short iClus = 0; iClus < nBEmcPidTraits; iClus++){
    StPicoBEmcPidTraits* cluster = mPicoDst->bemcPidTraits(iClus);
    if(!cluster){ cout<<"Cluster pointer does not exist.. iClus = "<<iClus<<endl; continue; }

    // cluster and tower ID
    int clusID = cluster->bemcId();  // index in bemc point array
    int towID = cluster->btowId();   // projected tower Id: 1 - 4800
    int towID2 = cluster->btowId2(); // emc 2nd and 3rd closest tower local id  ( 2nd X 10 + 3rd), each id 0-8
    int towID3 = cluster->btowId3(); // emc 2nd and 3rd closest tower local id  ( 2nd X 10 + 3rd), each id 0-8
    if(towID < 0) continue;

    // get tower location - from ID: via this method, need to correct eta
    float tEta, tPhi;    // need to be floats
    StEmcGeom *mGeom2 = (StEmcGeom::instance("bemc"));
    mGeom2->getEtaPhi(towID,tEta,tPhi);
    if(tPhi < 0)    tPhi += 2*pi;
    if(tPhi > 2*pi) tPhi -= 2*pi;
    if(fDebugLevel == 8) {
      cout<<"towID1 = "<<towID<<"  towID2 = "<<towID2<<"  towID3 = "<<towID3<<"   towerEta = "<<tEta<<"  towerPhi = "<<tPhi<<endl;
    }

    // cluster and tower position - from vertex and ID
    StThreeVectorF  towPosition;
    towPosition = mPosition->getPosFromVertex(mVertex, towID);
    double towPhi = towPosition.phi();
    double towEta = towPosition.pseudoRapidity();
    double detectorRadius = mGeom2->Radius();  // use this below so you don't have to hardcode it TODO

    // correct eta for Vz position // 231
    float theta = 2 * atan(exp(-towEta)); // getting theta from eta 
    double z = 0;
    if(towEta != 0) z = 225.405 / tan(theta);      // 231 cm = radius of SMD
    double zNominal = z - mVertex.z();
    double thetaCorr = atan2(225.405, zNominal); // theta with respect to primary vertex
    float etaCorr =-log(tan(thetaCorr / 2.0));   // eta with respect to primary vertex

    // correct eta for Vz position // 231
    float theta2 = 2 * atan(exp(-tEta)); // getting theta from eta 
    double z2 = 0;
    if(tEta != 0) z2 = 225.405 / tan(theta2);         // 231 cm = radius of SMD
    double zNominal2 = z2 - mVertex.z();
    double thetaCorr2 = atan2(225.405, zNominal2); // theta with respect to primary vertex
    float etaCorr2 =-log(tan(thetaCorr2 / 2.0));   // eta with respect to primary vertex

    // matched track index
    int trackIndex = cluster->trackIndex();
    StPicoTrack* trk = (StPicoTrack*)mPicoDst->track(trackIndex);
    //StMuTrack* mutrk = (StMuTrack*)mPicoDst->track(trackIndex);
    if(!trk) { cout<<"No trk pointer...."<<endl; continue; }
    if(!AcceptTrack(trk, Bfield, mVertex)) { continue; }

    // tower status set - towerID is matched to track passing quality cuts
    mTowerMatchTrkIndex[towID] = trackIndex;
    mTowerStatusArr[towID] = kTRUE;
    matchedTowerTrackCounter++;

/*  // probably don't need this chunk.. March 14, 2018
 *
    // get track variables to matched tower
    StThreeVectorF mTrkMom;
    if(doUsePrimTracks) { 
      // get primary track vector
      mTrkMom = trk->pMom();
    } else { 
      // get global track vector
      mTrkMom = trk->gMom(mVertex, Bfield); 
    }

    // track properties
    double pt = mTrkMom.perp();
    double phi = mTrkMom.phi();
    double eta = mTrkMom.pseudoRapidity();
    double p = mTrkMom.mag();
*/

    // print tower and track info
//    cout<<"towers: towPhi = "<<towPhi<<"  towEta = "<<towEta<<"  etaCorr = "<<etaCorr;  //<<endl;
//    cout<<"  tPhi = "<<tPhi<<"  tEta = "<<tEta<<"  etaCorr2 = "<<etaCorr2<<endl;
//    cout<<"tracks:  pt = "<<pt<<"  p = "<<p<<"  phi = "<<phi<<"  eta = "<<eta<<endl;

  }

  // print statment on matches
  //cout<<"Matched Tracks passing cuts (with tower): "<<matchedTowerTrackCounter<<"  nBTowHits = ";
  //cout<<mPicoDst->numberOfBTOWHits()<<"  unFiltered Tracks = "<<mPicoDst->numberOfTracks()<<"  Filtered Tracks = "<<fGoodTrackCounter<<endl;

  // loop over towers
  int nTowers = mPicoDst->numberOfBTOWHits();
  for(int itow = 0; itow < nTowers; itow++) {
    StPicoBTowHit *tower = mPicoDst->btowHit(itow);
    if(!tower) { cout<<"No tower pointer... iTow = "<<itow<<endl; continue; }

    // tower ID
    int towerID = tower->id();
    if(towerID < 0) continue; // double check these aren't still in the event list

    // cluster and tower position - from vertex and ID: shouldn't need additional eta correction
    StThreeVectorF towerPosition = mPosition->getPosFromVertex(mVertex, towerID);
    double towerPhi = towerPosition.phi();
    double towerEta = towerPosition.pseudoRapidity();
    int towerADC = tower->adc();
    double towerEunCorr = tower->energy();
    double towerE = tower->energy();

    // tower matched to firing trigger - TODO
    //if(fTowerToTriggerTypeHT1[emcTrigID])
    //if(fTowerToTriggerTypeHT2[emcTrigID])
    //if(fTowerToTriggerTypeHT3[emcTrigID])

    // perform tower cuts
    // if tower was not matched to an accepted track, use it for jet by itself if > 0.2 GeV
    if(mTowerStatusArr[towerID]) {
      //if(mTowerMatchTrkIndex[towerID] > 0) 
      StPicoTrack* trk = (StPicoTrack*)mPicoDst->track( mTowerMatchTrkIndex[towerID] );
      ///if(!trk) { cout<<"No trk pointer...."<<endl; continue; } // March5, 2018 commented back in TODO
      //if(!AcceptTrack(trk, Bfield, mVertex)) { continue; }

      // get track variables to matched tower
      StThreeVectorF mTrkMom;
      if(doUsePrimTracks) { 
        // get primary track vector
        mTrkMom = trk->pMom(); 
      } else { 
        // get global track vector
        mTrkMom = trk->gMom(mVertex, Bfield); 
      }

      double pt = mTrkMom.perp();
      double phi = mTrkMom.phi();
      double eta = mTrkMom.pseudoRapidity();
      double p = mTrkMom.mag();
      double pi0mass = Pico::mMass[0]; // GeV
      double E = 1.0*TMath::Sqrt(p*p + pi0mass*pi0mass);

      // apply hadronic correction
      towerE = towerE - (mHadronicCorrFrac * E);
    } 
    // else - no match so treat towers on their own

    // cut on tower energy - corrected or not
    if(towerE < 0) towerE = 0.0;
    if(towerE < mTowerEnergyMin) continue;

/*
    // Feb26, 2018: don't think I need this if using getPosFromVertex(vert, id)
    // correct eta for Vz position 
    float theta;
    theta = 2 * atan(exp(-towEta)); // getting theta from eta 
    double z = 0;
    if(towEta != 0) z = 231.0 / tan(theta);  // 231 cm = radius of SMD 
    double zNominal = z - mVertex.z();
    double thetaCorr = atan2(231.0, zNominal); // theta with respect to primary vertex
    float etaCorr =-log(tan(thetaCorr / 2.0)); // eta with respect to primary vertex 
*/

    // print
//    cout<<"itow: "<<itow<<"  towerID = "<<towerID<<"  towerPhi = "<<towerPhi<<"  towerEta = "<<towerEta<<"  towerADC = "<<towerADC<<"  towerE = "<<towerE<<"  towerEunCorr = "<<towerEunCorr<<"  mIndex = "<<mTowerMatchTrkIndex[towerID]<<endl;

    // fill QA histos for towers
    fHistNTowervsE->Fill(towerE);
    fHistNTowervsPhi->Fill(towerPhi);
    fHistNTowervsEta->Fill(towerEta);
    fHistNTowervsPhivsEta->Fill(towerPhi, towerEta);

  } // tower loop

  //} // cluster loop
} // cluster / tower QA
