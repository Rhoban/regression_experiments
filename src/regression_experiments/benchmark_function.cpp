#include "regression_experiments/benchmark_function.h"

#include "rosban_gp/tools.h"

#include "rosban_random/tools.h"

namespace regression_experiments
{

BenchmarkFunction::BenchmarkFunction(const std::string & name_,
                                     EvaluationFunction f_,
                                     const Eigen::MatrixXd limits_)
  : name(name_), f(f_), limits(limits_)
{}

BenchmarkFunction::BenchmarkFunction(const std::string & name_,
                                     std::function<double(double)> f,
                                     const Eigen::MatrixXd limits_)
  : BenchmarkFunction(name_,
                      [f](const Eigen::VectorXd & input)
                      { return f(input(0)); },
                      limits_)
{}

void BenchmarkFunction::getUniformSamples(int nb_samples,
                                          Eigen::MatrixXd & samples,
                                          Eigen::VectorXd & observations,
                                          std::default_random_engine * engine)
{
  // Generating random engine if none has been provided
  bool cleanup = false;
  if (engine == NULL)
  {
    engine = rosban_random::newRandomEngine();
    cleanup = true;
  }
  // Generating inputs and outputs
  samples = rosban_random::getUniformSamplesMatrix(limits, nb_samples, engine);
  observations = rosban_gp::generateObservations(samples, f, 0.05, engine);
  // Cleaning if required
  if (cleanup) delete(engine);
}

}
