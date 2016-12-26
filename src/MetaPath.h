#ifndef __MATA_PATH__
#define __MATA_PATH__

#include "KnowledgeGraph.h"
#include <vector>
#include <string>
#include <iostream>
#include <Eigen/Sparse>
#define INVALID_RELATION_ID 0x0FFFFFFF

class MetaPath{
    int relation_id;
	bool reach_dst;
    std::vector<MetaPath*> children;

	//relation and inversed relation
	inline static bool is_inversed_relation(int relation_id){ return relation_id < 0; }
	inline static int inverse_relation_id(int relation_id){ return -relation_id - 1; }
	inline std::string get_relation_name(const KnowledgeGraph& KG, int relation_id) const
	{ return relation_id >= 0 ? KG.get_relation_name(relation_id) : std::string("~").append(KG.get_relation_name(inverse_relation_id(relation_id))); }

	//extract MetaPath
	static MetaPath* extract_from(const KnowledgeGraph& KG, KnowledgeGraph::Node* src, KnowledgeGraph::Edge *edge, int max_length);
	static MetaPath* _extract_from(const KnowledgeGraph* KG, const std::vector<KnowledgeGraph::Edge*> *edges, int max_length, void* params);
	static MetaPath* merge(MetaPath* a, MetaPath* b);

	//calculate_feature
	static double calculate_feature(const KnowledgeGraph& KG, const KnowledgeGraph::Node* src, const KnowledgeGraph::Node* dst, 
		const KnowledgeGraph::Edge* forbiden_edge, int* path_start, int* path_end);
	static double calculate_feature(const KnowledgeGraph& KG, const KnowledgeGraph::Node* src, const KnowledgeGraph::Node* dst, 
		const KnowledgeGraph::Edge* forbiden_edge, int* raw_path);
	static void calculate_feature(const KnowledgeGraph& KG, const KnowledgeGraph::Node* src, const KnowledgeGraph::Node* dst, 
		const KnowledgeGraph::Edge* forbiden_edge, std::vector<int*>& raw_paths, std::vector<int>& nonzero_index, std::vector<double>& nonzero_value);
	static void _calculate_feature(const KnowledgeGraph* KG, const std::vector<std::pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> >* node_pairs, 
		const std::vector<KnowledgeGraph::Edge*>* forbiden_edges, std::vector<int*>* raw_paths,
		std::vector<std::vector<int> >* nonzero_indices, std::vector<std::vector<double> >* nonzero_values, void* param);
	//static void MetaPath::save_to_mat(const char* output_name, std::vector<std::vector<int> > nonzero_indices, std::vector<std::vector<double> > nonzero_values, int path_number);
	void _get_meta_path_list(std::vector<std::vector<int> >& result, std::vector<int>& current_path) const ;
	void destroy_raw_path(std::vector<int*>& raw_paths) const;
	std::vector<int*> get_raw_paths() const;

public:
	static bool is_less_than(const MetaPath* a, const MetaPath* b){ return a->relation_id < b->relation_id; }
	int get_number_of_path()const;

	//Extract meta path from KG for relation <relation_name>.
	static MetaPath* extract_from(const KnowledgeGraph& KG, std::string relation_name, int max_length, int thread_number);
	// Design Matrix. Each sample is stored in a row. Each column represents a feature.
	void calculate_feature(const KnowledgeGraph& KG, std::istream& node_pairs,
		const std::vector<KnowledgeGraph::Edge*>& forbiden_edges, Eigen::SparseMatrix<double, Eigen::RowMajor>& design_matrix, int thread_number = 1) const;
	void calculate_feature(const KnowledgeGraph& KG, const std::vector<std::pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> >& node_pairs,
		const std::vector<KnowledgeGraph::Edge*>& forbiden_edges, Eigen::SparseMatrix<double, Eigen::RowMajor>& design_matrix, int thread_number = 1) const;

	//Human readable
	void get_meta_path_list(std::vector<std::vector<int> >& result) const;
	void print_in_tree_format(std::ostream &os, const KnowledgeGraph& KG) const;
	void print_in_list_format(std::ostream &os, const KnowledgeGraph& KG) const;

	//load and save
	friend std::istream& operator >> (std::istream &is, MetaPath& meta_path);
	friend std::ostream& operator << (std::ostream &is, const MetaPath& meta_path);
    
	inline MetaPath(int relation_id, bool reach_dst = false) : relation_id(relation_id), reach_dst(reach_dst){}
	inline MetaPath() : relation_id(INVALID_RELATION_ID), reach_dst(false){}
    inline ~MetaPath(){
        for(MetaPath* meta_path : children){
            delete meta_path;
        }
    }
};

#endif