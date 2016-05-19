#include "regression_experiments/benchmark_function_factory.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_gp/tools.h"

#include "rosban_random/tools.h"

#include <fstream>

using regression_experiments::BenchmarkFunction;
using regression_experiments::BenchmarkFunctionFactory;

using regression_forests::Forest;
using regression_forests::Tree;
using regression_forests::Node;
using regression_forests::Sample;
using regression_forests::TrainingSet;
using regression_forests::GPApproximation;
using regression_forests::ApproximationType;
using regression_forests::ExtraTrees;

using rosban_gp::GaussianProcess;

// TODO: to move elswhere
void getDistribParameters(const Eigen::VectorXd & input,
                          const std::vector<GaussianProcess> & gps,
                          double & mean,
                          double & var)
{
  int nb_predictors = gps.size();
  Eigen::VectorXd means(nb_predictors);
  Eigen::VectorXd precisions(nb_predictors);
  // Compute values for each predictor
  for (size_t i = 0; i < gps.size(); i++)
  {
    const GaussianProcess & gp = gps[i];
    // compute values
    double tmp_mean, tmp_var;
    gp.getDistribParameters(input, tmp_mean, tmp_var);
    // Store values
    means(i) = tmp_mean;
    precisions(i) = 1.0 / tmp_var;
  }
  // Mix predictors
  Eigen::VectorXd weights = precisions;
  double total_weight = weights.sum();
  mean = weights.dot(means) / total_weight;
  // Since we artificially create nb_predictions, we cannot simply sum the precisions
  double final_precision = total_weight / nb_predictors;
  var = 1.0 / final_precision;
}

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

  ExtraTrees solver;
  solver.conf =  ExtraTrees::Config::generateAuto(benchmark_function->limits,
                                                  nb_samples,
                                                  ApproximationType::GP);

  // Building TrainingSet:

  TrainingSet ts(benchmark_function->limits.rows());

  for (int sample_id = 0; sample_id < samples.cols(); sample_id++)
  {
    Sample s(samples.col(sample_id), observations(sample_id));
    ts.push(s);
  }

  std::unique_ptr<Forest> forest = solver.solve(ts, benchmark_function->limits);

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
  for (int point = 0; point < nb_prediction_points; point++)
  {
    // Computing input
    double delta = benchmark_function->limits(0,1) - benchmark_function->limits(0,0);
    double x = benchmark_function->limits(0,0) + delta * point / nb_prediction_points;
    Eigen::VectorXd prediction_input(1);
    prediction_input(0) = x; 
    // Retrieving gaussian processes (TODO implement in another manner)
    std::vector<GaussianProcess> gps;
    for (size_t tree_id = 0; tree_id < forest->nbTrees(); tree_id++) {
      const Tree & tree = forest->getTree(tree_id);
      const Node * leaf = tree.root->getLeaf(prediction_input);
      const GPApproximation * gp_approximation = dynamic_cast<const GPApproximation *>(leaf->a);
      if (gp_approximation == nullptr) {
        throw std::runtime_error("Found an approximation which is not a gaussian process");
      }
      gps.push_back(gp_approximation->gp);
    }
    // Averaging gaussian processes
    double mean, var;
    getDistribParameters(prediction_input, gps, mean, var);
    // Getting +- 2 stddev
    double interval = 2 * std::sqrt(var);
    double min = mean - interval;
    double max = mean + interval;
    // Writing line
    out << "prediction," << x << ","
        << mean << "," << min << "," << max << std::endl;
  }

  out.close();
}
