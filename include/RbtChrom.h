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

// Aggregate class for managing a vector of RbtChromElement objects
// Also implements the RbtChromElement interface itself
// to delegate requests to each element in the vector
// NOTE: RbtChrom destructor is responsible for deleting all managed elements
#ifndef RBTCHROM_H_
#define RBTCHROM_H_

#include "RbtChromElement.h"
#include "RbtError.h"
#include "RbtModel.h"

class RbtChrom : public RbtChromElement {
public:
  // Class type string
  static std::string _CT;
  // Constructor for an empty chromosome
  RBTDLL_EXPORT RbtChrom();
  // Constructor for a combined chromosome for each model
  // in the list
  RbtChrom(const RbtModelList &modelList);
  virtual ~RbtChrom();

  // pure virtual from RbtChromElement
  virtual void Reset();
  virtual void Randomise();
  virtual void Mutate(double relStepSize);
  virtual void SyncFromModel();
  virtual void SyncToModel();
  virtual RbtChromElement *clone() const;
  virtual int GetLength() const;
  virtual int GetXOverLength() const;
  virtual void GetVector(std::vector<double> &v) const;
  virtual void GetVector(RbtXOverList &v) const;
  virtual void SetVector(const std::vector<double> &v, int &i);
  virtual void SetVector(const RbtXOverList &v, int &i);
  virtual void GetStepVector(std::vector<double> &v) const;
  virtual double CompareVector(const std::vector<double> &v, int &i) const;
  virtual void Print(std::ostream &s) const;

  // Aggregate methods
  // Appends a new chromosome element to the vector
  // RbtChrom destructor is responsible for deleting the new element
  // Null operation if pChromElement is nullptr
  virtual void Add(RbtChromElement *pChromElement);

protected:
private:
  RbtChromElementList m_elementList;
  // We need to store the model list so that
  // we can call UpdatePseudoAtoms() on each model following
  // a SyncToModel
  RbtModelList m_modelList;
};

#endif /*RBTCHROM_H_*/
