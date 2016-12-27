#include "LogisticRegression.h"
#include <iomanip>
void LogisticRegression::load(std::istream& is){
	Eigen::Index coef_number = 0;
	assert(is.good());
	is >> intercept;
	assert(!is.fail());
	is >> coef_number;
	weight.resize(coef_number);
	for (Eigen::Index i = 0; i < coef_number; i++){
		assert(!is.fail());
		is >> weight(i);
	}
	assert(!is.fail());

	//weight.resize(3);
	//weight.fill(1);
	//intercept = 10;
}

//serialize to stream
void LogisticRegression::save(std::ostream& os) const{	
	os.flags(std::ios::scientific);
	os << std::setprecision(16);
	assert(os.good());
	os << intercept << '\n';
	assert(os.good());
	os << weight.size() << '\n';	
	auto coef_number = weight.size();
	for (Eigen::Index i = 0; i < coef_number; i++){
		assert(os.good());
		os << weight(i) << '\n';
	}
	assert(os.good());
}


//train classifier
void LogisticRegression::train(Eigen::SparseMatrix<double>& design_matrix, Eigen::SparseMatrix<int>& label){
	assert("Waiting for implementation." && 0);
}

//predict
void LogisticRegression::predict(Eigen::SparseMatrix<double, Eigen::RowMajor>& design_matrix, Eigen::VectorXd& probablity)const {
	//std::cout << "weight.IsRowMajor=" << weight.IsRowMajor << std::endl;
	//std::cout << "design_matrix.IsRowMajor=" << design_matrix.IsRowMajor << std::endl;
	//assert(weight.IsRowMajor == design_matrix.IsRowMajor);
	probablity = design_matrix * weight +Eigen::VectorXd::Constant(design_matrix.rows(), intercept);
}

