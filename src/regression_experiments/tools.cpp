#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_fa/function_approximator.h"
#include "rosban_fa/trainer_factory.h"

#include "rosban_gp/scoring.h"

#include "rosban_utils/time_stamp.h"

#include "rosban_random/tools.h"

#include <fstream>
#include <memory>

using rosban_utils::TimeStamp;
using rosban_fa::Trainer;
using rosban_fa::TrainerFactory;
using rosban_fa::FunctionApproximator;

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
                     const std::string & trainer_name,
                     const std::vector<int> & points_by_dim,
                     Eigen::MatrixXd & samples_inputs,
                     Eigen::VectorXd & samples_outputs,
                     Eigen::MatrixXd & prediction_points,
                     Eigen::VectorXd & prediction_means,
                     Eigen::VectorXd & prediction_vars,
                     Eigen::MatrixXd & gradients)
{
  // getting random engine
  auto engine = rosban_random::getRandomEngine();
  // Building function
  BenchmarkFunctionFactory bff;
  std::unique_ptr<BenchmarkFunction> benchmark_function(bff.build(function_name));
  // Generating random input
  benchmark_function->getUniformSamples(nb_samples, samples_inputs, samples_outputs, &engine);
  // Solving
  std::unique_ptr<Trainer> trainer(TrainerFactory().build(trainer_name));
  std::shared_ptr<const FunctionApproximator> fa;
  fa = trainer->train(samples_inputs, samples_outputs, benchmark_function->getLimits());
  // Discretizing space
  prediction_points = discretizeSpace(benchmark_function->getLimits(), points_by_dim);
  // Computing predictions and variances
  predict(fa, prediction_points, prediction_means, prediction_vars);
  // Computing gradients
  gradients = Eigen::MatrixXd::Zero(prediction_points.rows(), prediction_points.cols());
  for (int i = 0; i < prediction_points.cols(); i++)
  {
    // Tmp variables
    Eigen::VectorXd point_gradient;
    // Running computation
    fa->gradient(prediction_points.col(i), point_gradient);
    // Filling results
    gradients.col(i) = point_gradient;
  }
}

void predict(std::shared_ptr<const FunctionApproximator> fa,
             const Eigen::MatrixXd & points,
             Eigen::VectorXd & prediction_means,
             Eigen::VectorXd & prediction_vars)
{
  prediction_means = Eigen::VectorXd::Zero(points.cols());
  prediction_vars = Eigen::VectorXd::Zero(points.cols());
  for (int i = 0; i < points.cols(); i++)
  {
    // Tmp variables
    double mean, var;
    Eigen::VectorXd point_gradient;
    // Running computation
    fa->predict(points.col(i), mean, var);
    // Filling results
    prediction_means(i) = mean;
    prediction_vars(i) = var;
  }
}

void runBenchmark(const std::string & function_name,
                  int nb_samples,
                  const std::string & trainer_name,
                  int nb_test_points,
                  double & smse,
                  double & learning_time,
                  double & prediction_time,
                  double & arg_max_loss,
                  double & max_prediction_error,
                  double & compute_max_time,
                  std::default_random_engine * engine)
{
  std::shared_ptr<Trainer> trainer(TrainerFactory().build(trainer_name));
  BenchmarkFunctionFactory bff;
  std::shared_ptr<BenchmarkFunction> function(bff.build(function_name));
  runBenchmark(function,
               nb_samples,
               trainer,
               nb_test_points,
               smse,
               learning_time,
               prediction_time,
               arg_max_loss,
               max_prediction_error,
               compute_max_time,
               engine);
}

void runBenchmark(std::shared_ptr<const BenchmarkFunction> function,
                  int nb_samples,
                  std::shared_ptr<const Trainer> trainer,
                  int nb_test_points,
                  double & smse,
                  double & learning_time,
                  double & prediction_time,
                  double & arg_max_loss,
                  double & max_prediction_error,
                  double & compute_max_time,
                  std::default_random_engine * engine)
{
  // Internal data:
  Eigen::MatrixXd samples_inputs;
  Eigen::VectorXd samples_outputs;
  Eigen::MatrixXd test_points;
  Eigen::VectorXd test_observations, prediction_means, prediction_vars;
                    
  bool clean_engine = false;
  // getting random engine
  if (engine == NULL) {
    engine = rosban_random::newRandomEngine();
    clean_engine = true;
  }
  // Generating samples and test points
  function->getUniformSamples(nb_samples, samples_inputs, samples_outputs, engine);
  function->getUniformSamples(nb_test_points, test_points, test_observations, engine);
  // Solving
  TimeStamp learning_start = TimeStamp::now();
  std::shared_ptr<const FunctionApproximator> fa;
  fa = trainer->train(samples_inputs, samples_outputs, function->getLimits());
  TimeStamp learning_end = TimeStamp::now();
  // Getting predictions for test points
  TimeStamp prediction_start = TimeStamp::now();
  predict(fa, test_points, prediction_means, prediction_vars);
  TimeStamp prediction_end = TimeStamp::now();
  // Evaluating prediction
  // Clean engine if necessary
  if (clean_engine) {
    delete(engine);
  }

  // Computing max
  Eigen::VectorXd best_input;
  double expected_max, measured_max;
  TimeStamp get_max_start = TimeStamp::now();
  fa->getMaximum(function->getLimits(), best_input, expected_max);
  TimeStamp get_max_end = TimeStamp::now();
  int nb_max_tests = 1000;
  measured_max = 0;
  for (int i = 0; i < nb_max_tests; i++) {
    measured_max += function->sample(best_input);
  }
  measured_max /= nb_max_tests;

  try{
    arg_max_loss = function->getMax() - measured_max;
    max_prediction_error = std::fabs(expected_max - measured_max);
  }
  catch(const std::runtime_error & exc) {
    arg_max_loss = -1;
    max_prediction_error = -1;
    std::cerr << exc.what() << std::endl;
  }

  // Computing output values
  smse = rosban_gp::computeSMSE(test_observations, prediction_means);
  learning_time = diffSec(learning_start, learning_end);
  prediction_time = diffSec(prediction_start, prediction_end);
  compute_max_time = diffSec(get_max_start, get_max_end);

  // Temporary disabling debug (not implemented for all trainers)
  //double suspicion_min = std::pow(10,2);
  //if (smse > suspicion_min) {
  //  std::cout << "Large smse: tracking debugs" << std::endl;
  //  for (int sample = 0; sample < nb_test_points; sample++) {
  //    double observation = test_observations(sample);
  //    double prediction = prediction_means(sample);
  //    double prediction_var = prediction_vars(sample);
  //    double diff2 = std::pow(observation - prediction, 2);
  //    if (diff2 > suspicion_min) {
  //      std::cout << "\tsample " << sample << ":" << std::endl;
  //      std::cout << "\t\tprediction     : " << prediction      << std::endl
  //                << "\t\tprediction var : " << prediction_var  << std::endl
  //                << "\t\tobservation    : " << observation     << std::endl
  //                << "\t\tdiff2          : " << diff2           << std::endl;
  //      fa->debugPrediction(test_points.col(sample), std::cout);
  //    }
  //  }
  //}
}

void writePrediction(const std::string & path,
                     const Eigen::MatrixXd & samples_inputs,
                     const Eigen::VectorXd & samples_outputs,
                     const Eigen::MatrixXd & prediction_points,
                     const Eigen::VectorXd & prediction_means,
                     const Eigen::VectorXd & prediction_vars,
                     const Eigen::MatrixXd & gradients)
{
  // Writing predictions + points
  std::ofstream out;
  out.open(path);
  out << "type,input,mean,min,max,gradient" << std::endl;

  // Writing Ref points
  for (int i = 0; i < samples_inputs.cols(); i++)
  {
    // write with the same format but min and max carry no meaning
    out << "observation," << samples_inputs(0,i) << ","
        << samples_outputs(i) << ",0,0,0" << std::endl;
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
        << mean << "," << min << "," << max << "," << gradients(0,point) << std::endl;
  }
  out.close();
}

}
