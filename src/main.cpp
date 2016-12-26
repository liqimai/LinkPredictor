#include <sstream>
#include <fstream>
#include <matio.h>
#include <iomanip>
#include "KnowledgeGraph.h"
#include "MetaPath.h"
#include "LogisticRegression.h"
void save_mat(const char* output_file, const char* var, const Eigen::SparseMatrix<double, Eigen::ColMajor>& matrix){
	assert(matrix.IsRowMajor == 0);
	assert(matrix.isCompressed());

	auto nnz = matrix.nonZeros();
	auto cols = matrix.cols();
	auto rows = matrix.rows();
	
	// sparse matrix are store in csc format, so feature matrix are transpoed
	// every column represents a pair of nodes
	// every row represents a metapath
	mat_sparse_t  sparse = { 0, };
	size_t dims[2] = {rows, cols};
	sparse.nzmax = (int)nnz;
	sparse.nir = (int)nnz;
	sparse.njc = (int)cols + 1;
	sparse.ndata = (int)nnz;

	//double* data = (double*)malloc(sizeof(double)*sparse.ndata);
	//int *ir = (int*)malloc(sizeof(int)*sparse.nir);
	//int *jc = (int*)malloc(sizeof(int)*sparse.njc);
	sparse.ir = (int*)matrix.innerIndexPtr();
	sparse.jc = (int*)matrix.outerIndexPtr();
	sparse.data = (void*)matrix.valuePtr();

	//Values:	22	7	_	3	5	14	_	_	1	_	17	8
	//InnerIndices : 1	2	_	0	2	4	_	_	2	_	1	4
	//OuterStarts : 0	3	5	8	10	12

	////prepare jc, jr and data
	//sparse.jc[0] = 0;
	//for (size_t i = 0, count = 0; i < nonzero_indices.size(); i++){
	//	memcpy(ir + count, nonzero_indices[i].data(), sizeof(*ir)*nonzero_indices[i].size());
	//	memcpy(data + count, nonzero_values[i].data(), sizeof(*data)*nonzero_indices[i].size());
	//	count += nonzero_indices[i].size();
	//	jc[i+1] = (int)count;
	//}
	
	mat_t *mat = Mat_Create(output_file, NULL);
	assert(mat);

	matvar_t *matvar = Mat_VarCreate(var, MAT_C_SPARSE, MAT_T_DOUBLE, 2, dims, &sparse, 0);
	assert(matvar);
#ifdef _DEBUG
	Mat_VarPrint(matvar,1);
#endif
	Mat_VarWrite(mat, matvar, MAT_COMPRESSION_NONE);
	Mat_VarFree(matvar);
	Mat_Close(mat);

	//features_os.flags(std::ios::scientific);
	//features_os << std::setprecision(16);
	//features_os <<
	//	"%%MatrixMarket matrix coordinate real general \n"
	//	"% \n";
	//features_os << nonzero_indices.size() << '\t' << path_number << '\t' << nonzero_count << endl;
	//for (unsigned int i = 0; i < samples_number; i++){
	//	for (unsigned int j = 0; j < nonzero_indices[i].size(); j++){
	//		features_os << i + 1 << '\t' << nonzero_indices[i][j] + 1 << '\t' << nonzero_values[i][j] << endl;
	//	}
	//}
}
void load_mat(const char* input_file, const char *var){//, const Eigen::SparseMatrix<double, Eigen::ColMajor>& matrix){

	int err = 0;
	mat_t *mat;
	matvar_t *matvar;

	mat = Mat_Open(input_file, MAT_ACC_RDONLY);
	assert(mat);
	matvar = Mat_VarRead(mat, var);
	assert(matvar);
	assert(matvar->data_type == MAT_T_DOUBLE);
	assert(matvar->class_type == MAT_C_SPARSE);
	assert(matvar->rank == 2);
	assert(matvar->isComplex == 0);
	assert(matvar->isLogical == 0);
	assert(matvar->compression == MAT_COMPRESSION_NONE);

#ifdef _DEBUG
	Mat_VarPrint(matvar, 1);
#endif

	mat_sparse_t* sparse = (mat_sparse_t*)matvar->data;
}

int main(int argc, char const *argv[])
{

	if (argc != 12){
		std::cout << 
			"Usage: GenerateTrain <train-set> <relation-name> <node2id-file> <relation2id-file> <max-thread> <extract-metapath> <max-length> <metapath-file> <mat-file> <label-file> <metapath-list>" << std::endl;
		return -1;
	}
	std::ifstream graph_file(argv[1]);
	assert(graph_file.is_open());
	std::string relation_name(argv[2]);/*KG.get_relation_names()[0]*/
	std::ofstream node_file(argv[3]);
	assert(node_file.is_open());
	std::ofstream relation_file(argv[4]);
	assert(relation_file.is_open());
	int thread_number = atoi(argv[5]);
	assert(thread_number > 0);
	int extract_metapath = atoi(argv[6]);
	int max_length = atoi(argv[7]);
	assert(max_length > 0);
	const char* mat_file = argv[9];
	std::ofstream label_file(argv[10]);
	assert(label_file.is_open());
	std::ofstream metapath_list_file(argv[11]);
	assert(metapath_list_file.is_open());


	KnowledgeGraph KG(graph_file);
	KG.save_node_names(node_file);
	node_file.close();
	assert(!node_file.is_open());
	KG.save_relation_names(relation_file);
	relation_file.close();
	assert(!relation_file.is_open());

	MetaPath* meta_path;
	if (extract_metapath){
		meta_path = MetaPath::extract_from(KG, relation_name, max_length, thread_number);
		std::ofstream metapath_file(argv[8]);
		assert(metapath_file.is_open());
		metapath_file << *meta_path;
		metapath_file.close();
		assert(!metapath_file.is_open());
	}
	else{
		std::ifstream metapath_file(argv[8]);
		metapath_file >> *meta_path;
	}
	meta_path->print_in_list_format(metapath_list_file, KG);
	metapath_list_file.close();
	assert(!metapath_list_file.is_open());

	std::vector<std::pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> > node_pairs;
	std::vector<int> labels;
	std::vector<KnowledgeGraph::Edge*> forbiden_edges;
	KG.generate_train_set(relation_name, node_pairs, labels, forbiden_edges, 10);
	KG.vector_of_labels_to_os(labels, label_file);
	label_file.close();
	assert(!label_file.is_open());

	Eigen::SparseMatrix <double, Eigen::RowMajor> design_matrix;
	meta_path->calculate_feature(KG, node_pairs, forbiden_edges, design_matrix, thread_number);
	save_mat(mat_file, "X", design_matrix.transpose());


	return 0;





	//if (argc != 5){
	//	std::cout << "Wrong arg number" << std::endl;
	//	return -1;
	//}
	//std::ifstream graph_file(argv[1]);
	//std::string relation_name(argv[2]);/*KG.get_relation_names()[0]*/
	//int max_length = atoi(argv[3]);
	//int thread_number = atoi(argv[4]);

	////save load test
	//KnowledgeGraph KG(graph_file);
	////std::string node_file = std::string(argv[1]).append(".node");
	////std::string relation_file = std::string(argv[1]).append(".relation");
	////KG.save_node_names(std::ofstream(node_file));
	////KG.save_relation_names(std::ofstream(relation_file));

	////KnowledgeGraph KG2;
	////KG2.load_node_names(std::ifstream(node_file));
	////KG2.load_relation_names(std::ifstream(relation_file));
	////KG2.save_node_names(std::ofstream(node_file));
	////KG2.save_relation_names(std::ofstream(relation_file));

	////metapath test
	//MetaPath* meta_path;

	////meta_path = MetaPath::extract_from(KG, relation_name, max_length, thread_number);
	////std::ofstream of("meta_path.txt");
	////of << *meta_path;
	////of.close();

	////meta_path->print_in_tree_format(std::cout, KG);
	////std::cout << std::endl;
	////meta_path->print_in_list_format(std::cout, KG);

	//meta_path = new MetaPath;
	//std::ifstream("meta_path.txt") >> *meta_path;

	//const char* node_pair_file_name = "node_pair.txt";
	//const char* label_file_name = "y.txt";
	//const char* feature_file_name = "X.mat";
	//std::stringstream ss;
	////std::cout << "Generating Train Set" << std::endl;
	////KG.generate_train_set(relation_name, std::ofstream(node_pair_file_name), std::ofstream(label_file_name));
	//std::cout << "Calculating Features" << std::endl;
	//Eigen::SparseMatrix <double, Eigen::RowMajor> design_matrix;
	//meta_path->calculate_feature(KG, std::ifstream(node_pair_file_name), design_matrix, thread_number);
	//save_mat(feature_file_name, "X", design_matrix.transpose());
	////load_mat(feature_file_name, "X");

	////logistic regerssion
	//LogisticRegression clf_lr; 

	//////load and save
	////{
	////	std::ifstream weight_if("tmp.txt");
	////	std::ofstream weight_of("tmp2.txt");
	////	clf_lr.load(weight_if);
	////	clf_lr.save(weight_of);
	////}
	//
	////std::ofstream feature_os("design_matrix.txt");
	////feature_os << design_matrix.rows() << ' ' << design_matrix.cols() << ' ' << design_matrix.nonZeros() << std::endl;
	////for (int k = 0; k < design_matrix.outerSize(); ++k)
	////	for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(design_matrix, k); it; ++it){
	////		feature_os << it.row() << ' ' << it.col() << ' ' << it.value() << '\n';
	////		//it.row();   // row index
	////		//it.col();   // col index (here it is equal to k)
	////		//it.index(); // inner index, here it is equal to it.row()
	////	}

	//Eigen::VectorXd result;
	//clf_lr.predict(design_matrix, result);

	////std::ofstream predict("predict_c++.txt");
	////predict.flags(std::ios::scientific);
	////predict << std::setprecision(16);
	////for (size_t i = 0; i < result.rows(); i++){
	////	predict << result(i) << std::endl;
	////}
 //   return 0;
}