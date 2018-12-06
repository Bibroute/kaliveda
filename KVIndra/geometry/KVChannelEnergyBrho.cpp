/***************************************************************************
$Id: KVChannelEnergyBrho.cpp,v 1.7 2007/02/27 11:56:33 franklan Exp $
                          KVChannelEnergyBrho.cpp  -  description
                             -------------------
    begin                :  Feb 7th 2004
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
#include "KVChannelEnergyBrho.h"
#include "TMath.h"
#include "KVSilicon.h"

ClassImp(KVChannelEnergyBrho);

////////////////////////////////////////////////////////////////////////////
//Linear channel-energy calibration for silicon and ChIo detectors calculated
//from Brho points for different light nuclei up to Z=6~8.
//The channel value is the grand gain (GG) value. If this is saturated, the
//pedestal-corrected PG channel is used to estimate the GG channel.


KVChannelEnergyBrho::KVChannelEnergyBrho(): KVCalibrator(3)
{
   SetType("Channel-Energy Brho");
}

//___________________________________________________________________________
KVChannelEnergyBrho::KVChannelEnergyBrho(KVDetector* kvd): KVCalibrator(3)
{
   //Create an electronic calibration object for a specific detector (*kvd)
   SetType("Channel-Energy Brho");
   SetDetector(kvd);
}

//___________________________________________________________________________
Double_t KVChannelEnergyBrho::Compute(Double_t chan) const
{
   //Calculate the calibrated energy (in MeV) for a given GG channel number.
   //
   //If GG is saturated (>4000: we avoid the top end of the GG),
   //an "extrapolated" GG is calculated from PG using experimental fits.

   if (fReady) {
      Double_t chan_corr;
      if (chan > 4090.) {
         chan = ((KVSilicon*) GetDetector())->GetGGfromPG();
      }
      chan_corr = chan - GetDetector()->GetPedestal("GG");
      return (GetParameter(0) + GetParameter(1) * chan_corr);
   }
   return 0.;
}


//___________________________________________________________________________
Double_t KVChannelEnergyBrho::operator()(Double_t chan)
{
   //Overloading of "()" to allow syntax such as:
   //
   //        KVChannelEnergyBrho calibrator;
   //           ....
   //        Float_t calibrated_volts = calibrator(channel);
   //
   //equivalently to:
   //
   //        Float_t calibrated_volts = calibrator.Compute(channel);
   return Compute(chan);
}

//___________________________________________________________________________
Double_t KVChannelEnergyBrho::Invert(Double_t energy)
{
   //Given the calibrated (or simulated) energy in MeV,
   //calculate the corresponding channel number according to the
   //calibration parameters (useful for filtering simulations).

   KVDetector* det = GetDetector();
   Int_t channel = 0;

   if (fReady) {
      // linear transfer function
      channel = (Int_t)(0.5 + (energy - GetParameter(0)) / GetParameter(1)
                        + det->GetPedestal("GG"));
   }
   else {
      Warning("Compute", "Parameters not correctly initialized");
   }
   return (Double_t) channel;
}
