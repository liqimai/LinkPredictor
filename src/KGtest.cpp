#include "verbosity.h"
#include "LogisticRegression.h"
#include "KnowledgeGraph.h"
#include "MetaPath.h"
#include "matio.h"
#include <iostream>
#include <fstream>
#include <assert.h>

double AP(Eigen::VectorXd& predictions, std::vector<int>& label){
    assert(predictions.size() == label.size());
    std::vector<double> positive_sample_result;
    std::vector<size_t> positive_sample_rank;

    for (size_t i = 0; i < label.size(); i++){
        if (label[i]){
            double prediction = predictions[i];
            int rank = 0;

            for (size_t j = 0; j < positive_sample_result.size(); j++)
                if (positive_sample_result[j] == prediction)
                    positive_sample_rank[j] += 1;

            for (int k = 0; k < predictions.size(); k++)
                if (predictions[k] > prediction)
                    rank++;

            positive_sample_result.push_back(prediction);
            positive_sample_rank.push_back(rank);
        }
    }

    double ap = 0;
    std::sort(positive_sample_rank.begin(), positive_sample_rank.end());
    //std::cout << "ranks:\n";
    for (int i = 0; i < positive_sample_rank.size(); i++){
        //std::cout << positive_sample_rank[i] << std::endl;
        ap += 1.0*(i+1) / (positive_sample_rank[i]+1);
    }
    ap /= positive_sample_rank.size();
    return ap;
}

//double RR(Eigen::VectorXd result, std::vector<int> label){
//	
//}
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
    size_t dims[2] = { static_cast<size_t>(rows), static_cast<size_t>(cols) };
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
    Mat_VarPrint(matvar, 1);
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

int main(int argc, char const *argv[])
{
    if (argc != 10){
        std::cout <<
            "Usage: test <node-file> <relation-file> <test-graph> <relation> <metapath-file> <classifier-file> <result-file> <thread-number> <verbosity>" << std::endl;
        return -1;
    }
    

    std::ifstream node_file(argv[1]);
    assert(node_file.is_open());
    std::ifstream relation_file(argv[2]);
    assert(relation_file.is_open());
    std::ifstream graph_file(argv[3]);
    assert(graph_file.is_open());
    std::string relation_name(argv[4]);/*KG.get_relation_names()[0]*/
    std::ifstream metapath_file(argv[5]);
    assert(metapath_file.is_open());
    std::ifstream classifier_file(argv[6]);
    assert(classifier_file.is_open());
    std::ofstream result_file(argv[7]);
    assert(result_file.is_open());
    int thread_number = atoi(argv[8]);
    assert(thread_number > 0);
    verbosity = atoi(argv[9]);
    assert(verbosity >= 0);

    KnowledgeGraph KG;
    KG.load_node_names(node_file);
    KG.load_relation_names(relation_file);
    KG.load(graph_file);
    node_file.close();
    relation_file.close();
    graph_file.close();
    assert(!node_file.is_open());
    assert(!relation_file.is_open());
    assert(!graph_file.is_open());

    //KG.save_node_names(std::ofstream("node.txt"));
    //KG.save_relation_names(std::ofstream("relation.txt"));

    size_t relation_id = KG.get_relation_id(relation_name);

    MetaPath* meta_path = new MetaPath;
    metapath_file >> *meta_path;
    metapath_file.close();
    assert(!metapath_file.is_open());

    //meta_path->print_in_list_format(std::ofstream("meta_path_list.txt"), KG);

    LogisticRegression clf_lr;
    clf_lr.load(classifier_file);
    classifier_file.close();
    assert(!classifier_file.is_open());

    auto& relation = KG.get_relations()[relation_id];
    double map = 0, map_for_src = 0, map_for_dst = 0;
    int count = 0;
    std::cout << std::endl;
    for (auto* edge: relation)
    {
        int origin_verbosity = verbosity;
        verbosity = 0;
        std::vector<std::pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> >  node_pairs;
        std::vector<int> labels;
        std::vector<KnowledgeGraph::Edge*> forbiden_edges;

        Eigen::SparseMatrix <double, Eigen::RowMajor> design_matrix;
        Eigen::VectorXd predictions;

        // std::cout << "generate_test_set..." << std::endl;
        KG.generate_test_set(edge->src, relation_id, false, node_pairs, labels, forbiden_edges);

        //KG.vector_of_node_pairs_to_os(node_pairs, std::ofstream("change_dst.txt"));
        //KG.vector_of_labels_to_os(labels, std::ofstream("change_dst_labels.txt"));

        // std::cout << "calculate_feature..." << std::endl;
        meta_path->calculate_feature(KG, node_pairs, forbiden_edges, design_matrix, thread_number);
        // std::cout << "predict..." << std::endl;
        clf_lr.predict(design_matrix, predictions);

        //std::cout << "design_matrix.nonZeros()=" << design_matrix.nonZeros() << std::endl;
        //save_mat("design_matrix.mat", "X", design_matrix.transpose());
        //std::ofstream("predictions.txt") << predictions;

        // std::cout << "AP..." << std::endl;
        double ap1 = AP(predictions, labels);
        map += ap1;
        map_for_src += ap1;

        // std::cout << "generate_test_set..." << std::endl;
        KG.generate_test_set(edge->dst, relation_id, true, node_pairs, labels, forbiden_edges);

        //KG.vector_of_node_pairs_to_os(node_pairs, std::ofstream("change_src.txt"));
        //KG.vector_of_labels_to_os(labels, std::ofstream("change_src_labels.txt"));

        // std::cout << "calculate_feature..." << std::endl;
        meta_path->calculate_feature(KG, node_pairs, forbiden_edges, design_matrix, thread_number);
        // std::cout << "predict..." << std::endl;
        clf_lr.predict(design_matrix, predictions);

        //std::cout << "design_matrix.nonZeros()=" << design_matrix.nonZeros() << std::endl;
        //save_mat("design_matrix.mat", "X", design_matrix.transpose());
        //std::ofstream("predictions.txt") << predictions;

        // std::cout << "AP..." << std::endl;
        double ap2 = AP(predictions, labels);
        map += ap2;
        map_for_dst += ap2;
        count++;

        verbosity = origin_verbosity;
        if (verbosity){
            std::cout << count << '/' << relation.size() << '\n';
            std::cout << "AP for src= " << ap1 << '\n';
            std::cout << "AP for dst= " << ap2 << std::endl;
        }
        else{
            std::cout << "\e[A" << count << '/' << relation.size() << std::endl;
        }
    }
    std::cout << std::endl;
    map = map / relation.size() / 2;
    map_for_src /= relation.size();
    map_for_dst /= relation.size();

    std::cout << "MAP= " << map << " for relation " << relation_name << std::endl;
    std::cout << "MAP for src= " << map_for_src << '\n';
    std::cout << "MAP for dst= " << map_for_dst << std::endl;

    result_file << "MAP= " << map << " for relation " << relation_name << '\n';
    result_file << "MAP for src= " << map_for_src << '\n';
    result_file << "MAP for dst= " << map_for_dst << "\n";
    return 0;
}
