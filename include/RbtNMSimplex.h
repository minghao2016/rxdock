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

#ifndef _RBTNMSIMPLEX_H_
#define _RBTNMSIMPLEX_H_

#ifdef __PGI
#define EIGEN_DONT_VECTORIZE
#endif
#include <Eigen/Core>
#include <iostream>

#include "RbtNMState.h"

namespace RbtNM {
/**
 * The Nelder-Mead Simplex algorithm
 * Complies to a simple and standard interface
 *
 * \param DataType is the type of inner values to consider
 * \param ParameterType is the type of ParameterType (Eigen if possible)
 * \param Function is the type of Function to optimize
 * \param Criterion is the type of the stopping Criterion
 */
template <class DataType, class ParameterType, class Function, class Criterion>
class Simplex {
private:
  DataType m_delta;
  ParameterType m_deltas;
  bool use_deltas;

  Eigen::Array<DataType, Eigen::Dynamic, Eigen::Dynamic> m_polytopePoints;
  Eigen::Array<DataType, Eigen::Dynamic, Eigen::Dynamic> m_polytopeValues;

  State<DataType, ParameterType> m_state;
  Criterion m_criterion;

  void InitializePolytope(const ParameterType &start_point, DataType delta,
                          Function &fun) // const
  {
    m_polytopePoints.resize(start_point.size(), start_point.size() + 1);
    m_polytopeValues.resize(1, start_point.size() + 1);
    m_polytopePoints.col(0) = start_point;
    m_polytopeValues(0, 0) = fun(start_point);
    Display(fun, m_polytopePoints.col(0));
    for (int i = 1; i < start_point.size() + 1; ++i) {
      m_polytopePoints.col(i) = start_point;
      m_polytopePoints(i - 1, i) += delta;
      m_polytopeValues(0, i) = fun(m_polytopePoints.col(i));
      Display(fun, m_polytopePoints.col(i));
    }
  }

  void InitializePolytope(const ParameterType &start_point,
                          ParameterType deltas, Function &fun) // const
  {
    m_polytopePoints.resize(start_point.size(), start_point.size() + 1);
    m_polytopeValues.resize(1, start_point.size() + 1);
    m_polytopePoints.col(0) = start_point;
    m_polytopeValues(0, 0) = fun(start_point);
    Display(fun, m_polytopePoints.col(0));
    for (int i = 1; i < start_point.size() + 1; ++i) {
      m_polytopePoints.col(i) = start_point;
      m_polytopePoints(i - 1, i) += deltas(i - 1);
      m_polytopeValues(0, i) = fun(m_polytopePoints.col(i));
      Display(fun, m_polytopePoints.col(i));
    }
  }

  void Display(Function &fun, const ParameterType &parameters) // const
  {
#ifdef _DEBUG
    std::cout << "Point: " << parameters << std::endl;
    std::cout << "Value: " << fun(parameters) << std::endl;
#endif // _DEBUG
  }

  void FindBestWorstNearWorst(Function &fun, int &best, int &worst,
                              int &near_worst) // const
  {
    if (m_polytopeValues(0, 0) > m_polytopeValues(0, 1)) {
      worst = 0;
      near_worst = 1;
    } else {
      worst = 1;
      near_worst = 0;
    }
    best = near_worst;
    for (int i = 2; i < m_polytopeValues.cols(); ++i) {
      if (m_polytopeValues(0, i) < m_polytopeValues(0, best)) {
        best = i;
      }
      if (m_polytopeValues(0, i) > m_polytopeValues(0, worst)) {
        near_worst = worst;
        worst = i;
      } else if (m_polytopeValues(0, i) > m_polytopeValues(0, near_worst)) {
        near_worst = i;
      }
    }
#ifdef _DEBUG
    std::cout << "Worst value is in position " << worst << "\t"
              << m_polytopeValues(0, worst) << std::endl
              << m_polytopePoints.col(worst) << std::endl;
    std::cout << "Near-worst value is in position " << near_worst << "\t"
              << m_polytopeValues(0, near_worst) << std::endl
              << m_polytopePoints.col(near_worst) << std::endl;
    std::cout << "Best value is in position " << best << "\t"
              << m_polytopeValues(0, best) << std::endl
              << m_polytopePoints.col(best) << std::endl;
#endif // _DEBUG
  }

  ParameterType CreateNewParameters(const ParameterType &sum,
                                    const ParameterType &discarded_point,
                                    DataType t) {
    DataType fac1 = (1 - t) / sum.size();
    DataType fac2 = fac1 - t;
    return sum * fac1 - discarded_point * fac2;
  }

public:
  Simplex(const Criterion &criterion)
      : m_delta(0), use_deltas(false), m_criterion(criterion) {}

  void Optimize(Function &fun) // const
  {
    m_state.iteration = 0;
    m_state.currentValue = fun(m_state.currentParameters);
    m_state.formerValue = std::numeric_limits<DataType>::max();
    m_state.bestValue = std::numeric_limits<DataType>::max();

    if (use_deltas) {
      InitializePolytope(m_state.currentParameters, m_deltas, fun);
    } else {
      InitializePolytope(m_state.currentParameters, m_delta, fun);
    }

    while (m_criterion(m_state)) {
#ifdef _DEBUG
      std::cout << "Starting iteration " << m_state.iteration << std::endl;
#endif // _DEBUG
      int best, worst, near_worst;
      FindBestWorstNearWorst(fun, best, worst, near_worst);
      m_state.currentValue = m_polytopeValues(0, best);
      m_state.currentParameters = m_polytopePoints.col(best);
      m_state.formerValue = m_polytopeValues(0, worst);
      m_state.formerParameters = m_polytopePoints.col(worst);

      if (m_state.currentValue < m_state.bestValue) {
        m_state.bestValue = m_state.currentValue;
        m_state.bestParameters = m_state.currentParameters;
      }

      ParameterType new_parameters = CreateNewParameters(
          m_polytopePoints.rowwise().sum(), m_polytopePoints.col(worst), -1);
      DataType new_value = fun(new_parameters);
#ifdef _DEBUG
      std::cout << "Trying normal" << std::endl;
      Display(fun, new_parameters);
#endif // _DEBUG
      if (new_value < m_state.bestValue) {
        ParameterType expansion_parameters = CreateNewParameters(
            m_polytopePoints.rowwise().sum(), m_polytopePoints.col(worst), -2);
        DataType expansion_value = fun(expansion_parameters);
#ifdef _DEBUG
        std::cout << "Trying expansion" << std::endl;
        Display(fun, expansion_parameters);
#endif // _DEBUG
        if (expansion_value < m_state.bestValue) {
          m_state.currentValue = m_polytopeValues(0, worst) = expansion_value;
          m_state.currentParameters = m_polytopePoints.col(worst) =
              expansion_parameters;
        } else {
          m_state.currentValue = m_polytopeValues(0, worst) = new_value;
          m_state.currentParameters = m_polytopePoints.col(worst) =
              new_parameters;
        }
      } else if (new_value > m_polytopeValues(0, near_worst)) {
        // New point is not better than near worst
        ParameterType contraction_parameters = CreateNewParameters(
            m_polytopePoints.rowwise().sum(), m_polytopePoints.col(worst), -.5);
        DataType contraction_value = fun(contraction_parameters);
#ifdef _DEBUG
        std::cout << "Trying contraction" << std::endl;
        Display(fun, contraction_parameters);
#endif // _DEBUG
        if (contraction_value > new_value) {
#ifdef _DEBUG
          std::cout << "Contraction around lowest" << std::endl;
          Display(fun, contraction_parameters);
#endif // _DEBUG
          ParameterType best_parameters = m_polytopePoints.col(best);
#ifdef _DEBUG
          std::cout << "Difference from the best" << std::endl;
          std::cout << ((m_polytopePoints.colwise() - best_parameters.array()) /
                        2)
                    << std::endl;
#endif // _DEBUG
          m_polytopePoints =
              ((m_polytopePoints.colwise() - best_parameters.array()) / 2)
                  .colwise() +
              best_parameters.array();
          for (int i = 0; i < m_polytopePoints.cols(); ++i) {
            m_polytopeValues(0, i) = fun(m_polytopePoints.col(i));
          }
        } else {
          m_state.currentValue = m_polytopeValues(0, worst) = contraction_value;
          m_state.currentParameters = m_polytopePoints.col(worst) =
              contraction_parameters;
        }
      } else {
        // New point, not the best, but better than the near worst
        m_state.currentValue = m_polytopeValues(0, worst) = new_value;
        m_state.currentParameters = m_polytopePoints.col(worst) =
            new_parameters;
      }

      ++m_state.iteration;
    }
  }

  /**
   * Retrieves the best parameters
   */
  const ParameterType &GetBestParameters() const {
    return m_state.bestParameters;
  }

  /**
   * Retrieves the best final value
   */
  const DataType GetBestValue() const { return m_state.bestValue; }

  void SetStartPoint(const ParameterType &point) {
    m_state.currentParameters = point;
  }

  void SetDelta(DataType delta) {
    this->m_delta = delta;
    use_deltas = false;
  }

  void SetDelta(ParameterType deltas) {
    this->m_deltas = deltas;
    use_deltas = true;
  }
};

template <class Function, class Criterion>
static Simplex<typename Function::DataType, typename Function::ParameterType,
               Function, Criterion>
CreateSimplex(Function &fun, const Criterion &criterion) // const
{
  return Simplex<typename Function::DataType, typename Function::ParameterType,
                 Function, Criterion>(criterion);
}
} // namespace RbtNM

#endif // _DEBUG /* _RBTNMSIMPLEX_H_ */
