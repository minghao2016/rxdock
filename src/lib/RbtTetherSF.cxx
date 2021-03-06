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

#include "RbtTetherSF.h"
#include "RbtMdlFileSource.h"
#include "RbtWorkSpace.h"
#include <sstream>

// Static data members
std::string RbtTetherSF::_CT("RbtTetherSF");
std::string RbtTetherSF::_REFERENCE_FILE("REFERENCE_FILE");

// NB - Virtual base class constructor (RbtBaseSF) gets called first,
// implicit constructor for RbtBaseInterSF is called second
RbtTetherSF::RbtTetherSF(const std::string &strName) : RbtBaseSF(_CT, strName) {
  // Add parameters It gets the right name in SetupReceptor
  AddParameter(_REFERENCE_FILE, "_reference.sd");
#ifdef _DEBUG
  std::cout << _CT << " parameterised constructor" << std::endl;
#endif //_DEBUG
  _RBTOBJECTCOUNTER_CONSTR_(_CT);
}

RbtTetherSF::~RbtTetherSF() {
#ifdef _DEBUG
  std::cout << _CT << " destructor" << std::endl;
#endif //_DEBUG
  _RBTOBJECTCOUNTER_DESTR_(_CT);
}

std::vector<int>
RbtTetherSF::ReadTetherAtoms(std::vector<std::string> &strAtoms) {
  std::vector<int> tetherAtoms;
  std::string strTetherAtoms = Rbt::ConvertListToDelimitedString(strAtoms);
  std::istringstream ist(strTetherAtoms.c_str());
  int i;
  char sep[2];
  while (ist >> i) {
    tetherAtoms.push_back(i - 1);
    ist.width(2);
    ist >> sep;
  }
  return tetherAtoms;
}

void RbtTetherSF::SetupReceptor() {
  if (GetReceptor().Null())
    return;
  std::string strWSName = GetWorkSpace()->GetName();
  std::string refExt = GetParameter(_REFERENCE_FILE);
  std::string refFile = Rbt::GetRbtFileName("", strWSName + refExt);
  RbtMolecularFileSourcePtr spReferenceSD(
      new RbtMdlFileSource(refFile, false, false, true));
  RbtModelPtr spReferenceMdl(new RbtModel(spReferenceSD));
  std::vector<std::string> strTetherAtomsL =
      spReferenceMdl->GetDataValue("TETHERED ATOMS");
  std::vector<int> tetherAtomsId = ReadTetherAtoms(strTetherAtomsL);
  RbtAtomList refAtoms = spReferenceMdl->GetAtomList();
  for (std::vector<int>::iterator iter = tetherAtomsId.begin();
       iter < tetherAtomsId.end(); iter++) {
    m_tetherCoords.push_back(refAtoms[*iter]->GetCoords());
  }
}

void RbtTetherSF::SetupLigand() {
  m_ligAtomList.clear();
  if (GetLigand().Null())
    return;

  std::vector<std::string> strTetherAtomsL =
      GetLigand()->GetDataValue("TETHERED ATOMS");
  m_tetherAtomList = ReadTetherAtoms(strTetherAtomsL);
  if (m_tetherAtomList.size() != m_tetherCoords.size())
    throw RbtBadArgument(_WHERE_,
                         "the number of tethered atoms in the ligand SD file "
                         "should be the same than in the reference SD file");

  m_ligAtomList = GetLigand()->GetAtomList();
#ifdef _DEBUG
  std::cout << _CT << "::SetupLigand(): #ATOMS = " << m_ligAtomList.size()
            << std::endl;
#endif //_DEBUG
}

void RbtTetherSF::SetupScore() {
  // No further setup required
}

double RbtTetherSF::RawScore() const {
  double score(0.0);
  int i = 0;
  for (std::vector<int>::const_iterator iter = m_tetherAtomList.begin();
       iter < m_tetherAtomList.end(); iter++, i++)
    score += Rbt::Length2(m_ligAtomList[*iter]->GetCoords(), m_tetherCoords[i]);
  return score;
}

// DM 25 Oct 2000 - track changes to parameter values in local data members
// ParameterUpdated is invoked by RbtParamHandler::SetParameter
void RbtTetherSF::ParameterUpdated(const std::string &strName) {
  RbtBaseSF::ParameterUpdated(strName);
}
