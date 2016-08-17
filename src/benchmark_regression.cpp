#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_fa/trainer_factory.h"

#include "rosban_random/tools.h"

#include <fstream>
#include <map>

using namespace regression_experiments;

using rosban_gp::GaussianProcess;
using rosban_fa::Trainer;
using rosban_fa::TrainerFactory;

class BenchmarkConfig : public rosban_utils::Serializable
{
public:
  /// Which trainers are used? name -> trainer
  std::map<std::string, std::shared_ptr<const Trainer>> methods;
  /// Which functions are used for benchmark? name -> function
  std::map<std::string, std::shared_ptr<const BenchmarkFunction>> functions;
  /// What is the minimal number of samples?
  int min_samples;
  /// How many points are used to evaluate smse
  int nb_prediction_points;
  /// How many trials are used for each combination (method, nb_samples, function) 
  int nb_trials_per_type;
  /// Should max be evaluated?
  bool eval_max;
  /// Maximal learning time [s]
  double max_learning_time;
  /// Maximal prediction time per point [s]
  double max_prediction_time;
  /// Maximal time for max_prediction [s]
  double max_compute_max_time;
  /// Number of threads allowed for each method
  int nb_threads;
  
  std::string class_name() const override
    {
      return "benchmark_config";
    }

  void to_xml(std::ostream &out) const override
    {
      (void) out;
      throw std::logic_error("BenchmarkConfig::to_xml: Not implemented");
    }

  void from_xml(TiXmlNode *node)
    {
      nb_threads           = rosban_utils::xml_tools::read<int>   (node, "nb_threads"          );
      min_samples          = rosban_utils::xml_tools::read<int>   (node, "min_samples"         );
      nb_prediction_points = rosban_utils::xml_tools::read<int>   (node, "nb_prediction_points");
      nb_trials_per_type   = rosban_utils::xml_tools::read<int>   (node, "nb_trials_per_type"  );
      eval_max             = rosban_utils::xml_tools::read<bool>  (node, "eval_max"            );
      max_learning_time    = rosban_utils::xml_tools::read<double>(node, "max_learning_time"   );
      max_prediction_time  = rosban_utils::xml_tools::read<double>(node, "max_prediction_time" );
      max_compute_max_time = rosban_utils::xml_tools::read<double>(node, "max_compute_max_time");
      // Read methods
      TrainerFactory tf;
      std::function<std::shared_ptr<const Trainer>(TiXmlNode*)> trainer_builder;
      trainer_builder = [this, &tf](TiXmlNode * node)
        {
          std::unique_ptr<Trainer> trainer = tf.build(node);
          trainer->setNbThreads(this->nb_threads);
          return std::move(trainer);
        };
      methods = rosban_utils::xml_tools::read_map(node, "methods", trainer_builder);
      // Read functions
      BenchmarkFunctionFactory bff;
      std::function<std::shared_ptr<const BenchmarkFunction>(TiXmlNode*)> bf_builder;
      bf_builder = [&bff](TiXmlNode * node)
        { return std::shared_ptr<BenchmarkFunction>(bff.build(node)); };
      functions = rosban_utils::xml_tools::read_map(node, "functions", bf_builder);
    }
};

int main()
{
  BenchmarkConfig conf;
  conf.load_file();

  std::vector<int> nb_samples_vec;
  for (int i = 1; i <= 15; i++) {
    nb_samples_vec.push_back(conf.min_samples * std::pow(2,i-1));
  }

  // Creating random engine
  auto engine = rosban_random::getRandomEngine();

  // Open and write header for regression benchmark
  std::ofstream out;
  out.open("benchmark_regression.csv");
  out << "function_name,"
      << "method,"
      << "nb_samples,"
      << "smse,"
      << "learning_time,"
      << "prediction_time";
  if (conf.eval_max) {
    out << ",squared_loss,squared_error,compute_max_time";
  }
  out << std::endl;

  for (auto & function_entry : conf.functions) {
    const std::string & function_name = function_entry.first;
    std::shared_ptr<const BenchmarkFunction> function = function_entry.second;
    for (auto & method_entry : conf.methods) {
      const std::string & method_name = method_entry.first;
      std::shared_ptr<const Trainer> trainer = method_entry.second;
      for (int nb_samples : nb_samples_vec) {
        std::cout << "Fitting '" << function_name << "' with '" << method_name
                  << "' (" << nb_samples << " samples)" << std::endl;
        double total_prediction_time = 0;
        double total_learning_time   = 0;
        double total_max_time        = 0;
        for (int trial = 1; trial <= conf.nb_trials_per_type; trial++) {
          std::cerr << "\ttrial: " << trial << "/" << conf.nb_trials_per_type << std::endl;
          double smse, learning_time, prediction_time;
          double arg_max_loss, max_prediction_error, compute_max_time;
          runBenchmark(function,
                       nb_samples,
                       trainer,
                       conf.nb_prediction_points,
                       smse,
                       learning_time,
                       prediction_time,
                       arg_max_loss,
                       max_prediction_error,
                       compute_max_time,
                       &engine);
          // prediction time per point
          prediction_time /= conf.nb_prediction_points;

          double loss2, error2;
          loss2 = arg_max_loss * arg_max_loss;
          error2 = max_prediction_error * max_prediction_error;
          out << function_name   << ","
              << method_name     << ","
              << nb_samples      << ","
              << smse            << ","
              << learning_time   << ","
              << prediction_time;
          if (conf.eval_max) {
            out << "," << loss2
                << "," << error2
                << "," << (learning_time + compute_max_time);
          }
          out << std::endl;
          // Cumulating time
          total_learning_time   += learning_time;
          total_prediction_time += prediction_time;
          total_max_time += compute_max_time;
        }
        double avg_learning_time    = total_learning_time   / conf.nb_trials_per_type;
        double avg_prediction_time  = total_prediction_time / conf.nb_trials_per_type;
        double avg_max_time         = total_max_time        / conf.nb_trials_per_type;
        // Do not compute with higher number of samples if one of time is
        // already above the threshold
        if (avg_learning_time   > conf.max_learning_time   ) break;
        if (avg_prediction_time > conf.max_prediction_time ) break;
        if (avg_max_time        > conf.max_compute_max_time) break;
      }
    }
  }
}
