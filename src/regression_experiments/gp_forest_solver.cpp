#include "regression_experiments/gp_forest_solver.h"

#include "rosban_regression_forests/algorithms/extra_trees.h"
#include "rosban_regression_forests/approximations/gp_approximation.h"

#include "rosban_gp/auto_tuning.h"
#include "rosban_gp/tools.h"
#include "rosban_gp/core/gaussian_process.h"

#include "rosban_utils/serializable.h"

#include "rosban_random/tools.h"

using regression_forests::Approximation;
using regression_forests::GPApproximation;
using regression_forests::ExtraTrees;
using regression_forests::TrainingSet;
using regression_forests::Forest;
using regression_forests::Tree;
using regression_forests::Node;

using rosban_gp::GaussianProcess;

namespace regression_experiments
{

GPForestSolver::GPForestSolver(Type t)
  : type(t), nb_threads(1)
{
}
GPForestSolver::~GPForestSolver() {}

void GPForestSolver::solve(const Eigen::MatrixXd & inputs,
                           const Eigen::VectorXd & observations,
                           const Eigen::MatrixXd & limits)
{
  int nb_samples = observations.rows();

  ExtraTrees solver;
  solver.conf =  ExtraTrees::Config::generateAuto(limits, nb_samples,
                                                  Approximation::ID::GP);

  // Updating nmin with respect to type
  switch(type)
  {
    case GPForestSolver::Type::SQRT:
      solver.conf.n_min = std::sqrt(nb_samples);
      break;
    case GPForestSolver::Type::CURT:
      solver.conf.n_min = std::pow(nb_samples, 1.0 / 3);
      break;
    case GPForestSolver::Type::LOG2:
      solver.conf.n_min = std::log2(nb_samples);
      break;
  }
  solver.conf.nb_threads = nb_threads;
  solver.conf.gp_conf = approximation_conf;

  TrainingSet ts(inputs, observations);

  // Solve problem
  forest = solver.solve(ts, limits);
}

void GPForestSolver::predict(const Eigen::MatrixXd & inputs,
                             Eigen::VectorXd & means,
                             Eigen::VectorXd & vars)
{
  Eigen::VectorXd result(inputs.cols());
  means = Eigen::VectorXd::Zero(inputs.cols());
  vars = Eigen::VectorXd::Zero(inputs.cols());
  for (int point = 0; point < inputs.cols(); point++) {
    Eigen::VectorXd prediction_input = inputs.col(point);
    // Retrieving gaussian processes at the given point
    std::vector<GaussianProcess> gps;
    for (size_t tree_id = 0; tree_id < forest->nbTrees(); tree_id++) {
      const Tree & tree = forest->getTree(tree_id);
      const Node * leaf = tree.root->getLeaf(prediction_input);
      std::shared_ptr<const GPApproximation> gp_approximation;
      gp_approximation = std::dynamic_pointer_cast<const GPApproximation>(leaf->a);
      if (!gp_approximation) {
        throw std::runtime_error("Found an approximation which is not a gaussian process");
      }
      gps.push_back(gp_approximation->gp);
    }
    // Averaging gaussian processes
    double mean, var;
    rosban_gp::getDistribParameters(prediction_input, gps, mean, var);
    means(point) = mean;
    vars(point) = var;
  }
}

void GPForestSolver::debugPrediction(const Eigen::VectorXd & input, std::ostream & out)
{
    // Retrieving gaussian processes at the given point
    std::vector<GaussianProcess> gps;
    for (size_t tree_id = 0; tree_id < forest->nbTrees(); tree_id++) {
      const Tree & tree = forest->getTree(tree_id);
      const Node * leaf = tree.root->getLeaf(input);
      std::shared_ptr<const GPApproximation> gp_approximation;
      gp_approximation = std::dynamic_pointer_cast<const GPApproximation>(leaf->a);
      if (!gp_approximation) {
        throw std::runtime_error("Found an approximation which is not a gaussian process");
      }
      gps.push_back(gp_approximation->gp);
    }
    // Averaging gaussian processes
    double mean, var;
    rosban_gp::getDistribParameters(input, gps, mean, var, &out);
}


void GPForestSolver::gradients(const Eigen::MatrixXd & inputs,
                               Eigen::MatrixXd & gradients)
{
  gradients = Eigen::MatrixXd::Zero(inputs.rows(), inputs.cols());
  for (int col = 0; col < inputs.cols(); col++) {
    Eigen::VectorXd input = inputs.col(col);
    Eigen::VectorXd grad = Eigen::VectorXd::Zero(input.rows());
    double total_weight = 0;
    for (size_t tree_id = 0; tree_id < forest->nbTrees(); tree_id++) {
      const Tree & tree = forest->getTree(tree_id);
      const Node * leaf = tree.root->getLeaf(input);
      std::shared_ptr<const GPApproximation> gp_approximation;
      gp_approximation = std::dynamic_pointer_cast<const GPApproximation>(leaf->a);
      if (!gp_approximation) {
        throw std::runtime_error("Found an approximation which is not a gaussian process");
      }
      double var = gp_approximation->gp.getVariance(input);
      if (var == 0) {
        var = std::pow(10,-20);//Avoiding to get a value of 0 for var
      }
      double weight = 1.0 / var;
      grad += gp_approximation->gp.getGradient(input) * weight;
      total_weight += weight;
    }
    gradients.col(col) = grad / total_weight;
  }
}

void GPForestSolver::getMaximum(const Eigen::MatrixXd & limits,
                                Eigen::VectorXd & input, double & output)
{
  // Preparing functions
  std::function<Eigen::VectorXd(const Eigen::VectorXd)> gradient_func;
  gradient_func = [this](const Eigen::VectorXd & guess)
    {
      Eigen::MatrixXd gradient;
      this->gradients(guess, gradient);
      return Eigen::VectorXd(gradient.col(0));
    };
  std::function<double(const Eigen::VectorXd)> scoring_func;
  scoring_func = [this](const Eigen::VectorXd & guess)
    {
      Eigen::VectorXd values, vars;
      this->predict(guess, values, vars);
      return values(0);
    };
  // Performing multiple rProp and conserving the best candidate
  Eigen::VectorXd best_guess;
  best_guess = rosban_gp::RandomizedRProp::run(gradient_func, scoring_func, limits,
                                               find_max_conf);
  input = best_guess;
  output = scoring_func(best_guess);
}

std::string GPForestSolver::class_name() const
{
  return "gp_forest_solver";
}

void GPForestSolver::to_xml(std::ostream &out) const
{
  rosban_utils::xml_tools::write<std::string>("type", to_string(type), out);
  rosban_utils::xml_tools::write<int>("nb_threads", nb_threads, out);
  approximation_conf.write("approximation_conf", out);
  find_max_conf.write("find_max_conf", out);
}

void GPForestSolver::from_xml(TiXmlNode *node)
{
  std::string type_str;
  rosban_utils::xml_tools::try_read<std::string>(node, "type", type_str);
  if (type_str.size() != 0) type = loadType(type_str);
  rosban_utils::xml_tools::try_read<int>(node, "nb_threads", nb_threads);
  approximation_conf.tryRead(node, "approximation_conf");
  find_max_conf.tryRead(node, "find_max_conf");
}

GPForestSolver::Type loadType(const std::string &s)
{
  if (s == "SQRT") return GPForestSolver::Type::SQRT;
  if (s == "CURT") return GPForestSolver::Type::CURT;
  if (s == "LOG2") return GPForestSolver::Type::LOG2;
  throw std::runtime_error("Unknown GPForestSolver::Type: '" + s + "'");
}

std::string to_string(GPForestSolver::Type type)
{
  switch(type)
  {
    case GPForestSolver::Type::SQRT: return "SQRT";
    case GPForestSolver::Type::CURT: return "CURT";
    case GPForestSolver::Type::LOG2: return "LOG2";
  }
  throw std::runtime_error("GPForestSolver::Type unknown in to_string");
}

}
