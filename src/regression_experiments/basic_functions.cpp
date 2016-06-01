#include "regression_experiments/basic_functions.h"

namespace regression_experiments
{

SinusSum::SinusSum(int nb_cycles_, int nb_dimensions_)
  : nb_cycles(nb_cycles_),
    nb_dimensions(nb_dimensions_)
{}

Eigen::MatrixXd SinusSum::getLimits()
{
  Eigen::MatrixXd limits(nb_dimensions, 2);
  limits.col(0) = Eigen::VectorXd::Constant(nb_dimensions, -M_PI * nb_cycles);
  limits.col(1) = Eigen::VectorXd::Constant(nb_dimensions,  M_PI * nb_cycles);
  return limits;
}

double SinusSum::sample(const Eigen::VectorXd & input)
{
  double total = 0;
  for (int dim = 0; dim < nb_dimensions; dim++) {
    total += std::sin(input(dim));
  }
  return total;
}

double SinusSum::getMax()
{
  return nb_dimensions;
}

std::string SinusSum::class_name() const
{
  return "sinus_sum";
}

void SinusSum::to_xml(std::ostream &out) const
{
  BenchmarkFunction::to_xml(out);
  rosban_utils::xml_tools::write<int>("nb_dimensions", nb_dimensions, out);
  rosban_utils::xml_tools::write<int>("nb_cycles"    , nb_cycles    , out);
}

void SinusSum::from_xml(TiXmlNode *node)
{
  BenchmarkFunction::from_xml(node);
  rosban_utils::xml_tools::try_read<int>(node, "nb_dimensions", nb_dimensions);
  rosban_utils::xml_tools::try_read<int>(node, "nb_cycles"    , nb_cycles    );
}

AbsDiff::AbsDiff(double input_max_, int nb_dimensions_)
  : input_max(input_max_),
    nb_dimensions(nb_dimensions_)
{}

Eigen::MatrixXd AbsDiff::getLimits()
{
  Eigen::MatrixXd limits(nb_dimensions, 2);
  limits.col(0) = Eigen::VectorXd::Constant(nb_dimensions, -input_max);
  limits.col(1) = Eigen::VectorXd::Constant(nb_dimensions,  input_max);
  return limits;
}

double AbsDiff::sample(const Eigen::VectorXd & input)
{
  double total = 0;
  for (int dim = 0; dim < nb_dimensions; dim++) {
    total -= std::fabs(input(dim));
  }
  return total;
}

double AbsDiff::getMax()
{
  return 0;
}

std::string AbsDiff::class_name() const
{
  return "abs_diff";
}

void AbsDiff::to_xml(std::ostream &out) const
{
  BenchmarkFunction::to_xml(out);
  rosban_utils::xml_tools::write<double>("input_max"    , input_max    , out);
  rosban_utils::xml_tools::write<int>   ("nb_dimensions", nb_dimensions, out);
}

void AbsDiff::from_xml(TiXmlNode *node)
{
  BenchmarkFunction::from_xml(node);
  rosban_utils::xml_tools::try_read<double>(node, "input_max"    , input_max    );
  rosban_utils::xml_tools::try_read<int>   (node, "nb_dimensions", nb_dimensions);
}

}
