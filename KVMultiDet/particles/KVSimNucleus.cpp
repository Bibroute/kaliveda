//Created by KVClassFactory on Mon Jun 28 15:02:00 2010
//Author: bonnet

#include "KVSimNucleus.h"

ClassImp(KVSimNucleus)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimNucleus</h2>
<h4>Classe d�riv�e de KVNucleus pour g�rer des simulations dans KaliVeda</h4>
<!-- */
// --> END_HTML
//La classe d�rive �galement de KVNameValueList pour pouvoir ajouter autant
// de parametres que possible afin d'enregistrer des propri�t�s des noyaux 
// propre � chaque simulation
//Deux TVector3 sont ajout�s en champs de la classe : 
// - le vecteur position
// - le vecteur moment angulaire
//
//Cette classe est coupl�e � KVSimEvent
////////////////////////////////////////////////////////////////////////////////

//___________________________
KVSimNucleus::KVSimNucleus()
{
	//Constructeur par defaut
}
	
//___________________________
KVSimNucleus::~KVSimNucleus()
{
}

//___________________________
void KVSimNucleus::Copy(TObject& obj) const
{

	//Copy l'object "this" vers obj
	
	KVNucleus::Copy(obj);	
	((KVSimNucleus &)obj).position = position;
	((KVSimNucleus &)obj).angmom = angmom;
}

//___________________________
void KVSimNucleus::SetPosition(Double_t rx, Double_t ry, Double_t rz)
{
	//set the position of the nucleus in position space
	position.SetXYZ(rx,ry,rz);
}

//___________________________
const TVector3* KVSimNucleus::GetPosition() const 
{
	//return the position of the nucleus as TVector3
	return &position; 
}

//___________________________
void KVSimNucleus::SetAngMom(Double_t lx, Double_t ly, Double_t lz)
{
	//set the angular momentum of the nucleus
	angmom.SetXYZ(lx,ly,lz);
}

//___________________________
const TVector3* KVSimNucleus::GetAngMom() const
{
	//return the angular momentum of the nucleus as TVector3
	return &angmom;
}
