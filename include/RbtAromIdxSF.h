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

// Indexed-grid-based intermolecular aromatic scoring function

#ifndef _RBTAROMIDXSF_H_
#define _RBTAROMIDXSF_H_

#include "RbtAnnotationHandler.h"
#include "RbtBaseIdxSF.h"
#include "RbtBaseInterSF.h"
#include "RbtPlane.h"

class RbtAromIdxSF : public RbtBaseInterSF,
                     public RbtBaseIdxSF,
                     public RbtAnnotationHandler {
public:
  // Class type string
  static std::string _CT;
  // Parameter names
  static std::string _INCR;
  static std::string _R12;
  static std::string _DR12MIN;
  static std::string _DR12MAX;
  static std::string _DAMIN;
  static std::string _DAMAX;
  // DM 12 Jun 2002 - score threshold used for counting aromatic interactions
  static std::string _THRESHOLD;

  RbtAromIdxSF(const std::string &strName = "AROM");
  virtual ~RbtAromIdxSF();

  // Override RbtBaseSF::ScoreMap to provide additional raw descriptors
  // virtual void ScoreMap(RbtStringVariantMap& scoreMap) const;

protected:
  virtual void SetupReceptor();
  virtual void SetupLigand();
  virtual void SetupScore();
  virtual double RawScore() const;

  // Clear the receptor and ligand grids and lists respectively
  // As we are not using smart pointers, there is some memory management to do
  void ClearReceptor();
  void ClearLigand();

  // DM 25 Oct 2000 - track changes to parameter values in local data members
  // ParameterUpdated is invoked by RbtParamHandler::SetParameter
  void ParameterUpdated(const std::string &strName);

private:
  //////////////////////////////////////////////////////////
  // DM 6 Feb 2003
  // To be consistent with the Polar score, we should really define a base
  // RbtAromSF class which will provide the scoring function primitives, then
  // have an RbtAromIdxSF subclass for intermolecular aromatic interactions, and
  // a RbtAromIntraSF subclass for ligand intramolecular interactions. Currently
  // there is no RbtAromIntraSF class so everything is contained within
  // RbtAromIdxSF

  // Generic scoring function params
  struct f1prms {
    double R0, DRMin, DRMax, slope;
    f1prms(double R, double DMin, double DMax)
        : R0(R), DRMin(DMin), DRMax(DMax), slope(1.0 / (DMax - DMin)) {}
  };

  inline f1prms GetRprms() const { return f1prms(m_R12, m_DR12Min, m_DR12Max); }
  inline f1prms GetAprms() const { return f1prms(0.0, m_DAMin, m_DAMax); }

  // Generic scoring function primitive (deviation from ideal geometry)
  inline double f1(double DR, const f1prms &prms) const {
    return (DR > prms.DRMax)
               ? 0.0
               : (DR > prms.DRMin) ? 1.0 - prms.slope * (DR - prms.DRMin) : 1.0;
  }

  // The actual aromatic score, between a given interaction center and a list of
  // near neighbour centers
  double AromScore(const RbtInteractionCenter *pIC1,
                   const RbtInteractionCenterList &IC2List, const f1prms &Rprms,
                   const f1prms &Aprms) const;
  double PiScore(const RbtInteractionCenter *pIC1,
                 const RbtInteractionCenterList &IC2List) const;
  // End of section that should ultimately be moved to RbtAromSF base class
  //////////////////////////////////////////////////////////

  RbtInteractionGridPtr m_spAromGrid;
  RbtInteractionGridPtr m_spGuanGrid;
  RbtInteractionCenterList m_recepAromList;
  RbtInteractionCenterList m_recepGuanList;
  RbtInteractionCenterList m_ligAromList;
  RbtInteractionCenterList m_ligGuanList;

  // DM 25 Oct 2000 - local copies of params
  double m_R12;
  double m_DR12Min;
  double m_DR12Max;
  double m_DAMin;
  double m_DAMax;

  // DM 12 Jun 2002 - keep track of number of ligand rings and guan carbons
  // involved in non-zero arom interactions
  mutable int m_nArom;
  mutable int m_nGuan;
  mutable double m_ss; // Sigma-sigma score
  mutable double m_sp; // Sigma-pi score
  mutable double m_pp; // pi-pi score
  double m_threshold;
};

#endif //_RBTAROMIDXSF_H_
