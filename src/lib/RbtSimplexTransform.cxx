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

#ifdef __PGI
#define EIGEN_DONT_VECTORIZE
#endif
#include <Eigen/Dense>
#include <iomanip>

#include "RbtChrom.h"
#include "RbtSFRequest.h"
#include "RbtSimplexCostFunction.h"
#include "RbtSimplexTransform.h"
#include "RbtWorkSpace.h"
#define VERBOSE 8
#include "RbtNMCriteria.h"
#include "RbtNMSimplex.h"

// Static data member for class type
std::string RbtSimplexTransform::_CT("RbtSimplexTransform");
// Parameter names
std::string RbtSimplexTransform::_MAX_CALLS("MAX_CALLS");
std::string RbtSimplexTransform::_NCYCLES("NCYCLES");
std::string RbtSimplexTransform::_STOPPING_STEP_LENGTH("STOPPING_STEP_LENGTH");
std::string RbtSimplexTransform::_PARTITION_DIST("PARTITION_DIST");
std::string RbtSimplexTransform::_STEP_SIZE("STEP_SIZE");
std::string RbtSimplexTransform::_CONVERGENCE("CONVERGENCE");

std::string &RbtSimplexTransform::GetMaxCalls() { return _MAX_CALLS; }

std::string &RbtSimplexTransform::GetNCycles() { return _NCYCLES; }

std::string &RbtSimplexTransform::GetStepSize() { return _STEP_SIZE; }

////////////////////////////////////////
// Constructors/destructors
RbtSimplexTransform::RbtSimplexTransform(const std::string &strName)
    : RbtBaseBiMolTransform(_CT, strName) {
  AddParameter(_MAX_CALLS, 200);
  AddParameter(_NCYCLES, 5);
  AddParameter(_STOPPING_STEP_LENGTH, 10e-4);
  AddParameter(_PARTITION_DIST, 0.0);
  AddParameter(_STEP_SIZE, 0.1);
  AddParameter(_CONVERGENCE, 0.001);
#ifdef _DEBUG
  std::cout << _CT << " parameterised constructor" << std::endl;
#endif //_DEBUG
  _RBTOBJECTCOUNTER_CONSTR_(_CT);
}

RbtSimplexTransform::~RbtSimplexTransform() {
#ifdef _DEBUG
  std::cout << _CT << " destructor" << std::endl;
#endif //_DEBUG
  _RBTOBJECTCOUNTER_DESTR_(_CT);
}

////////////////////////////////////////
// Protected methods
///////////////////
void RbtSimplexTransform::SetupReceptor() {}

void RbtSimplexTransform::SetupLigand() {}

void RbtSimplexTransform::SetupSolvent() {}

void RbtSimplexTransform::SetupTransform() {
  // Construct the overall chromosome for the system
  m_chrom.SetNull();
  RbtWorkSpace *pWorkSpace = GetWorkSpace();
  if (pWorkSpace) {
    m_chrom = new RbtChrom(pWorkSpace->GetModels());
  }
}

////////////////////////////////////////
// Private methods
///////////////////
// Pure virtual in RbtBaseTransform
// Actually apply the transform
void RbtSimplexTransform::Execute() {
  // Get the current scoring function from the workspace
  RbtWorkSpace *pWorkSpace = GetWorkSpace();
  if (pWorkSpace == nullptr) // Return if this transform is not registered
    return;
  RbtBaseSF *pSF = pWorkSpace->GetSF();
  if (pSF == nullptr) // Return if workspace does not have a scoring function
    return;
  int iTrace = GetTrace();

  pWorkSpace->ClearPopulation();
  int maxcalls = GetParameter(_MAX_CALLS);
  int ncycles = GetParameter(_NCYCLES);
  double stopping = GetParameter(_STOPPING_STEP_LENGTH);
  double convergence = GetParameter(_CONVERGENCE);
  double stepSize = GetParameter(_STEP_SIZE);
  double partDist = GetParameter(_PARTITION_DIST);
  RbtRequestPtr spPartReq(new RbtSFPartitionRequest(partDist));
  RbtRequestPtr spClearPartReq(new RbtSFPartitionRequest(0.0));
  pSF->HandleRequest(spPartReq);

  m_chrom->SyncFromModel();
  // If we are minimising all degrees of freedom simultaneuously
  // we have to compile a vector of variable step sizes for the NMSearch
  std::vector<double> sv;
  m_chrom->GetStepVector(sv);
  int nsv = sv.size();
  double *steps = new double[nsv];
  for (int i = 0; i < nsv; ++i) {
    steps[i] = sv[i] * stepSize;
  }

  RbtSimplexCostFunction costFunction(pSF, m_chrom);

  // Builder to generate the optimizer with a composite stoping criterion
  auto optimizer = RbtNM::CreateSimplex(
      costFunction, RbtNM::CreateAndCriteria(
                        RbtNM::IterationCriterion(maxcalls),
                        RbtNM::RelativeValueCriterion<double>(stopping)));

  int calls = 0;
  double initScore = pSF->Score(); // Current score
  double min = initScore;
  std::vector<double> vc; // Vector representation of chromosome
  // Energy change between cycles - initialise so as not to terminate loop
  // immediately
  double delta = -convergence - 1.0;

  if (iTrace > 0) {
    std::cout.precision(3);
    std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
    std::cout.setf(std::ios_base::right, std::ios_base::adjustfield);
    std::cout << std::endl
              << _CT << std::endl
              << std::setw(5) << "CYCLE" << std::setw(5) << "MODE"
              << std::setw(5) << "DOF" << std::setw(10) << "CALLS"
              << std::setw(10) << "SCORE" << std::setw(10) << "DELTA"
              << std::endl;
    std::cout << std::endl
              << std::setw(5) << "Init" << std::setw(5) << "-" << std::setw(5)
              << "-" << std::setw(10) << calls << std::setw(10) << initScore
              << std::setw(10) << "-" << std::endl
              << std::endl;
    if (iTrace > 1) {
      std::cout << *m_chrom << std::endl;
    }
  }

  for (int i = 0; (i < ncycles) && (delta < -convergence); i++) {
    if (partDist > 0.0) {
      pSF->HandleRequest(spPartReq);
    }
    // Use a variable length simplex
    vc.clear();
    m_chrom->GetVector(vc);
    RbtSimplexCostFunction::ParameterType start_point =
        Eigen::Map<RbtSimplexCostFunction::ParameterType, Eigen::Unaligned>(
            vc.data(), vc.size());

    if (iTrace > 0) {
      std::cout << std::setw(5) << i << std::setw(5) << "ALL" << std::setw(5)
                << vc.size();
    }

    optimizer.SetStartPoint(start_point); // Starting parameters
    optimizer.SetDelta(delta);            // Simplex size
    optimizer.Optimize(costFunction);     // Optimization start

    double newmin = optimizer.GetBestValue();
    delta = newmin - min;
    calls += costFunction.nCalls;
    RbtSimplexCostFunction::ParameterType best = optimizer.GetBestParameters();
    std::vector<double> best_vec(best.data(), best.data() + best.size());
    m_chrom->SetVector(best_vec);

    if (iTrace > 0) {
      std::cout << std::setw(10) << calls << std::setw(10) << newmin
                << std::setw(10) << delta << std::endl;
      if (iTrace > 1) {
        std::cout << *m_chrom << std::endl;
      }
    }

    min = newmin;
  }
  m_chrom->SyncToModel();
  pSF->HandleRequest(spClearPartReq); // Clear any partitioning
  delete[] steps;
  if (iTrace > 0) {
    min = pSF->Score();
    delta = min - initScore;
    std::cout << std::endl
              << std::setw(5) << "Final" << std::setw(5) << "-" << std::setw(5)
              << "-" << std::setw(10) << calls << std::setw(10) << min
              << std::setw(10) << delta << std::endl;
  }
}
