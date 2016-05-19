#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/solver_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_random/tools.h"

#include <fstream>
#include <memory>

namespace regression_experiments
{

Eigen::MatrixXd discretizeSpace(const Eigen::MatrixXd & limits,
                                const std::vector<int> & samples_by_dim)
{
  // Checking consistency
  if (limits.rows() != (int)samples_by_dim.size()) {
    throw std::runtime_error("discretizeSpace: inconsistency: limits.rows() != samples_by_dim");
  }
  // Preparing preliminary data
  int total_points = 1;
  std::vector<int> intervals(samples_by_dim.size());
  Eigen::VectorXd delta = limits.col(1) - limits.col(0);
  for (size_t dim = 0; dim < samples_by_dim.size(); dim++)
  {
    intervals[dim] = total_points;
    total_points *= samples_by_dim[dim];
  }
  // Preparing points
  Eigen::MatrixXd points(limits.rows(), total_points);
  for (int dim = 0; dim < limits.rows(); dim++) {
    for (int point = 0; point < total_points; point++) {
      // Determining index inside given dimension
      int dim_index = point / intervals[dim];
      dim_index = dim_index % samples_by_dim[dim];
      // Computing value
      double value = (limits(dim, 1) + limits(dim, 0)) / 2;// default value
      if (samples_by_dim[dim] != 1) {
        double step_size = delta(dim) / (samples_by_dim[dim] - 1);
        value = limits(dim, 0) + step_size * dim_index;
      }
      points(dim, point) = value;
    }
  }
  return points;
}

void buildPrediction(const std::string & function_name,
                     int nb_samples,
                     const std::string & solver_name,
                     const std::vector<int> & points_by_dim,
                     Eigen::MatrixXd & samples_inputs,
                     Eigen::VectorXd & samples_outputs,
                     Eigen::MatrixXd & prediction_points,
                     Eigen::VectorXd & prediction_means,
                     Eigen::VectorXd & prediction_vars)
{
  // getting random engine
  auto engine = rosban_random::getRandomEngine();
  // Building function
  BenchmarkFunctionFactory bff;
  std::unique_ptr<BenchmarkFunction> benchmark_function(bff.build(function_name));
  // Generating random input
  benchmark_function->getUniformSamples(nb_samples, samples_inputs, samples_outputs, &engine);
  // Solving
  std::unique_ptr<Solver> solver(SolverFactory().build(solver_name));
  solver->solve(samples_inputs, samples_outputs, benchmark_function->limits);
  // Predicting
  prediction_points = discretizeSpace(benchmark_function->limits, points_by_dim);
  solver->predict(prediction_points, prediction_means, prediction_vars);
}

void writePrediction(const std::string & path,
                     const Eigen::MatrixXd & samples_inputs,
                     const Eigen::VectorXd & samples_outputs,
                     const Eigen::MatrixXd & prediction_points,
                     const Eigen::VectorXd & prediction_means,
                     const Eigen::VectorXd & prediction_vars)
{
  // Writing predictions + points
  std::ofstream out;
  out.open(path);
  out << "type,input,mean,min,max" << std::endl;

  // Writing Ref points
  for (int i = 0; i < samples_inputs.cols(); i++)
  {
    // write with the same format but min and max carry no meaning
    out << "observation," << samples_inputs(0,i) << ","
        << samples_outputs(i) << ",0,0" << std::endl;
  }

  // Writing predictions
  for (int point = 0; point < prediction_points.cols(); point++)
  {
    Eigen::VectorXd prediction_input = prediction_points.col(point);
    double mean = prediction_means(point);
    double var  = prediction_vars(point);
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

}
