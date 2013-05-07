//Created by KVClassFactory on Tue May  7 09:22:02 2013
//Author: John Frankland,,,

#ifndef __KVGEOSTRUCELEMENT_H
#define __KVGEOSTRUCELEMENT_H

#include "KVBase.h"
#include "KVUniqueNameList.h"
#include "KVDetector.h"

class KVGeoStrucElement : public KVBase
{
    void init();

protected:
    KVUniqueNameList fDetectors;       //detectors in this structure element
    KVUniqueNameList fStructures;      //daughter structures
    KVUniqueNameList fParentStrucList;         //parent structures

    void AddParentStructure(KVGeoStrucElement*);
    void RemoveParentStructure(KVGeoStrucElement*);

public:
    KVGeoStrucElement();
    KVGeoStrucElement(const Char_t* name, const Char_t* title = "");
    virtual ~KVGeoStrucElement();

    virtual void Add(KVBase*);
    virtual void Remove(KVBase*);

    void Clear(Option_t *opt = "");

    virtual Bool_t Contains(KVBase *name) const
    {
        // Returns kTRUE if this structure contains the detector or structure
        // given as argument

        KVDetector* det = (KVDetector*)GetDetectors()->FindObject(name);
        if(det) return kTRUE;
        return (GetStructures()->FindObject(name)!=0);
    }

    KVGeoStrucElement* GetStructure(const Char_t* name)
    {
        // Return daughter structure with given name
        return (KVGeoStrucElement*)fStructures.FindObject(name);
    }
    KVGeoStrucElement* GetStructure(const Char_t *type, const Char_t *name) const;
    KVDetector* GetDetector(const Char_t* name) const
    {
        // Return detector in this structure with given name
        return (KVDetector*)fDetectors.FindObject(name);
    }
    KVDetector* GetDetectorAny(const Char_t* name)
    {
        // Return detector in structure with given name.
        // If not found in this structure, we look in all daughter structures
        // for a detector with the name.

        KVDetector* det = (KVDetector*)GetDetectors()->FindObject(name);
        if(!det){
            TIter next(GetStructures());
            KVGeoStrucElement* elem;
            while( (elem = (KVGeoStrucElement*)next()) ){
                det = elem->GetDetectorAny(name);
                if(det) return det;
            }
        }
        return 0x0;
    }
    const KVSeqCollection* GetDetectors() const
    {
        return &fDetectors;
    }
    const KVSeqCollection* GetStructures() const
    {
        return &fStructures;
    }
    const KVSeqCollection* GetParents() const
    {
        return &fParentStrucList;
    }
    Bool_t Fired(Option_t* opt = "any") const
    {
        // Returns kTRUE if any detector or structure element in this structure
        // has 'Fired' with the given option

        TIter nextd(GetDetectors());
        KVDetector* det;
        while( (det = (KVDetector*)nextd()) ) if(det->Fired(opt)) return kTRUE;
        TIter nexts(GetStructures());
        KVGeoStrucElement* elem;
        while( (elem = (KVGeoStrucElement*)nexts()) ) if(elem->Fired(opt)) return kTRUE;
        return kFALSE;
    }
    KVGeoStrucElement *GetParentStructure(const Char_t *type, const Char_t *name="") const;

    void Print(Option_t *option="") const;

    ClassDef(KVGeoStrucElement,1)//A multidetector array geometry structural element
};

#endif
