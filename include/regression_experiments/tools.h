#pragma once

#include "regression_experiments/benchmark_function.h"

#include "rosban_fa/function_approximator.h"
#include "rosban_fa/trainer.h"

#include <Eigen/Core>

#include <memory>

namespace regression_experiments
{

/// Return a matrix containing product(samples_by_dim) columns and limits.rows() rows
/// Each column is a different sample
Eigen::MatrixXd discretizeSpace(const Eigen::MatrixXd & limits,
                                const std::vector<int> & samples_by_dim);

/// 1. Create random samples of the given function
/// 2. Solve them using the chosen trainer
/// 3. Predict the output on the given grid
void buildPrediction(const std::string & function_name,
                     int nb_samples,
                     const std::string & trainer_name,
                     const std::vector<int> & points_by_dim,
                     Eigen::MatrixXd & samples_inputs,
                     Eigen::VectorXd & samples_outputs,
                     Eigen::MatrixXd & prediction_points,
                     Eigen::VectorXd & prediction_means,
                     Eigen::VectorXd & prediction_vars,
                     Eigen::MatrixXd & gradients);

void predict(std::shared_ptr<const rosban_fa::FunctionApproximator> fa,
             const Eigen::MatrixXd & points,
             Eigen::VectorXd & prediction_means,
             Eigen::VectorXd & prediction_vars);

/// 1. Generate learning and test samples for the given function
/// 2. Create a regression model using the chosen trainer and the generated samples
/// 3. Evaluate the quality of the regression model using the test set
/// All time are in ms
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
                  std::default_random_engine * engine);

void runBenchmark(std::shared_ptr<const BenchmarkFunction> function,
                  int nb_samples,
                  std::shared_ptr<const rosban_fa::Trainer> trainer,
                  int nb_test_points,
                  double & smse,
                  double & learning_time,
                  double & prediction_time,
                  double & arg_max_loss,
                  double & max_prediction_error,
                  double & compute_max_time,
                  std::default_random_engine * engine);

void writePrediction(const std::string & path,
                     const Eigen::MatrixXd & samples_inputs,
                     const Eigen::VectorXd & samples_outputs,
                     const Eigen::MatrixXd & prediction_points,
                     const Eigen::VectorXd & prediction_means,
                     const Eigen::VectorXd & prediction_vars,
                     const Eigen::MatrixXd & gradients);

}
