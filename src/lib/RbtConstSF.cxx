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

#include "RbtConstSF.h"

// Static data member for class type
std::string RbtConstSF::_CT("RbtConstSF");
std::string RbtConstSF::_SOLVENT_PENALTY("SOLVENT_PENALTY");

RbtConstSF::RbtConstSF(const std::string &strName)
    : RbtBaseSF(_CT, strName), m_solventPenalty(0.5) {
  AddParameter(_SOLVENT_PENALTY, m_solventPenalty);
#ifdef _DEBUG
  std::cout << _CT << " parameterised constructor" << std::endl;
#endif //_DEBUG
  _RBTOBJECTCOUNTER_CONSTR_(_CT);
}

RbtConstSF::~RbtConstSF() {
#ifdef _DEBUG
  std::cout << _CT << " destructor" << std::endl;
#endif //_DEBUG
  _RBTOBJECTCOUNTER_DESTR_(_CT);
}

void RbtConstSF::ScoreMap(RbtStringVariantMap &scoreMap) const {
  if (isEnabled()) {
    // Divide the total raw score into "system" and "inter" components.
    double rs = InterScore();

    // First deal with the inter score which is stored in its natural location
    // in the map
    std::string name = GetFullName();
    scoreMap[name] = rs;
    AddToParentMapEntry(scoreMap, rs);

    // Now deal with the system raw score which needs to be stored in
    // SCORE.SYSTEM.CONST
    double system_rs = SystemScore();
    std::string systemName = RbtBaseSF::_SYSTEM_SF + "." + GetName();
    scoreMap[systemName] = system_rs;
    // increment the SCORE.SYSTEM total
    double parentScore = scoreMap[RbtBaseSF::_SYSTEM_SF];
    parentScore += system_rs * GetWeight();
    scoreMap[RbtBaseSF::_SYSTEM_SF] = parentScore;
  }
}

double RbtConstSF::RawScore() const { return InterScore() + SystemScore(); }

double RbtConstSF::InterScore() const {
  // With current protocols, ligand should always be enabled, so RawScore is 1
  double retVal = GetLigand()->GetEnabled() ? 1.0 : 0.0;
  return retVal;
}

double RbtConstSF::SystemScore() const {
  double retVal = 0.0;
  RbtModelList solventList = GetSolvent();
  for (RbtModelListConstIter iter = solventList.begin();
       iter != solventList.end(); ++iter) {
    if ((*iter)->GetEnabled()) {
      retVal += m_solventPenalty;
    }
  }
  return retVal;
}

void RbtConstSF::ParameterUpdated(const std::string &strName) {
  if (strName == _SOLVENT_PENALTY) {
    m_solventPenalty = GetParameter(_SOLVENT_PENALTY);
  } else {
    RbtBaseSF::ParameterUpdated(strName);
  }
}
