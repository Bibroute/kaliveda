/*
$Id: KVElasticScatterEvent.h,v 1.3 2009/01/14 15:35:50 ebonnet Exp $
$Revision: 1.3 $
$Date: 2009/01/14 15:35:50 $
*/

//Created by KVClassFactory on Thu Dec 11 14:45:29 2008
//Author: eric bonnet,,,

#ifndef __KVELASTICSCATTEREVENT_H
#define __KVELASTICSCATTEREVENT_H

#include "KVString.h"
#include "TVector3.h"
#include "KVTarget.h"
#include "KVLayer.h"
#include "KVMaterial.h"
#include "KVNucleus.h"
#include "KVEvent.h"
#include "KV2Body.h"
#include "KVMultiDetArray.h"
#include "KVReconstructedNucleus.h"
#include "KVReconstructedEvent.h"
#include "TObject.h"
#include "TTree.h"
#include "KVHashList.h"
#include "KVBase.h"
#include "KVDBSystem.h"
#include "KVPosition.h"
#include "KVParticle.h"


class KVElasticScatterEvent : public KVBase
{

   protected:
	KV2Body*						kb2;//!
	TVector3 					kIPPVector;//!
	KVTarget*					ktarget;//!
	KVNucleus*					proj;//!->
	KVNucleus*					targ;//!->
	KVReconstructedEvent* 	rec_evt;//!
	KVEvent*						sim_evt; //!
	
	KVHashList*					lhisto;//! to store control histogram
	KVHashList*					ltree;//!	to store tree

	Double_t 					th_min,th_max,phi_min,phi_max;//!
	Double_t						kXruth_evt;//!
	Int_t 						kchoix_layer;//!
	Int_t 						kTreatedNevts;//!		number of diffusion performed
	
	Int_t 						kDiffNuc;//!
	Option_t*					kRandomOpt;//!
	KVPosition					kposalea;//!
	Int_t 						kChoixSol;
	//Variables permettant de traiter les doubles solutions cinematiques
	//pour un theta donne
	/*
	Bool_t 						SecondTurn;
	Double_t 					Ekdiff_ST;
	Double_t						Thdiff_ST;
	Double_t						Phdiff_ST;
	*/
	void		init();
	void 		GenereKV2Body();
	Bool_t 	DefineTargetNucleusFromLayer(KVString layer_name="");
	void 		PropagateInTargetLayer();
	void 		NewInteractionPointInTargetLayer();
	void 		StartEvents();
	virtual void MakeDiffusion();
	void		SortieDeCible();
	
	public:
   
	enum {
      kProjIsSet = BIT(14),     	//kTRUE if projectile nucleus defined
      kTargIsSet = BIT(15),     	//kTRUE if target nucleus defined
      kHasTarget = BIT(16),		//kTRUE if target material defined
      kIsUpdated = BIT(17),		//flag indicating if ValidateEntrance method has to be called
      kIsDetectionOn = BIT(18)	//flag indicating if user asked detection of events
   };
	
	Bool_t IsProjNucSet() {  return TestBit(kProjIsSet); }
	Bool_t IsTargNucSet() {  return TestBit(kTargIsSet); }
	Bool_t IsTargMatSet() {  return TestBit(kHasTarget); }
 	Bool_t IsUpdated()	{  return TestBit(kIsUpdated); }
 	Bool_t IsDetectionOn() {  return TestBit(kIsDetectionOn); }
	
	void ChooseKinSol(Int_t choix=1);
  
	KVElasticScatterEvent();
   virtual ~KVElasticScatterEvent();
	
	virtual void SetSystem(KVDBSystem* sys);
	virtual void SetSystem(Int_t zp,Int_t ap,Double_t ekin,Int_t zt,Int_t at);
	virtual void SetTargNucleus(KVNucleus *nuc);
	virtual void SetProjNucleus(KVNucleus *nuc);
	
	void SetDetectionOn(Bool_t On=kTRUE);
	
	
	KVNucleus* GetNucleus(const Char_t* name) const;
	KVNucleus* GetNucleus(Int_t ii) const;
	
	virtual void SetTargetMaterial(KVTarget *targ,Bool_t IsRandomized=kTRUE);
	KVTarget* GetTarget() const;
	
	virtual TVector3& GetInteractionPointInTargetLayer();
	
	KVReconstructedNucleus* GetReconstructedNucleus(KVString nucname){
		return GetReconstructedEvent()->GetParticleWithName(nucname);
	}
	KVReconstructedEvent* GetReconstructedEvent(void) const {
		return rec_evt;
	}
	KVEvent* GetSimEvent(void) const {
		return sim_evt;
	}
	
	virtual void Reset();
	virtual Bool_t ValidateEntrance();
	
	void SetDiffNucleus(KVString name="PROJ");
	void SetRandomOption(Option_t* opt="isotropic");
	Bool_t IsIsotropic();
	
	virtual void Process(Int_t ntimes=1,Bool_t reset=kTRUE);
	
	virtual void SetAnglesForDiffusion(Double_t theta,Double_t phi);
	virtual void Filter();
	virtual void TreateEvent();
	
	virtual void DefineAngularRange(TObject* );
	void DefineAngularRange(Double_t tmin, Double_t tmax, Double_t pmin,Double_t pmax);
	Double_t GetTheta(KVString opt) const;
	Double_t GetPhi(KVString opt)const;
	
	KV2Body* GetKV2Body() {return kb2;}
	void Print();
	
	virtual void ClearTrees();
	virtual void ResetTrees();
	virtual void DefineTrees();
	KVHashList* GetTrees() const;

	virtual void ClearHistos();
	virtual void DefineHistos();
	virtual void ResetHistos();
	KVHashList* GetHistos() const;
	
	ClassDef(KVElasticScatterEvent,1)//simulate ElasticScatterEvent and answer of a given (multi-)detector
};

#endif
