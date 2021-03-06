//Created by KVClassFactory on Thu Oct  8 10:52:39 2015
//Author: John Frankland,,,

#ifndef __KVRECONNUCTRAJECTORY_H
#define __KVRECONNUCTRAJECTORY_H

#include "KVGeoDNTrajectory.h"
#ifdef WITH_CPP11
#include <unordered_map>
#else
#include <map>
#endif

class KVReconNucTrajectory : public KVGeoDNTrajectory {

   Int_t fIndependentIdentifications;//number of independent ID telescopes on trajectory
#ifdef WITH_CPP11
   std::unordered_map<std::string, KVDetector*> fDetLabels; //! map detector labels to detectors
#else
   std::map<std::string, KVDetector*> fDetLabels; //! map detector labels to detectors
#endif

public:
   KVReconNucTrajectory() : KVGeoDNTrajectory(), fIndependentIdentifications(0)
   {
      fAddToNodes = kFALSE;
   }
   KVReconNucTrajectory(const KVReconNucTrajectory&);
   KVReconNucTrajectory(const KVGeoDNTrajectory*, const KVGeoDetectorNode*);
   virtual ~KVReconNucTrajectory() {}
   KVReconNucTrajectory& operator=(const KVReconNucTrajectory&);

   void Copy(TObject& obj) const;

   Int_t GetNumberOfIndependentIdentifications() const
   {
      // Return number of independent ID telescopes on trajectory
      return fIndependentIdentifications;
   }
   void ls(Option_t* = "") const;
   KVDetector* GetDetector(const TString& label) const;

   ClassDef(KVReconNucTrajectory, 1) //Reconstructed trajectory of nucleus detected by array
};

#endif
