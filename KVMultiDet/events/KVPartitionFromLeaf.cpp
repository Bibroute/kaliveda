//Created by KVClassFactory on Fri May  7 11:30:10 2010
//Author: bonnet

#include "KVPartitionFromLeaf.h"
#include "TMath.h"

ClassImp(KVPartitionFromLeaf)

////////////////////////////////////////////////////////////////////////////////
/*
BEGIN_HTML
<h2>KVPartitionFromLeaf</h2>
<h4>Derived form KVPartition to optimize filling from a tree</h4>
END_HTML
- Utilisation avec un arbre contenant des branches de ce type
avec par evt un tableau de valeur de dimension fixe ou non
Exemple:
OBJ: TTree	tree	tree : 0 at: 0x9cfc7a8
 OBJ: TBranch	mt	mt/I : 0 at: 0x9cfcdc8
 OBJ: TBranch	tabz	tabz[mt]/I : 0 at: 0x9d00120

- Methode ConnectToLeaf(TLeaf* lf) permet de lier la KVPartition a la branche tableau (tree->GetLeaf("tabz"))
Exemple:
KVPartitionFromLeaf* par = new KVPartitionFromLeaf();
par->ConnectToLeaf(tree->GetLeaf("tabz"));

- Methode KVPartitionFromLeaf::SetZminZmax permet de definir des limites inf ou sup des tailles des fragments lors du remplissage de la partition, � definir
avant d'utiliser la methode Update();

- M�thode KVPartitionFromLeaf::Update(Long64_t entry), jour le role de remplissage de la partition et aussi du calcul des valeurs associ�es
On peut incrementer les entrees de l'arbre en meme temps ou non (defaut)
Exemple:
for (Int_t ii=0;ii<tt->GetEntries();ii+=1){

	tree->GetEntry(ii);
	...
	...
	par->Update();

}
ou
for (Int_t ii=0;ii<tt->GetEntries();ii+=1){

	par->Update(ii);

}


*/
////////////////////////////////////////////////////////////////////////////////

KVPartitionFromLeaf::KVPartitionFromLeaf()
{
   // Default constructor
	linked_leaf=0;
	zmin=-1;
	zmax=-1;
	select_min = kFALSE;
	select_max = kFALSE;
	
}

KVPartitionFromLeaf::~KVPartitionFromLeaf()
{
   // Destructor
}

void KVPartitionFromLeaf::ConnectToLeaf(TLeaf* lf){

	linked_leaf=lf;
	Int_t cmax;
	(lf->GetLeafCount() ? cmax = lf->GetLeafCount()->GetMaximum() : cmax = lf->GetNdata() );
	if (cmax>GetValMax())
		Warning("ConnectToLeaf","La dimension max du tableau %d est plus grande que KVPartition::GetValMax()",cmax,GetValMax());
	

}

void KVPartitionFromLeaf::Update(Long64_t entry){
	
	Reset();
	if (entry!=-1) linked_leaf->GetBranch()->GetEntry(entry);
	
	Int_t mult;
	(linked_leaf->GetLeafCount() ? mult = linked_leaf->GetLeafCount()->GetValue(0) : mult = linked_leaf->GetNdata() );
	
	for (Int_t mm=0;mm<mult;mm+=1){
		Int_t val = TMath::Nint(linked_leaf->GetValue(mm));
		if ( ( !select_min || (select_min && val>=zmin) ) && ( !select_max || (select_max && val<=zmax) ) ) 
			AddToRegle(val);
	}
	Compute();
	
}

void KVPartitionFromLeaf::SetZmin(Int_t val){
	
	zmin = val;
	select_min = ( (zmin==-1) ? kFALSE : kTRUE);

}
void KVPartitionFromLeaf::SetZmax(Int_t val){
	
	zmax = val;
	select_max = ( (zmax==-1) ? kFALSE : kTRUE);

}
void KVPartitionFromLeaf::SetZminZmax(Int_t vmin,Int_t vmax){
	
	SetZmin(vmin);
	SetZmax(vmax);

}
