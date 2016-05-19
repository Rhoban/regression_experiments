#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/solver_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_gp/tools.h"

#include "rosban_random/tools.h"

#include <fstream>

using namespace regression_experiments;

using regression_forests::Forest;
using regression_forests::Tree;
using regression_forests::Node;
using regression_forests::Sample;
using regression_forests::TrainingSet;
using regression_forests::GPApproximation;
using regression_forests::ApproximationType;
using regression_forests::ExtraTrees;

using rosban_gp::GaussianProcess;

int main(int argc, char ** argv)
{
  // getting random tool
  auto engine = rosban_random::getRandomEngine();

  // Setting problem properties
  int nb_samples = 50;
  int nb_prediction_points = 1000;

  // The function to fit
  BenchmarkFunctionFactory bff;
  std::unique_ptr<BenchmarkFunction> benchmark_function(bff.build("abs"));

  // Generating random input
  Eigen::MatrixXd samples;
  Eigen::VectorXd observations;
  benchmark_function->getUniformSamples(nb_samples, samples, observations, &engine);

  // Solve
  std::unique_ptr<Solver> solver(SolverFactory().build("gp_forest"));
  solver->solve(samples, observations, benchmark_function->limits);

  // Predict
  Eigen::MatrixXd points = discretizeSpace(benchmark_function->limits,
                                           {nb_prediction_points});
  Eigen::VectorXd means, vars;
  solver->predict(points, means, vars);

   // Writing predictions + points
  std::ofstream out;
  out.open("forest_gp_predictions.csv");
  out << "type,input,mean,min,max" << std::endl;

  // Writing Ref points
  for (int i = 0; i < samples.cols(); i++)
  {
    // write with the same format but min and max carry no meaning
    out << "observation," << samples(0,i) << "," << observations(i) << ",0,0" << std::endl;
  }
  
  // Writing predictions
  for (int point = 0; point < points.cols(); point++)
  {
    Eigen::VectorXd prediction_input = points.col(point);
    double mean = means(point);
    double var = vars(point);
    // Getting +- 2 stddev
    double interval = 2 * std::sqrt(var);
    double min = mean - interval;
    double max = mean + interval;
    // Writing line
    out << "prediction," << prediction_input(0) << ","
        << mean << "," << min << "," << max << std::endl;
  }

  out.close();
}
