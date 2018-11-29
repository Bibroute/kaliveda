//Created by KVClassFactory on Fri Jul 20 15:49:04 2018
//Author: eindra

#ifndef __KVEXPSETUPDB_H
#define __KVEXPSETUPDB_H

#include "KVExpDB.h"

class KVExpSetUpDB : public KVExpDB {

public:
   KVExpSetUpDB();
   KVExpSetUpDB(const Char_t* name);
   KVExpSetUpDB(const Char_t* name, const Char_t* title);

   virtual ~KVExpSetUpDB();

   void Build();
   void FillRunsTable();

   ClassDef(KVExpSetUpDB, 1) //Database for dataset using KVExpSetUp
};

#endif