#ifndef STRHOBASE_H
#define STRHOBASE_H

// $Id$
// adapted from the AliROOT class AliAnalysisTaskRhoBase.h for STAR
#include "StJetFrameworkPicoBase.h"

//#include "StJetMakerTask.h"
#include "StMaker.h"
//#include "StJet.h"

// root classes
class TString;
class TF1;
class TH1F;
class TH2F;
class TH3F;

// STAR classes
class StMaker;
class StRhoParameter;

// classes for header
class StRefMultCorr;

// might not want to inherit from 2 classes
//class StRhoBase : public StMaker, public StJet {
//class StRhoBase : public StJetMakerTask {
//class StRhoBase : public StMaker {  // TODO need to check if this is fine
class StRhoBase : public StJetFrameworkPicoBase {

 public:
  StRhoBase();
  StRhoBase(const char *name, Bool_t histo=kFALSE, const char *outName="", const char *jetMakerName="");
  //virtual ~StRhoBase() {}
  virtual ~StRhoBase();

  // class required functions
  virtual Int_t Init();
  virtual Int_t Make();
  virtual void  Clear(Option_t *opt="");
  virtual Int_t Finish();

  // booking of histograms (optional)
  void    DeclareHistograms();
  void    WriteHistograms();

  void                   SetEventZVtxRange(Double_t zmi, Double_t zma)         { fEventZVtxMinCut = zmi; fEventZVtxMaxCut = zma;     }

  void                   SetOutRhoName(const char *name)                       { fOutRhoName           = name    ;
                                                                                 fOutRhoScaledName     = Form("%s_Scaled",name);     }
  void                   SetCompareRhoName(const char *name)                   { fCompareRhoName       = name    ;                   }
  void                   SetCompareRhoScaledName(const char *name)             { fCompareRhoScaledName = name    ;                   }
  void                   SetScaleFunction(TF1* sf)                             { fScaleFunction        = sf      ;                   }
  void                   SetRhoFunction(TF1* rf)                               { fRhoFunction          = rf      ;                   }
  TF1*                   LoadRhoFunction(const char* path, const char* name);
  void                   SetInEventSigmaRho(Double_t s)                        { fInEventSigmaRho      = s       ;                   }
  void                   SetAttachToEvent(Bool_t a)                            { fAttachToEvent        = a       ;                   }
  void                   SetSmallSystem(Bool_t setter = kTRUE)                 { fIsAuAu               = !setter ;                   }

  const char*            GetOutRhoName() const                                 { return fOutRhoName.Data()       ;                   }
  const char*            GetOutRhoScaledName() const                           { return fOutRhoScaledName.Data() ;                   }

  StRhoParameter* GetRho()                        { return fOutRho; }
  StRhoParameter* GetRhoScaled()                  { return fOutRhoScaled; }
//  double GetRhoVal() {return GetRho()->GetVal(); }
//  Double_t        GetRhoVal()            const    {if (fRho) return fRho->GetVal(); else return 0;}

 protected:
  Bool_t                 FillHistograms();

  virtual Double_t       GetRhoFactor(Double_t cent);
  virtual Double_t       GetScaleFactor(Double_t cent);

  TString                fOutRhoName;                    // name of output rho object
  TString                fOutRhoScaledName;              // name of output scaled rho object
  TString                fCompareRhoName;                // name of rho object to compare
  TString                fCompareRhoScaledName;          // name of scaled rho object to compare
  TF1                   *fRhoFunction;                   // pre-computed rho as a function of centrality
  TF1                   *fScaleFunction;                 // pre-computed scale factor as a function of centrality
  Double_t               fInEventSigmaRho;               // in-event sigma rho
  Bool_t                 fAttachToEvent;                 // whether or not attach rho to the event objects list
  Bool_t                 fIsAuAu;                        // different histogram ranges for pp/pAu and AuAu
  
  StRhoParameter        *fOutRho;                        //!output rho object
  StRhoParameter        *fOutRhoScaled;                  //!output scaled rho object
  StRhoParameter        *fCompareRho;                    //!rho object to compare
  StRhoParameter        *fCompareRhoScaled;              //!scaled rho object to compare

  TH2F                  *fHistJetPtvsCent;               //!jet pt vs. centrality
  TH2F                  *fHistJetAreavsCent;             //!jet area vs. centrality
  TH2F                  *fHistJetRhovsCent;              //!jet pt/area vs. centrality
  TH2F                  *fHistNjetvsCent;                //!no. of jets vs. centrality
  TH2F                  *fHistJetPtvsNtrack;             //!jet pt vs. no. of tracks
  TH2F                  *fHistJetAreavsNtrack;           //!jet area vs. no. of tracks
  TH2F                  *fHistNjetvsNtrack;              //!no. of jets vs. no. of tracks
  TH2F                  *fHistNjUEoverNjVsNj[12];        //!ratio no. of jets below rho*A+sigma_rho over. no. of jets vs. no. of jets
  TH2F                  *fHistJetNconstVsPt[4];          //!jet no. of constituents vs. pt
  TH2F                  *fHistJetRhovsEta[4];            //!rho vs. eta
  TH2F                  *fHistRhovsCent;                 //!rho vs. centrality
  TH2F                  *fHistRhoScaledvsCent;           //!rhoscaled vs. centrality
  TH2F                  *fHistDeltaRhovsCent;            //!delta rho vs. centrality
  TH2F                  *fHistDeltaRhoScalevsCent;       //!delta rhoscaled vs. centrality

  TH3F                  *fHistRhovsNtrackvsMult;         //!rho vs. no. of tracks vs mult
  TH3F                  *fHistRhoScaledvsNtrackvsMult;   //!rhoscaled vs. no. of tracks vs mult
  TH2F                  *fHistDeltaRhovsNtrack;          //!delta rho vs. no. of tracks
  TH2F                  *fHistDeltaRhoScalevsNtrack;     //!delta rho scaled vs. no. of tracks
 
  TH2F                  *fHistRhovsNcluster;             //!rho vs. no. of clusters
  TH2F                  *fHistRhoScaledvsNcluster;       //!rhoscaled vs. no. of clusters

/*
  // maker names
  TString                mOutName;
  TString                fJetMakerName;
  TString                fJetBGMakerName;
  TString                fRhoMakerName;
*/

  StRhoBase(const StRhoBase&);             // not implemented
  StRhoBase& operator=(const StRhoBase&);  // not implemented
  
  ClassDef(StRhoBase, 1); // Rho base task
};
#endif
