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

#include "RbtPdbFileSource.h"

// Constructors
RbtPdbFileSource::RbtPdbFileSource(const char *fileName)
    : RbtBaseMolecularFileSource(
          fileName, "PDB_FILE_SOURCE") // Call base class constructor
{
  _RBTOBJECTCOUNTER_CONSTR_("RbtPdbFileSource");
}

RbtPdbFileSource::RbtPdbFileSource(const std::string fileName)
    : RbtBaseMolecularFileSource(
          fileName, "PDB_FILE_SOURCE") // Call base class constructor
{
  _RBTOBJECTCOUNTER_CONSTR_("RbtPdbFileSource");
}

// Default destructor
RbtPdbFileSource::~RbtPdbFileSource() {
  _RBTOBJECTCOUNTER_DESTR_("RbtPdbFileSource");
}

void RbtPdbFileSource::Parse() {
  // Expected string constants in PDB files
  const std::string strTitleKey("REMARK ");
  const std::string strAtomKey("ATOM ");
  const std::string strHetAtmKey("HETATM ");
  // const std::string strEndKey("END");

  // Only parse if we haven't already done so
  if (!m_bParsedOK) {
    ClearMolCache(); // Clear current cache
    Read();          // Read the file

    try {
      for (RbtFileRecListIter fileIter = m_lineRecs.begin();
           fileIter != m_lineRecs.end(); fileIter++) {
        // Ignore blank lines
        if ((*fileIter).length() == 0) {
          continue;
        }
        // Check for Title record
        else if ((*fileIter).find(strTitleKey) == 0) {
          std::string strTitle = *fileIter;
          strTitle.erase(0, strTitleKey.length());
          m_titleList.push_back(strTitle);
        }
        // Check for Atom or HetAtm records
        else if (((*fileIter).find(strAtomKey) == 0) ||
                 ((*fileIter).find(strHetAtmKey) == 0)) {
          std::string strDummy;
          int nAtomId(1);             // atom number
          std::string strAtomName;    // atom name
          std::string strSegmentName; // segment name (chain identifier)
          std::string strSubunitName; // subunit(residue) name
          std::string strSubunitId;   // subunit(residue) ID
          RbtCoord coord;             // X,Y,Z coords
          // DM 15 June 2006 - change default occupancy to 1
          // to avoid disabling solvent if pdb file does not contain
          // occupancy field
          double occupancy(1.0);
          double tempFactor(0.0);

          RbtFileRec::size_type length = (*fileIter).size();
          if (length > 10) {
            std::istringstream istr((*fileIter).substr(6, 5).c_str());
            istr >> nAtomId;
            // std::cout << "Atom ID      =" << nAtomId << std::endl;
          }
          if (length > 15) {
            std::istringstream istr((*fileIter).substr(12, 4).c_str());
            istr >> strAtomName;
            // std::cout << "Atom name    =" << strAtomName << std::endl;
          }
          if (length > 19) {
            std::istringstream istr((*fileIter).substr(17, 3).c_str());
            istr >> strSubunitName;
            // std::cout << "Subunit name =" << strSubunitName << std::endl;
          }
          if (length > 21) {
            std::istringstream istr((*fileIter).substr(21, 1).c_str());
            istr >> strSegmentName;
            // std::cout << "Segment name =" << strSegmentName << std::endl;
          }
          if (length > 25) {
            std::istringstream istr((*fileIter).substr(22, 4).c_str());
            istr >> strSubunitId;
            // std::cout << "Subunit ID   =" << strSubunitId << std::endl;
          }
          if (length > 53) {
            std::istringstream istr((*fileIter).substr(30, 24).c_str());
            istr >> coord.x >> coord.y >> coord.z;
            // std::cout << "coord        =" << coord << std::endl;
          }
          if (length > 59) {
            std::istringstream istr((*fileIter).substr(54, 6).c_str());
            istr >> occupancy;
            // std::cout << "occupancy    =" << occupancy << std::endl;
          }
          if (length > 65) {
            std::istringstream istr((*fileIter).substr(60, 6).c_str());
            istr >> tempFactor;
            // std::cout << "tempFactor   =" << tempFactor << std::endl;
          }

          // Construct a new atom (constructor only accepts the 2D params)
          RbtAtomPtr spAtom(new RbtAtom(nAtomId,
                                        0, // Atomic number undefined
                                        strAtomName, strSubunitId,
                                        strSubunitName, strSegmentName));

          // Now set the 3-D params we have
          spAtom->SetCoords(coord);

          // Store the occupancy and temperature factor values
          // in User1 and User2 for use by solvent flexibility code
          // User1 is also used by scoring functions so these
          // values must be used immediately after creating the model
          spAtom->SetUser1Value(occupancy);
          spAtom->SetUser2Value(tempFactor);

          m_atomList.push_back(spAtom);
          m_segmentMap[strSegmentName]++; // increment atom count in segment map
        }

        //////////////////////////////////////////////////////////
        // If we get this far everything is OK
        m_bParsedOK = true;
      }
    }

    catch (RbtError &error) {
      ClearMolCache(); // Clear the molecular cache so we don't return
                       // incomplete atom and bond lists
      throw;           // Rethrow the RbtError
    }
  }
}
