#ifndef _IONCHAMBER_CLASS
#define _IONCHAMBER_CLASS

#include "Rtypes.h"
#include"Defines.h"
#include"LogFile.h"
#include"Random.h"
#include "TTree.h"

class IonisationChamberv
{
   Bool_t Ready;
 public:
  IonisationChamberv(LogFile *LOg);
  virtual ~IonisationChamberv(void);
  
  LogFile *L;

  bool Present; 

  void Init(void); 
  void InitRaw(void); 
  void Calibrate(void); 
  void Show_Raw(void);
  void Show(void);
  void Treat(void);
  void inAttach(TTree *inT);
  void outAttach(TTree *outT);
  void CreateHistograms();
  void FillHistograms();
  void PrintCounters(void);

  Random *Rnd;

  //energy Raw
  UShort_t E_Raw[3*8];
  UShort_t E_Raw_Nr[3*8];
  Int_t E_RawM;

  //Calibration coeff
  Float_t ECoef[3][7][3];

  //energy Calibrated
  Float_t E[3*8];
  UShort_t ENr[3*8];
  Int_t EM;
  Float_t ERef[3][2]; //Position & energy
  Float_t ES[3]; //Energy sum per row
  Float_t ETotal;
  Float_t ETresh[3];

  //Counters
  Int_t Counter[5];

  ClassDef(IonisationChamberv,0)
};

#endif
