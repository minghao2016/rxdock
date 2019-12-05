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

// Base implementation class for all vdW scoring functions

#ifndef _RBTVDWSF_H_
#define _RBTVDWSF_H_

#include "RbtAnnotationHandler.h"
#include "RbtAtom.h"
#include "RbtBaseSF.h"
#include "RbtParameterFileSource.h"
#include "RbtTriposAtomType.h"

class RbtVdwSF : public virtual RbtBaseSF, public virtual RbtAnnotationHandler {
public:
  // Class type string
  static std::string _CT;
  // Parameter names
  static std::string _USE_4_8; // TRUE = 4-8; FALSE = 6-12
  static std::string
      _USE_TRIPOS; // TRUE = Tripos 5.2 well depths; FALSE = GOLD well depths
  static std::string _RMAX; // Maximum range as a multiple of rmin
  static std::string _ECUT; // Energy cutoff for transition to short-range
                            // quadratic, as a multiple of well depth
  static std::string _E0;   // Energy at zero distance, as a multiple of ECUT

  RBTDLL_EXPORT static std::string &GetEcut();

  virtual ~RbtVdwSF();

protected:
  RbtVdwSF();

  // As this has a virtual base class we need a separate OwnParameterUpdated
  // which can be called by concrete subclass ParameterUpdated methods
  // See Stroustrup C++ 3rd edition, p395, on programming virtual base classes
  void OwnParameterUpdated(const std::string &strName);

  // Used by subclasses to calculate vdW potential between pAtom and all atoms
  // in atomList
  double VdwScore(const RbtAtom *pAtom, const RbtAtomRList &atomList) const;
  // As above, but with additional checks for enabled state of each atom
  double VdwScoreEnabledOnly(const RbtAtom *pAtom,
                             const RbtAtomRList &atomList) const;
  // XB Same as above, used to calcutate intra terms without the reweighting
  // factors RbtDouble VdwScoreIntra(const RbtAtom* pAtom, const RbtAtomRList&
  // atomList) const; Looks up the maximum range (rmax_sq) for any interaction
  // i.e. from across a row of m_vdwTable
  double MaxVdwRange(const RbtAtom *pAtom) const;
  double MaxVdwRange(RbtTriposAtomType::eType t) const;

  // Index the intramolecular interactions
  void BuildIntraMap(const RbtAtomRList &atomList,
                     RbtAtomRListList &intns) const;
  void BuildIntraMap(const RbtAtomRList &atomList1,
                     const RbtAtomRList &atomList2,
                     RbtAtomRListList &intns) const;
  void Partition(const RbtAtomRList &atomList, const RbtAtomRListList &intns,
                 RbtAtomRListList &prtIntns, double dist = 0.0) const;

private:
  // vdW scoring function params
  struct vdwprms {
    double A, B;       // 6-12 or 4-8 params
    double rmin;       // Sum of vdw radii
    double kij;        // Well depth
    double rmax_sq;    // Max distance**2 over which the vdw score should be
                       // calculated
    double rcutoff_sq; // Distance**2 at which the short-range quadratic
                       // potential kicks in
    double ecutoff;    // Energy at the cutoff point
    double slope;      // Slope of the short-range quadratic potential
    double e0;         // Energy at zero distance
  };

  typedef std::vector<RbtVdwSF::vdwprms> RbtVdwRow;
  typedef RbtVdwRow::iterator RbtVdwRowIter;
  typedef RbtVdwRow::const_iterator RbtVdwRowConstIter;
  typedef std::vector<RbtVdwRow> RbtVdwTable;
  typedef RbtVdwTable::iterator RbtVdwTableIter;
  typedef RbtVdwTable::const_iterator RbtVdwTableConstIter;

  // Generic scoring function primitive for 6-12
  inline double f6_12(double R_sq, const vdwprms &prms) const {
    // Zero well depth or long range: return zero
    if ((prms.kij == 0.0) || (R_sq > prms.rmax_sq)) {
      return 0.0;
    }
    // Short range: use quadratic potential
    else if (R_sq < prms.rcutoff_sq) {
      return prms.e0 - (prms.slope * R_sq);
    }
    // Everywhere else is 6-12
    else {
      double rr6 = 1.0 / (R_sq * R_sq * R_sq);
      return rr6 * (rr6 * prms.A - prms.B);
    }
  }

  // Generic scoring function primitive for 4-8
  inline double f4_8(double R_sq, const vdwprms &prms) const {
    // Zero well depth or long range: return zero
    if ((prms.kij == 0.0) || (R_sq > prms.rmax_sq)) {
      return 0.0;
    }
    // Short range: use quadratic potential
    else if (R_sq < prms.rcutoff_sq) {
      return prms.e0 - (prms.slope * R_sq);
    }
    // Everywhere else is 4-8
    else {
      double rr4 = 1.0 / (R_sq * R_sq);
      return rr4 * (rr4 * prms.A - prms.B);
    }
  }

  void Setup(); // Initialise m_vdwTable with appropriate params for each atom
                // type pair
  void SetupCloseRange(); // Regenerate the short-range params only (called more
                          // frequently)

  // Private predicate
  // Is the distance between atoms less than a given value ?
  // Function checks d**2 to save performing a sqrt
  class isD_lt : public std::unary_function<RbtAtom *, bool> {
    double d_sq;
    RbtAtom *a;

  public:
    explicit isD_lt(RbtAtom *aa, double dd) : d_sq(dd * dd), a(aa) {}
    bool operator()(RbtAtom *aa) const {
      return Rbt::Length2(aa->GetCoords(), a->GetCoords()) < d_sq;
    }
  };

  // Private data members
  bool m_use_4_8;
  bool m_use_tripos;
  double m_rmax;
  double m_ecut;
  double m_e0;
  RbtParameterFileSourcePtr m_spVdwSource; // File source for vdw params
  RbtVdwTable m_vdwTable; // Lookup table for all vdW params (indexed by Tripos
                          // atom type)
  std::vector<double>
      m_maxRange; // Vector of max ranges for each Tripos atom type
};

#endif //_RBTVDWSF_H_
