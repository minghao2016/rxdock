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

#include "RbtGPFFCrossDock.h"
#include "RbtDebug.h"
#include "RbtGPGenome.h"
#include "RbtGPParser.h"
#include <fstream>
#include <sstream>

std::string RbtGPFFCrossDock::_CT("RbtGPFFCrossDock");

void RbtGPFFCrossDock::ReadTables(std::istream &in, RbtReturnTypeArray &it,
                                  RbtReturnTypeArray &sft) {
  RbtReturnType value;
  int nip, nsfi;
  int i = 0, j, recordn;
  in >> nip;
  in.get();
  int nctes = 15;
  RbtGPGenome::SetNIP(nip + nctes);
  in >> nsfi;
  RbtGPGenome::SetNSFI(nsfi);
  in >> recordn;
  inputTable.clear();
  SFTable.clear();
  char name[80];
  CreateRandomCtes(nctes);
  while (!in.eof()) {
    // read name, don't store it for now
    in.get();
    in.getline(name, 80, ',');
    in >> value;
    RbtReturnTypeList ivalues;
    ivalues.push_back(new RbtReturnType(value));
    for (j = 1; j < nip; j++) {
      in.get();
      in >> value;
      ivalues.push_back(new RbtReturnType(value));
    }
    for (j = 0; j < nctes; j++)
      ivalues.push_back(new RbtReturnType(ctes[j]));
    RbtReturnTypeList sfvalues;
    for (j = 0; j < nsfi; j++) {
      in.get();
      in >> value;
      sfvalues.push_back(new RbtReturnType(value));
    }
    inputTable.push_back(RbtReturnTypeList(ivalues));
    SFTable.push_back(RbtReturnTypeList(sfvalues));
    i++;
    in >> recordn;
  }
  std::cout << "Read: " << inputTable[0][0] << std::endl;
  it = inputTable;
  sft = SFTable;
}

double RbtGPFFCrossDock::CalculateFitness(RbtGPGenomePtr g,
                                          RbtReturnTypeArray &it,
                                          RbtReturnTypeArray &sft,
                                          bool function) {
  if (function) {
    std::cout << "Error, no function possible with Cross Docking\n";
    std::exit(1);
  }
  RbtGPParser p(g->GetNIP(), g->GetNIF(), g->GetNN(), g->GetNO());
  RbtReturnTypeList o;
  double good = 0.0;
  double bad = 0.0;
  double neutral = 0.0;
  double hitlimit = 0.0;
  int ntm, nm;
  RbtReturnTypeList inputs;
  RbtReturnTypeList SFValues;
  double hit;
  unsigned int i = 0;
  while (i < it.size()) {
    inputs = it[i];
    SFValues = sft[i];
    hit = *(SFValues[SFValues.size() - 1]);
    ntm = 0;
    nm = 0;
    while (hit > hitlimit) {
      /*            std::cout << "input  : ";
                  for (RbtInt j = 0 ; j < inputs.size() ; j++)
                      std::cout << *(inputs[j]) << " ";
                  std::cout << std::endl; */
      RbtGPChromosomePtr c = g->GetChrom();
      o = p.Parse(c, inputs);
      c->Clear();
      // std::cout << *(o[0]) << std::endl;
      if (*(o[0]) >= hitlimit)
        ntm++;
      nm++;
      i++;
      inputs = it[i];
      SFValues = sft[i];
      hit = *(SFValues[SFValues.size() - 1]);
    }
    RbtGPChromosomePtr c = g->GetChrom();
    o = p.Parse(c, inputs);
    c->Clear();
    if (*(o[0]) < hitlimit) {
      //       if (ntm >= (nm / 2.0))
      if (ntm >= ((nm * 2.0) / 3.0))
        good++;
      else
        neutral++;
    } else {
      //        if (ntm >= (nm / 2.0))
      //        if (ntm >= ((nm * 2.0) / 3.0))
      bad++;
    }
    i++;
  }
  objective = good - 0.4 * neutral - bad;
  fitness = objective;
  g->SetFitness(fitness);
  // For now, I am using tournament selection. So the fitness
  // function doesn't need to be scaled in any way
  return fitness;
}

double RbtGPFFCrossDock::CalculateFitness(RbtGPGenomePtr g,
                                          RbtReturnTypeArray &it,
                                          RbtReturnTypeArray &sft,
                                          double hitlimit, bool function) {
  RbtGPParser p(g->GetNIP(), g->GetNIF(), g->GetNN(), g->GetNO());
  RbtReturnTypeList o;
  double good = 0.0;
  double bad = 0.0;
  double neutral = 0.0;
  int ntm, nm;
  RbtReturnTypeList inputs;
  RbtReturnTypeList SFValues;
  double hit;
  unsigned int i = 0;
  while (i < it.size()) {
    inputs = it[i];
    SFValues = sft[i];
    hit = *(SFValues[SFValues.size() - 1]);
    ntm = 0;
    nm = 0;
    while (hit > hitlimit) {
      /*            std::cout << "input  : ";
                  for (RbtInt j = 0 ; j < inputs.size() ; j++)
                      std::cout << *(inputs[j]) << " ";
                  std::cout << std::endl; */
      RbtGPChromosomePtr c = g->GetChrom();
      o = p.Parse(c, inputs);
      c->Clear();
      // std::cout << *(o[0]) << std::endl;
      double limit = 0.0;
      if (*(o[0]) >= limit)
        ntm++;
      nm++;
      i++;
      inputs = it[i];
      SFValues = sft[i];
      hit = *(SFValues[SFValues.size() - 1]);
    }
    RbtGPChromosomePtr c = g->GetChrom();
    o = p.Parse(c, inputs);
    c->Clear();
    if (*(o[0]) < hitlimit) {
      if (ntm >= ((nm * 2.0) / 3.0))
        good++;
      else
        neutral++;
    } else
      bad++;
    i++;
  }
  // objective value always an increasing function
  std::cout << good << "\t" << neutral << "\t" << bad << std::endl;
  objective = good / (good + bad);
  fitness = objective;
  // g->SetFitness(fitness);
  // For now, I am using tournament selection. So the fitness
  // function doesn't need to be scaled in any way
  return fitness;
}

void RbtGPFFCrossDock::CreateRandomCtes(int nctes) {
  if (ctes.size() == 0) // it hasn't already been initialized
  {
    int a, b;
    double c;
    ctes.push_back(0.0);
    ctes.push_back(1.0);
    std::cout << "c0 \t0.0" << std::endl;
    std::cout << "c1 \t1.0" << std::endl;
    for (int i = 0; i < (nctes - 2); i++) {
      a = m_rand.GetRandomInt(200) - 100;
      b = m_rand.GetRandomInt(10) - 5;
      c = (a / 10.0) * std::pow(10, b);
      std::cout << "c" << i + 2 << " \t" << c << std::endl;
      ctes.push_back(c);
    }
  }
}
