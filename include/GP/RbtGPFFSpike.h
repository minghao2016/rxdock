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

// GPFFSpike Class. GPFFSpike represents a fitness function
// valid for the CGP. It is used to train with the Spike set.

#ifndef _RBT_GPFFSPIKE_H_
#define _RBT_GPFFSPIKE_H_

#include "RbtContext.h"
#include "RbtGPFitnessFunction.h"
#include "RbtGPGenome.h"
#include "RbtGPTypes.h"
#include "RbtRand.h"
// #include <cmath>

class RbtGPFFSpike : public RbtGPFitnessFunction {
public:
  static std::string _CT;
  RbtGPFFSpike(RbtContextPtr c) : contextp(c) {
    _RBTOBJECTCOUNTER_CONSTR_(_CT);
  }
  virtual ~RbtGPFFSpike() { _RBTOBJECTCOUNTER_DESTR_(_CT); }

  void ReadTables(std::istream &, RbtReturnTypeArray &, RbtReturnTypeArray &);
  double CalculateFitness(RbtGPGenomePtr, RbtReturnTypeArray &,
                          RbtReturnTypeArray &, double, bool);
  double CalculateFitness(RbtGPGenomePtr, RbtReturnTypeArray &,
                          RbtReturnTypeArray &, bool);

private:
  void CreateRandomCtes(int);
  std::vector<double> ctes;
  RbtCellContextPtr contextp;
  RbtGPFFSpike(); // default constructor disabled
};

// Useful typedefs
typedef SmartPtr<RbtGPFFSpike> RbtGPFFSpikePtr; // Smart pointer
typedef std::vector<RbtGPFFSpikePtr>
    RbtGPFFSpikeList; // Vector of smart pointers
typedef RbtGPFFSpikeList::iterator RbtGPFFSpikeListIter;
typedef RbtGPFFSpikeList::const_iterator RbtGPFFSpikeListConstIter;
#endif //_RbtGPFFSpike
