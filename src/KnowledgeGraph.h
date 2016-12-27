#ifndef __KNOWLEDGEGRAPH__
#define __KNOWLEDGEGRAPH__

#include <map>
#include <vector>
#include <string>
#include <iostream>

class MetaPath;

class KnowledgeGraph
{
public:
	class Node;
	class Edge;

	class Node{
	public:
		size_t id;
		std::map<size_t, std::vector<Edge*> > out_relations;
		std::map<size_t, std::vector<Edge*> > in_relations;

		inline Node(size_t id) : id(id){}
		inline void add_edge(Edge* edge);
	};

	class Edge{
	public:
		Node* src;
		Node* dst;
		size_t relation_id;
		inline Edge(Node &src, Node &dst, size_t relation_id) :src(&src), dst(&dst), relation_id(relation_id){}
	};

public:
    inline KnowledgeGraph(){};
	KnowledgeGraph(std::istream& is){ load(is); }
    ~KnowledgeGraph(){clear();}
	//data set
	void generate_train_set(const std::string& relation_name, std::ostream& node_pairs, std::ostream& labels, 
		std::vector<Edge*>& forbiden_edges, size_t negative_samples_for_each_pair = 10) const;
	void generate_train_set(const std::string& relation_name, std::vector<std::pair<Node*, Node*> >& node_pairs, std::vector<int>& labels, 
		std::vector<Edge*>& forbiden_edges, size_t negative_samples_for_each_pair = 10) const;
	void generate_test_set(Node* node, size_t relation_id, bool change_src, 
		std::vector<std::pair<Node*, Node*> >&  node_pairs, std::vector<int>& labels, std::vector<Edge*>& forbiden_edges) const;
	void vector_of_node_pairs_to_os(const std::vector<std::pair<KnowledgeGraph::Node*, KnowledgeGraph::Node*> >&  node_pairs, std::ostream& os) const;
	void vector_of_labels_to_os(const std::vector<int>& labels, std::ostream& os) const;

	//get
	inline const std::string& get_node_name(size_t node_id) const { return id2node[node_id]; }
	inline const std::string& get_relation_name(size_t relation_id) const { return id2relation[relation_id]; }
	size_t get_node_id(const std::string &node_name) const;
	size_t get_relation_id(const std::string &relation_name) const;
    inline const std::vector<std::string>& get_relation_names() const{return id2relation;}
	inline const std::vector<std::string>& get_node_names() const{ return id2node; }
	inline const std::vector<std::vector<Edge*> >& get_relations() const{ return relations; }
	inline const std::vector<Node*>& get_nodes() const{ return nodes; }
	size_t get_facts_number() const;

	//load and save
	void save_relation_names(std::ostream& os) const;
	void save_node_names(std::ostream& os) const;
	void load_relation_names(std::istream& is);
	void load_node_names(std::istream& is);
	void load(std::istream& is);

	//clear
	void clear_nodes();
	void clear_relations();
	void clear_edges();
	void clear();
	friend std::ostream& operator << (std::ostream &is, const KnowledgeGraph& KG);
	friend class MetaPath;

private:
	std::map<std::string, size_t> relation2id;
    std::vector<std::string> id2relation;

	std::map<std::string, size_t> node2id;
    std::vector<std::string> id2node;

    std::vector<Node*> nodes;
    std::vector<std::vector<Edge*> > relations;

	inline Node& get_node(size_t id){ return *nodes[id]; }
	inline const Node& get_node(size_t id) const { return *nodes[id]; }
	size_t add_node(const std::string& node_name); //Add a new node record, if not exist. Always return the id.
	size_t add_relation(const std::string& relation_name); //Add a new relation record, if not exist. Always return the id.
	inline void add_edge(size_t src_id, size_t dst_id, size_t relation_id);
};

std::ostream& operator << (std::ostream &is, const KnowledgeGraph& KG);

#endif
