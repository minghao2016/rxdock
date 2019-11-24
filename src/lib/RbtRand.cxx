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

#include "RbtRand.h"
#include "Singleton.h"

/////////////
// Constructor
RbtRand::RbtRand() {
  // Seed the random number generator
  // Fixed seed in debug mode
  // Seed from the random device in release mode
#ifdef _DEBUG
  Seed();
#else  //_DEBUG
  SeedFromRandomDevice();
#endif //_DEBUG
  _RBTOBJECTCOUNTER_CONSTR_("RbtRand");
}

/////////////
// Destructor
RbtRand::~RbtRand() { _RBTOBJECTCOUNTER_DESTR_("RbtRand"); }

////////////////
// Public methods

// Seed the random number generator
void RbtRand::Seed(int seed) { m_rng.seed(seed); }

// Seed the random number generator from the random device
void RbtRand::SeedFromRandomDevice() {
  pcg_extras::seed_seq_from<std::random_device> seed_source;
  m_rng.seed(seed_source);
}

// Get a random double between 0 and 1
double RbtRand::GetRandom01() {
  std::uniform_real_distribution<> uniform_dist(0.0, 1.0);
  return uniform_dist(m_rng);
}

// Get a random integer between 0 and nMax-1
int RbtRand::GetRandomInt(int nMax) {
  if (nMax == 0) {
    return nMax;
  }
  std::uniform_int_distribution<> uniform_dist(0, nMax - 1);
  return uniform_dist(m_rng);
}

// Get a random unit vector distributed evenly over the surface of a sphere
RbtVector RbtRand::GetRandomUnitVector() {
  double z = 2.0 * GetRandom01() - 1.0;
  double t = 2.0 * M_PI * GetRandom01();
  double w = std::sqrt(1.0 - z * z);
  double x = w * std::cos(t);
  double y = w * std::sin(t);
  return RbtVector(x, y, z);
}

// Get a random number from the Normal distribution (mean, variance)
double RbtRand::GetGaussianRandom(double mean, double variance) {
  std::normal_distribution<> gaussian_dist{mean, variance};
  return gaussian_dist(m_rng);
}

// Get a random number from the Cauchy distribution (mean, variance)
double RbtRand::GetCauchyRandom(double mean, double variance) {
  std::cauchy_distribution<> cauchy_dist{mean, variance};
  return cauchy_dist(m_rng);
}

///////////////////////////////////////
// Non-member functions in Rbt namespace

// Returns reference to single instance of RbtRand class (singleton)
RbtRand &Rbt::GetRbtRand() { return Singleton<RbtRand>::instance(); }
