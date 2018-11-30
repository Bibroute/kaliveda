//Created by KVClassFactory on Fri Feb 26 14:22:01 2016
//Author: bonnet,,,

#include "KVFAZIETO.h"
#include "KVUnits.h"
#include "KVFAZIABlock.h"

#include <KVDataSet.h>

ClassImp(KVFAZIETO)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIETO</h2>
<h4>description of the FAZIA-12B demonstrator</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIETO::KVFAZIETO()
   : KVFAZIA()
{
   // Default constructor
   SetTitle(ClassName());
}

KVFAZIETO::~KVFAZIETO()
{
   // Destructor
}

void KVFAZIETO::GetGeometryParameters()
{
   //defined configuration of the blocks
   fNblocks = KVBase::GetDataSetEnv(fDataSet, "FAZIA.NBlocks", 12.0);
   fFDist = 100.0;
   fFThetaMin = 1.5;
   SetGeometryImportParameters(.25, 1., 1.5, 0, 12);
}


//________________________________________________________________

void KVFAZIETO::BuildFAZIA()
{
   // Number of blocks in demonstrator, their numbers, and positions can be modified for
   // different datasets using the following environment variables:
   //
   // [dataset].FAZIA.NBlocks:  3
   // [dataset].FAZIA.BlockNumbers: 0,1,4
   // [dataset].FAZIA.BlockPositions: 11,9,5
   //
   // This will build a 3 block demonstrator with blocks numbered 0, 1, and 4
   // which will be placed at the given block positions in the standard
   // 12-block geometry (where blocks are numbered clockwise starting from
   // the innermost block placed at roughly 12 o'clock)

   Info("BuildFAZIA", "Compact geometry, %f cm from target",
        fFDist);

   TGeoVolume* top = gGeoManager->GetTopVolume();

   Double_t distance_block_cible = fFDist * KVUnits::cm;
   Double_t thick_si1 = 300 * KVUnits::um;
   TGeoTranslation trans;
   trans.SetDz(distance_block_cible + thick_si1 / 2.);

   KVFAZIABlock* block = new KVFAZIABlock;

   TGeoRotation rot1, rot2;
   TGeoHMatrix h;
   TGeoHMatrix* ph = 0;
   Double_t theta = 0;
   Double_t phi = 0;

   Double_t theta_min = fFThetaMin;//smallest lab polar angle in degrees
   Double_t centre_hole = 2.*tan(theta_min * TMath::DegToRad()) * distance_block_cible;
   Double_t dx = (block->GetTotalSideWithBlindage()) / 2.;

   TVector3 centre;
   // block numbering and positions
   KVString block_numbers = KVBase::GetDataSetEnv(fDataSet, "FAZIA.BlockNumbers", "0,1,2,3,4,5,6,7,8,9,10,11");
   block_numbers.Begin(",");
   KVString block_positions = KVBase::GetDataSetEnv(fDataSet, "FAZIA.BlockPositions", "0,1,2,3,4,5,6,7,8,9,10,11");
   block_positions.Begin(",");

   for (Int_t i = 0; i < fNblocks; i += 1) {

      Int_t block_number = block_numbers.Next().Atoi();
      Int_t bb = block_positions.Next().Atoi();

      //for FAZIASYM ordering
      if (bb == 3)      centre.SetXYZ(-1 * (dx     - centre_hole / 2), 1 * (-dx    - centre_hole / 2),  distance_block_cible);
      else if (bb == 2) centre.SetXYZ(-1 * (dx     + centre_hole / 2), 1 * (dx     - centre_hole / 2),  distance_block_cible);
      else if (bb == 1) centre.SetXYZ(-1 * (-dx    + centre_hole / 2), 1 * (dx     + centre_hole / 2),  distance_block_cible);
      else if (bb == 0) centre.SetXYZ(-1 * (-dx    - centre_hole / 2), 1 * (-dx    + centre_hole / 2),  distance_block_cible);
      else if (bb == 10) centre.SetXYZ(-1 * (3 * dx   - centre_hole / 2),  1 * (-dx   - centre_hole / 2),  distance_block_cible);
      else if (bb == 11) centre.SetXYZ(-1 * (dx     - centre_hole / 2),  1 * (-3 * dx - centre_hole / 2),  distance_block_cible);
      else if (bb == 9) centre.SetXYZ(-1 * (3 * dx   + centre_hole / 2),  1 * (dx    - centre_hole / 2),  distance_block_cible);
      else if (bb == 8) centre.SetXYZ(-1 * (dx     + centre_hole / 2),  1 * (3 * dx  - centre_hole / 2),  distance_block_cible);
      else if (bb == 7) centre.SetXYZ(-1 * (-dx    + centre_hole / 2),  1 * (3 * dx  + centre_hole / 2),  distance_block_cible);
      else if (bb == 6) centre.SetXYZ(-1 * (-3 * dx  + centre_hole / 2),  1 * (dx    + centre_hole / 2),  distance_block_cible);
      else if (bb == 5)centre.SetXYZ(-1 * (-3 * dx  - centre_hole / 2), 1 * (-dx    + centre_hole / 2),  distance_block_cible);
      else if (bb == 4)centre.SetXYZ(-1 * (-dx    - centre_hole / 2),  1 * (-3 * dx + centre_hole / 2),  distance_block_cible);


      else {
         Warning("BuildFAZIA", "Block position definition is done only for %d blocks", fNblocks);
      }
      theta = centre.Theta() * TMath::RadToDeg();
      phi = centre.Phi() * TMath::RadToDeg();
      printf("BLK #%d => theta=%1.2lf - phi=%1.2lf\n", block_number, theta, phi);

      rot2.SetAngles(phi + 90., theta, 0.);
      rot1.SetAngles(-1.*phi, 0., 0.);
      h = rot2 * trans * rot1;
      ph = new TGeoHMatrix(h);
      top->AddNode(block, block_number, ph);
   }

}
/*
TVector3 cent_b1(dx - centre_hole / 2, -dx - centre_hole / 2, distance_block_cible); //position centre of block 1
TVector3 cent_b2(dx + centre_hole / 2 , dx - centre_hole / 2, distance_block_cible); //position centre
TVector3 cent_b3(-dx + centre_hole / 2, dx + centre_hole / 2, distance_block_cible); //position centre of block 3
TVector3 cent_b4(-dx - centre_hole / 2, -dx + centre_hole / 2, distance_block_cible); //position centre of block 4

TVector3 cent_b5(3*dx   - centre_hole/2,  -dx   - centre_hole / 2,            distance_block_cible); //position centre of block 5
TVector3 cent_b6(dx     - centre_hole/2,  -3*dx - centre_hole / 2,      distance_block_cible); //position centre of block 6
TVector3 cent_b7(3*dx   + centre_hole/2,  dx    - centre_hole / 2,         distance_block_cible); //position centre of block 2
TVector3 cent_b8(dx     + centre_hole/2,  3*dx  - centre_hole / 2,   distance_block_cible); //position centre of block 2

TVector3 cent_b9 (-dx   + centre_hole/2,  3*dx + centre_hole/2,   distance_block_cible); //position centre of block 3
TVector3 cent_b10(-3*dx + centre_hole/2,  dx + centre_hole/2,     distance_block_cible); //position centre of block 3
TVector3 cent_b11(-3*dx - centre_hole/2, -dx + centre_hole/2,     distance_block_cible); //position centre of block 4
TVector3 cent_b12(-dx   - centre_hole/2,  -3*dx + centre_hole/2,  distance_block_cible); //position centre of block 4
*/
