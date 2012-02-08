//Created by KVClassFactory on Tue Feb  7 16:49:15 2012
//Author: bonnet

#ifndef __KVGAUS2D_H
#define __KVGAUS2D_H

#include "KVAutoFit.h"

class KVGaus2D : public KVAutoFit
{

   public:
   KVGaus2D();
   KVGaus2D (const KVGaus2D&) ;
   virtual ~KVGaus2D();
	
	virtual Bool_t NewFunction_2D();
	virtual Double_t f2D(Double_t *xx,Double_t *para);	
	
	virtual void Save();
	
   ClassDef(KVGaus2D,1)//Fit gaussien a deux dimensions
};

#endif
