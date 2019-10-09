/***************************************************************************
                          KVIDINDRACsI.cpp  -  description
                             -------------------
    begin                : Fri Feb 20 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVIDINDRACsI.h"
#include "KVINDRACodeMask.h"
#include "TMath.h"
#include "KVIdentificationResult.h"
#include <KVIDGCsI.h>
#include <KVINDRADetector.h>
#include <KVReconstructedNucleus.h>

ClassImp(KVIDINDRACsI)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//KVIDINDRACsI
//
//Identification in CsI R-L matrices of INDRA
//
//Identification subcodes are written in bits 0-3 of KVIDSubCodeManager
//(see KVINDRACodes). They correspond to the values of KVIDGCsI::GetQualityCode()
//(see KVIDGCsI class description).
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

KVIDINDRACsI::KVIDINDRACsI()
{
   fIDCode = kIDCode_CsI;
   fZminCode = kIDCode_ZminCsI;
   fECode = kECode1;
   SetSubCodeManager(4, 3);
   CsIGrid = 0;

   fThresMin[0][0] = 1;
   fThresMax[0][0] = 2;    // protons
   fThresMin[0][1] = 2;
   fThresMax[0][1] = 6;    // deutons
   fThresMin[0][2] = 5;
   fThresMax[0][2] = 11;   // tritons
   fThresMin[0][3] = -1;
   fThresMax[0][3] = -1;   // 4H - NO!
   fThresMin[1][0] = -1;
   fThresMax[1][0] = -1;   // 1He - NO!
   fThresMin[1][1] = -1;
   fThresMax[1][1] = -1;   // 2He - NO!
   fThresMin[1][2] = 20;
   fThresMax[1][2] = 40;   // 3He
   fThresMin[1][3] = 1;
   fThresMax[1][3] = 3;    // alphas

   /* in principle all CsI R-L telescopes can identify mass & charge */
   SetHasMassID(kTRUE);
}

//________________________________________________________________________________________//

const Char_t* KVIDINDRACsI::GetArrayName()
{
   // Name of telescope given in the form CSI_R_L_Ring-numberTelescope-number
   // where ring and telescope numbers are those of the smallest (in angular terms)
   // detector of the telescope (if both are the same size, either one will do).
   // The detectors are signified by their TYPE names i.e. KVDetector::GetType

   //in order to access angular dimensions of detectors, we need their KVTelescopes
   KVINDRADetector* de_det = dynamic_cast<KVINDRADetector*>(GetDetector(1));
   UInt_t ring, mod;
   ring = de_det->GetRingNumber();
   mod = de_det->GetModuleNumber();
   TString dummy;
   dummy.Form("%s_R_L", GetDetector(1)->GetType());
   SetType(dummy.Data());
   dummy.Form("%s_%02d%02d", GetType(), ring, mod);
   SetName(dummy.Data());

   return fName.Data();
}

//________________________________________________________________________________________//

Bool_t KVIDINDRACsI::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid KVIDGCsI* fGrid.

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;

   //perform identification
   Double_t csir, csil;
   GetIDGridCoords(csil, csir, CsIGrid, x, y);
   CsIGrid->Identify(csil, csir, IDR);
   IDR->SetGridName(CsIGrid->GetName());

   // set general ID code
   IDR->IDcode = kIDCode2;

   // general ID code for gammas
   if (IDR->IDquality == KVIDGCsI::kICODE10)
      IDR->IDcode = kIDCode0;

   return kTRUE;
}

void KVIDINDRACsI::SetIdentificationStatus(KVReconstructedNucleus* n)
{
   // For filtering simulations
   // If n->GetEnergy() is above threshold for mass identification, we set
   // n->IsAMeasured(kTRUE) (and n->IsZMeasured(kTRUE)).
   // Otherwise, we just set n->IsZMeasured(kTRUE) and use the A given by
   // the mass formula for the particle
   //
   // individual thresholds defined for 1H, 2H, 3H, 3He, 4He
   // for A>5 identification if CsI energy > 40 MeV
   //
   // If A is not measured, we make sure the KE of the particle corresponds to the simulated one

   n->SetZMeasured();

   if (n->GetA() > 5) {
      if (GetDetector(1)->GetEnergy() > 40)
         n->SetAMeasured();
      else {
         double e = n->GetE();
         n->SetZ(n->GetZ());
         n->SetE(e);
      }
      return;
   }
   if (fThresMin[n->GetZ() - 1][n->GetA() - 1] > 0) {
      Bool_t okmass = gRandom->Uniform() < smootherstep(fThresMin[n->GetZ() - 1][n->GetA() - 1], fThresMax[n->GetZ() - 1][n->GetA() - 1], GetDetector(1)->GetEnergy());
      if (okmass) {
         n->SetAMeasured();
      }
   }
   else {
      double e = n->GetE();
      n->SetZ(n->GetZ());
      n->SetE(e);
   }
}


float KVIDINDRACsI::clamp(float x, float lowerlimit, float upperlimit)
{
   if (x < lowerlimit)
      x = lowerlimit;
   if (x > upperlimit)
      x = upperlimit;
   return x;
}

float KVIDINDRACsI::smootherstep(float edge0, float edge1, float x)
{
   // Scale, and clamp x to 0..1 range
   x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
   // Evaluate polynomial
   return x * x * x * (x * (x * 6 - 15) + 10);
}
