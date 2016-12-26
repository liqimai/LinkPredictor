#include "MetaPath.h"
#include "verbosity.h"
#include <algorithm>
#include <assert.h>
#include <thread>
#include <future>
// #include <chrono>
#include <mutex>

using namespace std;

class ProgressRecord{
	mutex mux;
	size_t count;
	size_t total;
	size_t next;
public:
	inline ProgressRecord(size_t total) : total(total), count(0), next(0){}
	inline void increase(){
		mux.lock();
		count++;
		if (count >= next || count == total){
			cout << count << '/' << total << endl;
			next += total / 100;
		}
		mux.unlock();
	}
};


class ThreadParam{
public:
	int thread_id;
	int total_thread_number;
	ProgressRecord *progress_record;
	ThreadParam(){};
	ThreadParam(int thread_id, int total_thread_number, ProgressRecord *progress_record)
		: thread_id(thread_id), total_thread_number(total_thread_number), progress_record(progress_record){}
};

MetaPath* MetaPath::_extract_from(const KnowledgeGraph *_KG, const vector<KnowledgeGraph::Edge*> *edges, int max_length, void* params){
	const KnowledgeGraph& KG = *_KG;
	MetaPath* rval = NULL;
	ProgressRecord* progress_record = (ProgressRecord*)params;
	for (KnowledgeGraph::Edge *edge : *edges){
		//cout << "Find meta path for " << KG.get_node_name(edge->src->id) << ' ' << KG.get_relation_name(edge->relation_id) << ' ' << KG.get_node_name(edge->dst->id) << endl;
		auto *tmp = extract_from(KG, edge->src, edge, max_length);
		//tmp->print(cout, KG);
		//cout << endl;
		rval = merge(tmp, rval);

		if (verbosity > 1)
			progress_record->increase();
	}
	//cout << *rval << endl;
	return rval;
}



MetaPath* MetaPath::extract_from(const KnowledgeGraph& KG, string relation_name, int max_length, int thread_number){
	auto relation_id = KG.get_relation_id(relation_name);
	auto &relation = KG.relations[relation_id];
	MetaPath* rval = NULL;
	if (verbosity > 0){
		cout << "Find meta path for \"" << relation_name << '"' << endl;
		cout << "        Max Length: " << max_length << endl;
		cout << "        Edges:      " << relation.size() << endl;
		cout << "        Threads:    " << thread_number << endl;
	}

	ProgressRecord progress_record(relation.size());
	vector<vector<KnowledgeGraph::Edge*> > edges_for_thread(thread_number);

	int count = 0;
	for (KnowledgeGraph::Edge *edge : relation){
		edges_for_thread[count].push_back(edge);
		count = (count + 1 ) % thread_number;
	}

	//start mulit-thread
	future<MetaPath*>* return_values = new future<MetaPath*>[thread_number];
	for (int i = 0; i < thread_number; i++){
		return_values[i] = async(launch::async, MetaPath::_extract_from, &KG, &edges_for_thread[i], max_length, &progress_record);
	}

	//collect result
	for (int i = 0; i < thread_number; i++){
		rval = merge(rval, return_values[i].get());
	}
	delete[] return_values;

	if (verbosity > 0)
		cout << rval->get_number_of_path() << " Meta Paths Found " << endl;
	return rval;
}


MetaPath* MetaPath::extract_from(const KnowledgeGraph& KG, KnowledgeGraph::Node* src, KnowledgeGraph::Edge *forbiden_edge, int max_length){
	KnowledgeGraph::Node* dst = forbiden_edge->dst;
	MetaPath* result = new MetaPath();
	if (src == dst)
		result->reach_dst = true;

	if (max_length != 0){
		for (const pair<const size_t, vector<KnowledgeGraph::Edge*> > &relation : src->out_relations){
			MetaPath* child = NULL;
			for (const KnowledgeGraph::Edge *edge : relation.second){
				if (edge != forbiden_edge){
					child = merge(child, extract_from(KG, edge->dst, forbiden_edge, max_length - 1));
				}
			}
			if (child != NULL){
				child->relation_id = (int)relation.first;
				result->children.push_back(child);
			}
		}

		for (const pair<const size_t, vector<KnowledgeGraph::Edge*> > relation : src->in_relations){
			MetaPath* child = NULL;
			for (const KnowledgeGraph::Edge *edge : relation.second){
				if (edge != forbiden_edge){
					child = merge(child, extract_from(KG, edge->src, forbiden_edge, max_length - 1));
				}
			}
			if (child != NULL){
				child->relation_id = inverse_relation_id((int)relation.first);
				result->children.push_back(child);
			}
		}
	}


	if (result->children.size() || result->reach_dst){
		sort(result->children.begin(), result->children.end(), MetaPath::is_less_than);
		return result;
	}
	else{
		delete result;
		return NULL;
	}
}

MetaPath* MetaPath::merge(MetaPath* a, MetaPath* b){ // a and b will be destroied
	if (a != NULL)
		sort(a->children.begin(), a->children.end(), MetaPath::is_less_than);
	if (b != NULL)
		sort(b->children.begin(), b->children.end(), MetaPath::is_less_than);

	if (a == NULL)
		return b;
	if (b == NULL)
		return a;

	assert(a->relation_id == b->relation_id);
	MetaPath* rval = new MetaPath();
	rval->relation_id = a->relation_id;
	rval->reach_dst = a->reach_dst || b->reach_dst;



	auto ait = a->children.begin();
	auto bit = b->children.begin();
	while (ait != a->children.end() || bit != b->children.end() ){
		if ( bit == b->children.end()){
push_a:
			rval->children.push_back(*ait);
			ait = a->children.erase(ait);
		}
		else if (ait == a->children.end()){
push_b:
			rval->children.push_back(*bit);
			bit = b->children.erase(bit);
		}
		else if ((*ait)->relation_id < (*bit)->relation_id){
			goto push_a;
		}
		else if ((*bit)->relation_id < (*ait)->relation_id){
			goto push_b;
		}
		else{
			rval->children.push_back(merge(*ait, *bit));
			ait = a->children.erase(ait);
			bit = b->children.erase(bit);
		}
	}
	delete a;
	delete b;

	return rval;
}

int MetaPath::get_number_of_path()const {
	int rval = reach_dst ? 1 : 0;
	for (MetaPath* child : children)
		rval += child->get_number_of_path();
	return rval;
}


double MetaPath::calculate_feature(const KnowledgeGraph& KG, const KnowledgeGraph::Node* src, const KnowledgeGraph::Node* dst, 
	const KnowledgeGraph::Edge* forbiden_edge, int* path_start, int* path_end){
	if (path_end == path_start)
		if (src == dst)
			return 1;
		else
			return 0;
	double p = 0;
	int relation_id = *path_start;
	int inversed_relation_id = inverse_relation_id(relation_id);
	if (is_inversed_relation(relation_id)){
		auto itr = src->in_relations.find(inversed_relation_id);
		if (itr != src->in_relations.end()){
			size_t degree = itr->second.size();
			for (const KnowledgeGraph::Edge* edge : itr->second)
				if (edge != forbiden_edge)
					p += calculate_feature(KG, edge->src, dst, forbiden_edge, path_start + 1, path_end) / degree;
		}
	}
	else{
		auto itr = src->out_relations.find(relation_id);
		if (itr != src->out_relations.end()){
			size_t degree = itr->second.size();
			for (const KnowledgeGraph::Edge* edge : src->out_relations.at(relation_id))
				if (edge != forbiden_edge)
					p += calculate_feature(KG, edge->dst, dst, forbiden_edge, path_start + 1, path_end) / degree;
		}
	}
	return p;
}

double MetaPath::calculate_feature(const KnowledgeGraph& KG, const KnowledgeGraph::Node* src, 
	const KnowledgeGraph::Node* dst, const KnowledgeGraph::Edge* forbiden_edge, int* raw_path){
	int* path_start = raw_path + 1;
	int* path_end = raw_path + 1 + *raw_path;
	return calculate_feature(KG, src, dst, forbiden_edge, path_start, path_end);
}

void MetaPath::calculate_feature(const KnowledgeGraph& KG, const KnowledgeGraph::Node* src, const KnowledgeGraph::Node* dst, 
	const KnowledgeGraph::Edge* forbiden_edge, vector<int*>& raw_paths, vector<int>& nonzero_index, vector<double>& nonzero_value){
	int i = 0;
	for (auto raw_path : raw_paths){
		double feature_value = calculate_feature(KG, src, dst, forbiden_edge, raw_path);
		if (feature_value != 0){
			nonzero_index.push_back(i);
			nonzero_value.push_back(feature_value);
		}
		i++;
	}
}

static int* to_raw_path(vector<int>& path){
	int* raw_path = new int[path.size() + 1];
	int* current = raw_path;
	*(current++) = (int)path.size();
	for (auto relation_id : path) {
		*(current++) = relation_id;
	}
	return raw_path;
}

vector<int*> MetaPath::get_raw_paths() const {
	vector<vector<int> > meta_path_list;
	get_meta_path_list(meta_path_list);
	vector<int*> raw_paths(meta_path_list.size());
	for (size_t i = 0; i < meta_path_list.size(); i++) {
		raw_paths[i] = to_raw_path(meta_path_list[i]);
	}
	return raw_paths;

//#if _DEBUG
//	for (unsigned int i = 0; i< node_pairs.size(); i++){
//		auto& node_pair = node_pairs[i];
//		cout << "****** " << KG.get_node_name(node_pair.first->id) << " -> " << KG.get_node_name(node_pair.second->id) << " ******" << endl;
//		for (unsigned int j = 0; j < nonzero_indices[i].size(); j++){
//			cout << nonzero_values[i][j] << ' ';
//			for (auto relation_id : meta_path_list[nonzero_indices[i][j]])
//				cout << get_relation_name(KG, relation_id) << ' ';
//			cout << endl;
//		}
//	}
//#endif
}

void MetaPath::destroy_raw_path(vector<int*>& raw_paths) const {
	for (size_t i = 0; i < raw_paths.size(); i++) {
		delete[] raw_paths[i];
	}
	raw_paths.clear();
}



void MetaPath::_calculate_feature(const KnowledgeGraph* KG, const vector<pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> >* node_pairs, 
	const vector<KnowledgeGraph::Edge*>* forbiden_edges, vector<int*> *raw_paths,
	vector<vector<int> >* nonzero_indices, vector<vector<double> >* nonzero_values, void* param){

	ThreadParam *thread_param = (ThreadParam *)param;
	for (unsigned int i = 0; i< (*node_pairs).size(); i++){
		if ((i % thread_param->total_thread_number) == thread_param->thread_id){
			auto& node_pair = (*node_pairs)[i];
			calculate_feature(*KG, node_pair.first, node_pair.second, (*forbiden_edges)[i], *raw_paths, (*nonzero_indices)[i], (*nonzero_values)[i]);
			assert((*nonzero_indices)[i].size() == (*nonzero_values)[i].size());
			if (verbosity > 1)
				thread_param->progress_record->increase();
		}
	}

}


void MetaPath::calculate_feature(const KnowledgeGraph& KG, istream& node_name_pairs,
	const vector<KnowledgeGraph::Edge*>& forbiden_edges,
	Eigen::SparseMatrix<double, Eigen::RowMajor>& design_matrix, int thread_number)const {
	assert(thread_number > 0);
	assert(node_name_pairs.good());

	vector<pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> > node_pairs;
	string src_name, dst_name;
	while (!node_name_pairs.eof()){
		node_name_pairs >> src_name >> dst_name;
		assert(!node_name_pairs.fail());
		node_pairs.push_back(pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*>(
			(KnowledgeGraph::Node*) &KG.get_node(KG.get_node_id(src_name)),
			(KnowledgeGraph::Node*) &KG.get_node(KG.get_node_id(dst_name))
			));
		node_name_pairs >> ws;
	}
	calculate_feature(KG, node_pairs, forbiden_edges, design_matrix, thread_number);
}

void MetaPath::calculate_feature(const KnowledgeGraph& KG,
	const vector<pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> >& node_pairs,
	const vector<KnowledgeGraph::Edge*>& forbiden_edges,
	Eigen::SparseMatrix<double, Eigen::RowMajor>& design_matrix, int thread_number) const {
	assert(node_pairs.size() == forbiden_edges.size());
	vector<int*> raw_paths = get_raw_paths();


	vector<vector<int> > nonzero_indices(node_pairs.size());
	vector<vector<double> > nonzero_values(node_pairs.size());
	ProgressRecord progress_record(node_pairs.size());
	ThreadParam* thread_params = new ThreadParam[thread_number];
	thread* thread_pool = new thread[thread_number];
	if (verbosity > 0){
		std::cout << "calculate_feature..." << std::endl;
		std::cout << "        #pairs   : " << node_pairs.size() << std::endl;
		std::cout << "        #features: " << get_number_of_path() << std::endl;
		std::cout << "        #threads : " << thread_number << std::endl;
	}
	//start threads
	for (int i = 0; i < thread_number; i++){
		thread_params[i] = ThreadParam(i, thread_number, &progress_record);
		thread_pool[i] = thread(
			_calculate_feature, &KG, &node_pairs, &forbiden_edges, &raw_paths, &nonzero_indices, &nonzero_values, (void*)&(thread_params[i]));
	}
	//join threads
	for (int i = 0; i < thread_number; i++){
		thread_pool[i].join();
	}

	//thread_params[0] = ThreadParam(0, thread_number, &progress_record);
	//_calculate_feature(&KG, &node_pairs, &raw_paths, &nonzero_indices, &nonzero_values, (void*)&(thread_params[0]));

	//destroy thread_params, thread_pool, raw_paths
	delete[] thread_params;
	delete[] thread_pool;
	destroy_raw_path(raw_paths);

	//construct the design matrix
	vector<Eigen::Triplet<double>> tripletList;

	for (size_t i = 0, count = 0; i < nonzero_indices.size(); i++){
		auto& nonzero_index = nonzero_indices[i];
		auto& nonzero_value = nonzero_values[i];
		for (size_t j = 0; j < nonzero_index.size(); j++){
			tripletList.push_back(Eigen::Triplet<double>((int)i, (int)nonzero_index[j], nonzero_value[j]));
		}
	}
	design_matrix = Eigen::SparseMatrix<double, Eigen::RowMajor>(nonzero_indices.size(), get_number_of_path());
	design_matrix.setFromTriplets(tripletList.begin(), tripletList.end());
	design_matrix.makeCompressed();

	//save_to_mat(output_name, nonzero_indices, nonzero_values, get_number_of_path());
}

//void MetaPath::calculate_feature(KnowledgeGraph& KG, const vector<pair<string, string> >& node_name_pairs){
//	vector<pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> > node_pairs;
//	for (auto& node_id_pair : node_name_pairs){
//		node_pairs.push_back(
//			pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*>(
//			&KG.get_node(KG.get_node_id(node_id_pair.first)),
//			&KG.get_node(KG.get_node_id(node_id_pair.second))
//			));
//	}
//	calculate_feature(KG, node_pairs);
//}

void MetaPath::get_meta_path_list(vector<vector<int> >& result) const {
	vector<int> current_path;
	for (auto* child : children){
		child->_get_meta_path_list(result, current_path);
	}
}

void MetaPath::_get_meta_path_list(vector<vector<int> >& result, vector<int>& current_path) const {
	current_path.push_back(relation_id);
	if (reach_dst)
		result.push_back(current_path);
	for (auto child: children){
		child->_get_meta_path_list(result, current_path);
	}
	current_path.pop_back();
}

void MetaPath::print_in_list_format(ostream &os, const KnowledgeGraph& KG) const {
	vector<vector<int> > meta_path_list;
	get_meta_path_list(meta_path_list);
	for (auto& path : meta_path_list){
		for (auto relation_id : path)
			os << get_relation_name(KG, relation_id) << ' ';
		os << endl;
	}
}

void MetaPath::print_in_tree_format(ostream &os, const KnowledgeGraph& KG) const {
	if (relation_id != INVALID_RELATION_ID)
		os << get_relation_name(KG, relation_id) << ' ';
	else
		os << "HEAD ";
	os << reach_dst << ' ';
	for (auto* child : children){
		child->print_in_tree_format(os, KG);
	}
	os << "$ ";
}

istream& operator >> (istream &is, MetaPath& meta_path) {
	for (auto *it : meta_path.children){
		delete it;
	}
	meta_path.children.clear();
	is >> meta_path.relation_id >> meta_path.reach_dst;
	assert(!is.fail() && !is.eof());
	while (is >> std::ws, is.peek() != '$'){
		MetaPath *child = new MetaPath();
		is >> *child;
		meta_path.children.push_back(child);
	}
	is.get();
	return is;
}

ostream& operator << (ostream &os, const MetaPath& meta_path) {
	os << meta_path.relation_id << ' ' << meta_path.reach_dst << ' ';
	for (auto* child: meta_path.children){
		os << *child;
	}
	os << "$ ";
	return os;
}
