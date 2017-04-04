#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_fa/pwl_forest_trainer.h"

#include "rosban_random/tools.h"

#include <fstream>

using namespace regression_experiments;

int main(int argc, char ** argv)
{
  // Random initialization
  auto engine = rosban_random::getRandomEngine();

  // Building function to be predicted
  BenchmarkFunctionFactory bff;  
  std::unique_ptr<BenchmarkFunction> bf = bff.build("sinus_sum");

  // Setting problem properties
  int nb_samples = 25;
  int nb_prediction_points = 1000;

  // Getting samples
  Eigen::MatrixXd samples_inputs;
  Eigen::VectorXd samples_outputs;
  bf->getUniformSamples(nb_samples, samples_inputs, samples_outputs, &engine);

  // Getting prediction points
  Eigen::MatrixXd prediction_inputs = discretizeSpace(bf->getLimits(), {nb_prediction_points}); 

  // Writing samples
  std::ofstream samples_out;
  samples_out.open("samples.csv");
  samples_out << "input,output" << std::endl;
  for (int sample_id = 0; sample_id < samples_outputs.rows(); sample_id++) {
    samples_out << samples_inputs(sample_id, 0) << "," << samples_outputs(sample_id) << std::endl;
  }
  samples_out.close();

  // Opening prediction file
  std::ofstream prediction_out;
  prediction_out.open("predictions.csv");
  prediction_out  << "nbTrees,input,output" << std::endl;
  
  // Iterate on the number of trees
  for (int nb_trees : {1, 10,100}) {
    // Creating trainer
    rosban_fa::PWLForestTrainer trainer;
    trainer.setNbTrees(nb_trees);

    // Training function approximators
    std::shared_ptr<const rosban_fa::FunctionApproximator> fa;
    fa = trainer.train(samples_inputs, samples_outputs, bf->getLimits());

    // Predicting outputs
    Eigen::VectorXd predictions, gradients;
    predict(fa, prediction_inputs, predictions, gradients);

    // Writing predictions
    for (int prediction_id = 0; prediction_id < nb_prediction_points; prediction_id++) {
      prediction_out << nb_trees << "," << prediction_inputs(prediction_id,0)
                     << "," << predictions(prediction_id) << std::endl;
    }
  }

  prediction_out.close();
}
