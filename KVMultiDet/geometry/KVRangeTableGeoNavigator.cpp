//Created by KVClassFactory on Tue Apr 23 10:27:56 2013
//Author: John Frankland,,,

#include "KVRangeTableGeoNavigator.h"
#include <TGeoMaterial.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoNode.h>
#include "KVNucleus.h"
#include <KVIonRangeTableMaterial.h>

ClassImp(KVRangeTableGeoNavigator)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVRangeTableGeoNavigator</h2>
<h4>Propagate particles through a geometry and calculate their energy losses</h4>
<!-- */
// --> END_HTML
// Given a valid ROOT geometry, we propagate the particles of an event
// and, every time a particle traverses a volume made of a TGeoMaterial
// with a name corresponding to a material known by this range table,
// we calculate the energy loss of the particle,
// and store it in the particle's list KVParticle::fParameters in the form
//
//   "DE:[detector name]" = [energy lost in volume]
//
// We also store the coordinates (world-frame) of the particle's entry and exit
// in each volume:
//
//   "Xin:[detector name]" = [entry coordinates for volume]
//   "Yin:[detector name]"
//   "Zin:[detector name]"
//   "Xout:[detector name]" = [exit coordinates for volume]
//   "Yout:[detector name]"
//   "Zout:[detector name]"
//
// If the particle stops in the volume, the 'out' coordinates will correspond to
// the calculated range of the particle in the volume.
// In the case of multilayer detectors, "[detector name]" in the previous
// examples is replaced by "[detector name]/[layer name]"
//
// EXAMPLE OF USE
// ==============
//
// In this case the geometry is made up of 3-layer Ionisation Chamber detectors
// (window + gas + window), with an "ACTIVE" gas layer, and single-layer
// Silicon detectors placed behind the IC.
//
// KVEvent* evt = new KVEvent();
// KVNucleus* nuc = evt->AddParticle();
// Double_t theta = gRandom->Uniform(0,30);
// Double_t phi = gRandom->Uniform(0,360);
// nuc->SetZAandE(1,1,200); nuc->SetTheta(theta); nuc->SetPhi(phi);
//
// KVRangeTableGeoNavigator rtgn(gGeoManager, new KVedaLoss)
// rtgn.PropagateEvent(evt)
// evt->Print()
//
// KVEvent with 1 particles :
// ------------------------------------
// KVNameValueList::EventParameters : Parameters associated with an event (0x3739f10)
// KVNucleus Z=1 A=1 E*=0
// KVParticle mass=938.783 Theta=14.017 Phi=44.7553 KE=199.73
// KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x37b3510)
// <DE:CHIO_3/WINDOW_1=0.00151169>
// <Xin:CHIO_3/WINDOW_1=3.34783>
// <Yin:CHIO_3/WINDOW_1=3.31934>
// <Zin:CHIO_3/WINDOW_1=18.8848>
// <Xout:CHIO_3/WINDOW_1=3.34787>
// <Yout:CHIO_3/WINDOW_1=3.31939>
// <Zout:CHIO_3/WINDOW_1=18.885>
// <DE:CHIO_3/ACTIVE_GAS_1=0.00123736>
// <Xin:CHIO_3/ACTIVE_GAS_1=3.34787>
// <Yin:CHIO_3/ACTIVE_GAS_1=3.31939>
// <Zin:CHIO_3/ACTIVE_GAS_1=18.885>
// <Xout:CHIO_3/ACTIVE_GAS_1=3.70242>
// <Yout:CHIO_3/ACTIVE_GAS_1=3.67093>
// <Zout:CHIO_3/ACTIVE_GAS_1=20.885>
// <DE:CHIO_3/WINDOW_2=0.00151169>
// <Xin:CHIO_3/WINDOW_2=3.70242>
// <Yin:CHIO_3/WINDOW_2=3.67093>
// <Zin:CHIO_3/WINDOW_2=20.885>
// <Xout:CHIO_3/WINDOW_2=3.70247>
// <Yout:CHIO_3/WINDOW_2=3.67097>
// <Zout:CHIO_3/WINDOW_2=20.8853>
// <DE:SI_1_5=0.265778>
// <Xin:SI_1_5=3.73792>
// <Yin:SI_1_5=3.70612>
// <Zin:SI_1_5=21.0852>
// <Xout:SI_1_5=3.74324>
// <Yout:SI_1_5=3.7114>
// <Zout:SI_1_5=21.1152>
//
////////////////////////////////////////////////////////////////////////////////

void KVRangeTableGeoNavigator::ParticleEntersNewVolume(KVNucleus* part)
{
   // Overrides method in KVGeoNavigator base class.
   // Every time a particle enters a new volume, we check the material to see
   // if it is known (i.e. contained in the range table fRangeTable).
   // If so, then we calculate the energy loss of the particle in this volume,
   // reduce the particle's energy accordingly, and, if the energy falls below
   // a certain cut-off (default 1.e-3 MeV; can be modified with
   // SetCutOffKEForPropagation(Double_t) ), we stop the propagation.
   //
   // The (cumulated) energy losses in the active layers of all hit detectors
   // are updated with the energy lost by this particle

   Double_t de = 0;
   Double_t e = part->GetEnergy();
   if (e <= fCutOffEnergy) {
      e = 0.;
      SetStopPropagation();//propagation will stop after this step
      if (IsTracking()) {
         AddPointToCurrentTrack(GetEntryPoint().X(), GetEntryPoint().Y(), GetEntryPoint().Z());
         AddPointToCurrentTrack(GetExitPoint().X(), GetExitPoint().Y(), GetExitPoint().Z());
      }
      return;
   }

   // calculate energy losses in known materials for charged particles
   TGeoMaterial* material = GetCurrentVolume()->GetMaterial();
   KVIonRangeTableMaterial* irmat = 0;
   if ((irmat = fRangeTable->GetMaterial(material))) {
      de = irmat->GetLinearDeltaEOfIon(
              part->GetZ(), part->GetA(), e, GetStepSize(), 0.,
              material->GetTemperature(),
              material->GetPressure());
      e -= de;
      if (e <= fCutOffEnergy) {
         e = 0.;
         SetStopPropagation();//propagation will stop after this step
      }
      //set flag to say that particle has been slowed down
      part->SetIsDetected();
      //If this is the first absorber that the particle crosses, we set a "reminder" of its
      //initial energy
      if (!part->GetPInitial()) part->SetE0();

      TString absorber_name;
      KVDetector* theDet = GetDetectorFromPath(GetCurrentPath());
      Bool_t active_layer = kFALSE;
      if (theDet) {
         if (!theDet->IsSingleLayer()) {
            absorber_name.Form("%s/%s", theDet->GetName(), GetCurrentNode()->GetName());
            if (strncmp(GetCurrentNode()->GetName(), "ACTIVE", 6) == 0) active_layer = kTRUE;
         }
         else {
            absorber_name = theDet->GetName();
            active_layer = kTRUE;
         }
      }
      else
         absorber_name = irmat->GetName();

      if (part->GetZ()) {
         part->GetParameters()->SetValue(Form("DE:%s", absorber_name.Data()), de);
         if (active_layer) {
            // update energy loss in active layer of detector
            Double_t E = theDet->GetEnergyLoss() + de;
            theDet->SetEnergyLoss(E);
            //theDet->AddHit(part);//don't put a reference to simulated particle in detector
         }
      }
      //part->GetParameters()->SetValue(Form("Xin:%s", absorber_name.Data()), GetEntryPoint().X());
      //part->GetParameters()->SetValue(Form("Yin:%s", absorber_name.Data()), GetEntryPoint().Y());
      //part->GetParameters()->SetValue(Form("Zin:%s", absorber_name.Data()), GetEntryPoint().Z());
      if (StopPropagation()) {
         // If particle stops in this volume, we use as 'exit point' the point corresponding to
         // the calculated range of the particle
         Double_t r = irmat->GetRangeOfLastDE() / irmat->GetDensity();
         TVector3 path = GetExitPoint() - GetEntryPoint();
         TVector3 midVol = GetEntryPoint() + (r / path.Mag()) * path;
         //part->GetParameters()->SetValue(Form("Xout:%s", absorber_name.Data()), midVol.X());
         //part->GetParameters()->SetValue(Form("Yout:%s", absorber_name.Data()), midVol.Y());
         //part->GetParameters()->SetValue(Form("Zout:%s", absorber_name.Data()), midVol.Z());
         if (IsTracking()) {
            AddPointToCurrentTrack(GetEntryPoint().X(), GetEntryPoint().Y(), GetEntryPoint().Z());
            AddPointToCurrentTrack(midVol.X(), midVol.Y(), midVol.Z());
         }
      }
      else {
         //part->GetParameters()->SetValue(Form("Xout:%s", absorber_name.Data()), GetExitPoint().X());
         //part->GetParameters()->SetValue(Form("Yout:%s", absorber_name.Data()), GetExitPoint().Y());
         //part->GetParameters()->SetValue(Form("Zout:%s", absorber_name.Data()), GetExitPoint().Z());
         if (IsTracking()) {
            AddPointToCurrentTrack(GetEntryPoint().X(), GetEntryPoint().Y(), GetEntryPoint().Z());
            AddPointToCurrentTrack(GetExitPoint().X(), GetExitPoint().Y(), GetExitPoint().Z());
         }
      }
      part->SetEnergy(e);
   }
}

void KVRangeTableGeoNavigator::InitialiseTrack(KVNucleus* part, TVector3* TheOrigin)
{
   // Start a new track to visualise trajectory of nucleus through the array

   fTrackTime = 0.;
   Int_t pdg = part->GetN() * 100 + part->GetZ();
   gGeoManager->SetPdgName(pdg, part->GetSymbol());
   Int_t itrack = gGeoManager->AddTrack(GetTrackID(), pdg, part);
   IncrementTrackID();
   fCurrentTrack = gGeoManager->GetTrack(itrack);
   if (TheOrigin) AddPointToCurrentTrack(TheOrigin->x(), TheOrigin->y(), TheOrigin->z());
   else AddPointToCurrentTrack(0, 0, 0);
}

void KVRangeTableGeoNavigator::PropagateParticle(KVNucleus* part, TVector3* TheOrigin)
{
   // Slight modification of KVGeoNavigator::PropagateParticle:
   //   if particle hits a DEADZONE, set its energy to zero
   // We start a new track to represent the particle's trajectory through the array

   if (IsTracking()) InitialiseTrack(part, TheOrigin);

   KVGeoNavigator::PropagateParticle(part, TheOrigin);

   if (part->GetParameters()->HasParameter("DEADZONE")) {
      //Info("PropagateParticle","stopped in DEADZONE:%s",part->GetParameters()->GetStringValue("DEADZONE"));
      //part->SetEnergy(0);
      if (IsTracking()) {
         AddPointToCurrentTrack(GetEntryPoint().X(), GetEntryPoint().Y(), GetEntryPoint().Z());
      }
   }
}
