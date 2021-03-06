/***********************************************************************
 * The rDock program was developed from 1998 - 2006 by the software team
 * at RiboTargets (subsequently Vernalis (R&D) Ltd).
 * In 2006, the software was licensed to the University of York for
 * maintenance and distribution.
 * In 2012, Vernalis and the University of York agreed to release the
 * program as Open Source software.
 * This version is licensed under GNU-LGPL version 3.0 with support from
 * the University of Barcelona.
 * http://rdock.sourceforge.net/
 ***********************************************************************/

// Stores the information that is global to the interpreter
// in this case, values of ctes, names of variables,...
// It basically contains a map of variables, class RbtVble is also defined here

#ifndef _RBT_CONTEXT_H_
#define _RBT_CONTEXT_H_

#include "RbtBaseSF.h"
#include "RbtDockingSite.h"
#include "RbtGPTypes.h"
#include "RbtModel.h"
#include "RbtVble.h"
#include <fstream>

typedef std::map<std::string, RbtVblePtr> RbtStringVbleMap; // Map of Vbles
typedef std::map<int, RbtVblePtr> RbtIntVbleMap;            // Map of Vbles
typedef RbtStringVbleMap::iterator RbtStringVbleMapIter;
typedef RbtIntVbleMap::iterator RbtIntVbleMapIter;

class RbtContext {
public:
  static std::string _CT;
  ///////////////////
  // Constructors
  ///////////////////
  RbtContext(const RbtContext &c);
  RbtContext(); // default constructor disabled
                ///////////////////
                // Destructors
                ///////////////////
  virtual ~RbtContext();
  virtual void Assign(std::string, RbtReturnType) = 0;
  virtual void Assign(int, RbtReturnType) = 0;
  virtual const RbtVble &GetVble(int) = 0;
  virtual const RbtVble &GetVble(std::string) = 0;
  virtual void SetVble(int key, const RbtVble &v) = 0;
  //    virtual RbtString GetName(RbtInt)=0;
  //    virtual RbtReturnType GetValue(RbtInt)=0;
  //    virtual RbtString GetName(RbtString)=0;
  //    virtual RbtReturnType GetValue(RbtString)=0;
};

class RbtCellContext : public RbtContext {
public:
  // static RbtString _CT;
  RbtCellContext(std::ifstream &ifile);
  RbtCellContext();
  RbtCellContext(const RbtCellContext &c);
  virtual ~RbtCellContext();
  void Assign(int key, RbtReturnType val) {
    RbtIntVbleMapIter it = vm.find(key);
    if (it != vm.end())
      vm[key]->SetValue(val);
    else {
      vm[key] = new RbtVble(std::to_string(val), val);
    }
  }
  void Assign(std::string s, RbtReturnType val) {
    throw RbtError(_WHERE_, "This is not a string context");
  }
  //    RbtString GetName(RbtInt key) { return vm[key].GetName();}
  //    RbtReturnType GetValue(RbtInt key) {return vm[key].GetValue();}
  //    RbtString GetName(RbtString){return "";}
  //    RbtReturnType GetValue(RbtString){return 0.0;}
  const RbtVble &GetVble(int key) { return *(vm[key]); }
  void SetVble(int key, const RbtVble &v) { *(vm[key]) = v; }
  const RbtVble &GetVble(std::string key) {
    throw RbtError(_WHERE_, "This is not a string context");
  }
  //    void Clear();

private:
  RbtIntVbleMap vm;
  int ninputs;
};

class RbtStringContext : public RbtContext {

public:
  RbtStringContext();
  RbtStringContext(SmartPtr<std::ifstream> ifile);
  RbtStringContext(const RbtStringContext &c);
  virtual ~RbtStringContext();
  void Assign(std::string key, RbtReturnType val) {
    RbtStringVbleMapIter it = vm.find(key);
    if (it != vm.end())
      vm[key]->SetValue(val);
    else {
      vm[key] = new RbtVble(key, val);
    }
  }
  void Assign(int i, RbtReturnType val) {
    throw RbtError(_WHERE_, "This is not a cell context");
  }

  //    RbtString GetName(RbtInt){return "";}
  //    RbtReturnType GetValue(RbtInt){return 0.0;}
  //    RbtString GetName(RbtString key) { return vm[key].GetName();}
  //    RbtReturnType GetValue(RbtString key) {return vm[key].GetValue();}
  double Get(RbtModelPtr lig, std::string name);
  double Get(RbtModelPtr rec, RbtDockingSitePtr site, std::string name);
  double Get(RbtBaseSF *spSF, std::string name, RbtModelPtr lig);
  const RbtVble &GetVble(std::string key) { return *(vm[key]); }
  const RbtVble &GetVble(int key) {
    throw RbtError(_WHERE_, "This is not a cell context");
  }
  void SetVble(int key, const RbtVble &v) { *(vm[""]) = v; }
  void UpdateLigs(RbtModelPtr lig);
  void UpdateSite(RbtModelPtr rec, RbtDockingSitePtr site);
  void UpdateScores(RbtBaseSF *spSF, RbtModelPtr lig);

private:
  RbtStringVbleMap vm;
};

// Useful typedefs
typedef SmartPtr<RbtCellContext> RbtCellContextPtr;        // Smart pointer
typedef std::vector<RbtCellContextPtr> RbtCellContextList; // Vector of smart
                                                           // pointers
typedef RbtCellContextList::iterator RbtCellContextListIter;
typedef RbtCellContextList::const_iterator RbtCellContextListConstIter;
typedef SmartPtr<RbtStringContext> RbtStringContextPtr; // Smart pointer
typedef std::vector<RbtStringContextPtr>
    RbtStringContextList; // Vector of smart pointers
typedef RbtStringContextList::iterator RbtStringContextListIter;
typedef RbtStringContextList::const_iterator RbtStringContextListConstIter;
typedef SmartPtr<RbtContext> RbtContextPtr;        // Smart pointer
typedef std::vector<RbtContextPtr> RbtContextList; // Vector of smart pointers
typedef RbtContextList::iterator RbtContextListIter;
typedef RbtContextList::const_iterator RbtContextListConstIter;

#endif //_RbtContext_H_
