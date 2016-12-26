#ifndef __LOGOSTIC_REGRESSION__
#define __LOGOSTIC_REGRESSION__
#include "BinaryClassifier.h"

class LogisticRegression : public BinaryClassifier
{
public:
    //deserialize from stream
    virtual void load(std::istream& is);

    //serialize to stream
    virtual void save(std::ostream& os) const ;

    //train classifier
    virtual void train(Eigen::SparseMatrix<double>& design_matrix, Eigen::SparseMatrix<int>& label);

    //predict
	virtual void predict(Eigen::SparseMatrix<double, Eigen::RowMajor>& design_matrix, Eigen::VectorXd& probablity) const;
	LogisticRegression(){};
	virtual ~LogisticRegression(){};
private:
    Eigen::VectorXd weight;
    double intercept;
};
#endif