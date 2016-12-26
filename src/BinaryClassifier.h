#ifndef __CLASSIFIER__
#define __CLASSIFIER__
#include <iostream>
#include <Eigen/Sparse>
class BinaryClassifier
{
public:
	//deserialize from stream
	virtual void load(std::istream& is) = 0;

	//serialize to stream
	virtual void save(std::ostream& os) const = 0;

	//train classifier
	virtual void train(Eigen::SparseMatrix<double>& design_matrix, Eigen::SparseMatrix<int>& label) = 0;

	//predict
	virtual void predict(Eigen::SparseMatrix<double, Eigen::RowMajor>& design_matrix, Eigen::VectorXd& probablity) const = 0;

	virtual ~BinaryClassifier(){};
};
#endif