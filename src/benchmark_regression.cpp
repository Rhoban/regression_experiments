#include "regression_experiments/benchmark_function_factory.h"
#include "regression_experiments/solver_factory.h"
#include "regression_experiments/tools.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_random/tools.h"

#include <fstream>
#include <map>

using namespace regression_experiments;

using rosban_gp::GaussianProcess;

class BenchmarkConfig : public rosban_utils::Serializable
{
public:
  std::map<std::string, std::shared_ptr<Solver>> solvers;
  std::map<std::string, std::shared_ptr<BenchmarkFunction>> functions;
  int min_samples;
  int nb_prediction_points;
  int nb_trials_per_type;
  double max_learning_time;
  /// Maximal prediction time per point
  double max_prediction_time;
  
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
      min_samples          = rosban_utils::xml_tools::read<int>   (node, "min_samples"         );
      nb_prediction_points = rosban_utils::xml_tools::read<int>   (node, "nb_prediction_points");
      nb_trials_per_type   = rosban_utils::xml_tools::read<int>   (node, "nb_trials_per_type"  );
      max_learning_time    = rosban_utils::xml_tools::read<double>(node, "max_learning_time"   );
      max_prediction_time  = rosban_utils::xml_tools::read<double>(node, "max_prediction_time" );
      // Read solvers
      SolverFactory sf;
      std::function<std::shared_ptr<Solver>(TiXmlNode*)> solver_builder;
      solver_builder = [&sf](TiXmlNode * node) { return std::shared_ptr<Solver>(sf.build(node)); };
      solvers = rosban_utils::xml_tools::read_map(node, "solvers", solver_builder);
      // Read functions
      BenchmarkFunctionFactory bff;
      std::function<std::shared_ptr<BenchmarkFunction>(TiXmlNode*)> bf_builder;
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
  for (int i = 1; i <= 10; i++) {
    nb_samples_vec.push_back(conf.min_samples * std::pow(2,i-1));
  }

  // Creating random engine
  auto engine = rosban_random::getRandomEngine();

  // Open and write header for regression benchmark
  std::ofstream out;
  out.open("benchmark_regression.csv");
  out << "function_name,"
      << "solver,"
      << "nb_samples,"
      << "smse,"
      << "learning_time,"
      << "prediction_time"
      << std::endl;
  
  // Open and write header for max benchmark
  std::ofstream max_out;
  max_out.open("benchmark_max.csv");
  max_out << "function_name,"
          << "solver,"
          << "nb_samples,"
          << "squared_loss,"
          << "squared_error,"
          << "learning_time,"
          << "compute_max_time"
          << std::endl;

  for (auto & function_entry : conf.functions) {
    const std::string & function_name = function_entry.first;
    std::shared_ptr<BenchmarkFunction> function = function_entry.second;
    for (auto & solver_entry : conf.solvers) {
      const std::string & solver_name = solver_entry.first;
      std::shared_ptr<Solver> solver = solver_entry.second;
      for (int nb_samples : nb_samples_vec) {
        std::cout << "Fitting '" << function_name << "' with '" << solver_name
                  << "' (" << nb_samples << " samples)" << std::endl;
        double total_prediction_time = 0;
        double total_learning_time   = 0;
        for (int trial = 1; trial <= conf.nb_trials_per_type; trial++) {
          std::cerr << "\ttrial: " << trial << "/" << conf.nb_trials_per_type << std::endl;
          double smse, learning_time, prediction_time;
          double arg_max_loss, max_prediction_error, compute_max_time;
          runBenchmark(function,
                       nb_samples,
                       solver,
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
              << solver_name     << ","
              << nb_samples      << ","
              << smse            << ","
              << learning_time   << ","
              << prediction_time << std::endl;
          max_out << function_name    << ","
                  << solver_name      << ","
                  << nb_samples       << ","
                  << loss2            << ","
                  << error2           << ","
                  << learning_time    << ","
                  << compute_max_time << std::endl;
          // Cumulating time
          total_learning_time   += learning_time;
          total_prediction_time += prediction_time;
        }
        double avg_learning_time    = total_learning_time   / conf.nb_trials_per_type;
        double avg_prediction_time  = total_prediction_time / conf.nb_trials_per_type;
        // Do not compute with higher number of samples if time is already above the threshold
        if (avg_learning_time   > conf.max_learning_time  ) break;
        if (avg_prediction_time > conf.max_prediction_time) break;
      }
    }
  }
}
