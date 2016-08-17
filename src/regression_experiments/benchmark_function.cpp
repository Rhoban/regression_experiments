#include "regression_experiments/benchmark_function.h"

#include "rosban_gp/tools.h"

#include "rosban_random/tools.h"

#include <functional>

namespace regression_experiments
{

BenchmarkFunction::BenchmarkFunction(double observation_noise_)
  : observation_noise(observation_noise_)
{}

double BenchmarkFunction::getMax() const
{
  throw std::runtime_error("Unimplemented getMax for given function");
}

void BenchmarkFunction::getUniformSamples(int nb_samples,
                                          Eigen::MatrixXd & samples,
                                          Eigen::VectorXd & observations,
                                          std::default_random_engine * engine,
                                          bool apply_noise) const
{
  // Generating random engine if none has been provided
  bool cleanup = false;
  if (engine == NULL)
  {
    engine = rosban_random::newRandomEngine();
    cleanup = true;
  }
  // Generating inputs and outputs
  samples = rosban_random::getUniformSamplesMatrix(getLimits(), nb_samples, engine);
  //std::function<double(const Eigen::VectorXd &)
  auto f = [this](const Eigen::VectorXd & input) { return this->sample(input); };            
  if (apply_noise) {
    observations = rosban_gp::generateObservations(samples, f, observation_noise, engine);
  }
  else {
    observations = rosban_gp::generateObservations(samples, f, 0.0, engine);
  }
  // Cleaning if required
  if (cleanup) delete(engine);
}

void BenchmarkFunction::to_xml(std::ostream &out) const
{
  rosban_utils::xml_tools::write<double>("observation_noise", observation_noise, out);
}

void BenchmarkFunction::from_xml(TiXmlNode *node)
{
  rosban_utils::xml_tools::try_read<double>(node, "observation_noise", observation_noise);
}

}
