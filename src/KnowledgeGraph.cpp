#include "KnowledgeGraph.h"
#include <assert.h>
#include <random>
#include <chrono>
using namespace std;

size_t KnowledgeGraph::get_node_id(const std::string &node_name) const {
    auto itr = node2id.find(node_name);
    assert(itr != node2id.end());
    return itr->second;
}


size_t KnowledgeGraph::get_relation_id(const std::string &relation_name) const {
    auto itr = relation2id.find(relation_name);
    assert(itr != relation2id.end());
    return itr->second; 
}

size_t KnowledgeGraph::add_node(const string& node_name){
    auto it = node2id.find(node_name);
    if(it == node2id.end()){
        size_t node_id = node2id.size();
        id2node.push_back(node_name);
        node2id[node_name] = node_id;
        nodes.push_back(new Node(node_id));
        return node_id;
    }
    else{
        return it->second;
    }
}

void KnowledgeGraph::clear_edges(){
    for (std::vector<Edge*>& relation : relations){
        for (Edge* edge : relation){
            delete edge;
        }
        relation.clear();
    }
    for (Node* node : nodes){
        node->in_relations.clear();
        node->out_relations.clear();
    }
}

void KnowledgeGraph::clear_nodes(){
    clear_edges();
    for (Node* node : nodes){
        delete node;
    }
    nodes.clear();
    node2id.clear();
    id2node.clear();
}

void KnowledgeGraph::clear_relations(){
    clear_edges();
    relations.clear();
    relation2id.clear();
    id2relation.clear();
}

size_t KnowledgeGraph::add_relation(const string& relation_name){
    auto it = relation2id.find(relation_name);
    if(it == relation2id.end()){
        size_t relation_id = relation2id.size();
        id2relation.push_back(relation_name);
        relation2id[relation_name] = relation_id;
        relations.push_back(std::vector<Edge*>());
        return relation_id;
    }
    else{
        return it->second;
    }
}

inline void KnowledgeGraph::Node::add_edge(Edge* edge){
    assert(edge->src == this || edge->dst == this);
    if (edge->src == this)
        out_relations[edge->relation_id].push_back(edge);
    if (edge->dst == this)
        in_relations[edge->relation_id].push_back(edge);
}

KnowledgeGraph::Edge * KnowledgeGraph::add_edge(size_t src_id, size_t dst_id, size_t relation_id){
    Edge *edge = new Edge(get_node(src_id), get_node(dst_id), relation_id);
    relations[relation_id].push_back(edge);
    edge->src->add_edge(edge);
    edge->dst->add_edge(edge);
    return edge;
}

void KnowledgeGraph::load(istream& is){
    clear_edges();
    assert(is.good());
    string src, relation, dst;
    while(!is.eof()){
        is >> src >> dst >> relation;
        //cout << src << ' ' << relation << ' ' << dst << endl;
        assert(!is.fail());
        size_t src_id = add_node(src);
        size_t dst_id = add_node(dst);
        size_t relation_id = add_relation(relation);
        add_edge(src_id, dst_id, relation_id);
        std::ws(is);
    }
    cout << "Load successfully!" << endl;
    cout << "#nodes     " << nodes.size() << endl;
    cout << "#relations " << relation2id.size() << endl;
    cout << "#facts     " << get_facts_number() << endl;
}

void KnowledgeGraph::load_test(istream &is, vector<vector<Edge*> > &test_edge){
    assert(is.good());
    test_edge.clear();
    test_edge.resize(relations.size());
    string src, relation, dst;
    while(!is.eof()){
        is >> src >> dst >> relation;
        //cout << src << ' ' << relation << ' ' << dst << endl;
        assert(!is.fail());
        size_t src_id = add_node(src);
        size_t dst_id = add_node(dst);
        size_t relation_id = add_relation(relation);
        test_edge[relation_id].push_back(add_edge(src_id, dst_id, relation_id));
        std::ws(is);
    }
    //TODO: check to ensure that there is no new node or relation added.
    cout << "Load successfully!" << endl;
    cout << "#nodes     " << nodes.size() << endl;
    cout << "#relations " << relation2id.size() << endl;
    cout << "#facts     " << get_facts_number() << endl;
}
void KnowledgeGraph::clear(){
    clear_edges();
    clear_nodes();
    clear_relations();
}

size_t KnowledgeGraph::get_facts_number() const{
    size_t in_degree = 0;
    size_t out_degree = 0;
    for (const KnowledgeGraph::Node *node : nodes){
        for (auto& relation : node->out_relations){
            out_degree += relation.second.size();
        }
        for (auto& relation : node->in_relations){
            in_degree += relation.second.size();
        }
    }
    assert(in_degree == out_degree);
    return in_degree;
}

std::ostream& operator << (std::ostream &is, const KnowledgeGraph& KG){
    cout << "#nodes     " << KG.nodes.size() << endl;
    cout << "#relations " << KG.relation2id.size() << endl;
    cout << "#facts     " << KG.get_facts_number() << endl;
    for (const KnowledgeGraph::Node *node : KG.nodes){
        cout << "Node_id: " << node->id << endl;
        cout << "Name:    " << KG.get_node_name(node->id) << endl;
        for (auto& relation : node->out_relations){
            //cout << "    Out-Relation id:   " << relation.first << endl;
            //cout << "    Out-Relation name: " << KG.get_relation_name(relation.first) << endl;
            //cout << "    Dst List:         ";
            for (const KnowledgeGraph::Edge *edge : relation.second){
                cout << "    " << KG.get_node_name(edge->src->id) << ' ' << KG.get_relation_name(edge->relation_id) << ' ' << KG.get_node_name(edge->dst->id) << endl;
            }
        }
        for (auto& relation : node->in_relations){
            //cout << "    In-Relation id:    " << relation.first << endl;
            //cout << "    In-Relation name:  " << KG.get_relation_name(relation.first) << endl;
            //cout << "    Src List:          ";
            for (const KnowledgeGraph::Edge *edge : relation.second){
                cout << "    " << KG.get_node_name(edge->src->id) << ' ' << KG.get_relation_name(edge->relation_id) << ' ' << KG.get_node_name(edge->dst->id) << endl;
            }
        }
    }
    cout << endl << "Edge List:" << endl;
    for (const std::vector<KnowledgeGraph::Edge*>& relation : KG.relations){
        for (KnowledgeGraph::Edge* edge : relation){
            cout << KG.get_node_name(edge->src->id) << ' ' << KG.get_relation_name(edge->relation_id) << ' ' << KG.get_node_name(edge->dst->id) << endl;
        }
    }
    return is;
}

void KnowledgeGraph::save_relation_names(ostream& os) const{
    assert(os.good());
    for (auto relation_name : id2relation){
        os << relation_name << endl;
        assert(!os.fail());
    }
}

void KnowledgeGraph::save_node_names(ostream& os) const{
    assert(os.good());
    for (auto node_name : id2node){
        os << node_name << endl;
        assert(!os.fail());
    }
}

void KnowledgeGraph::load_relation_names(istream& is){
    assert(is.good());
    clear_relations();
    string relation_name;
    while (!is.eof()){
        is >> relation_name;
        assert(!is.fail());
        add_relation(relation_name);
        std::ws(is);
    }
}

void KnowledgeGraph::load_node_names(istream& is){
    assert(is.good());
    clear_nodes();
    string node_name;
    while (!is.eof()){
        is >> node_name;
        assert(!is.fail());
        add_node(node_name);
        std::ws(is);
    }
}

void KnowledgeGraph::generate_train_set(const string& relation_name, ostream& node_pairs, ostream& labels, vector<Edge*>& forbiden_edges, size_t negative_samples_for_each_pair) const{
    assert(node_pairs.good());
    assert(labels.good());
    vector<pair<Node*, Node*> > _node_pairs;
    vector<int> _labels;
    generate_train_set(relation_name, _node_pairs, _labels, forbiden_edges, negative_samples_for_each_pair);
    vector_of_node_pairs_to_os(_node_pairs, node_pairs);
    vector_of_labels_to_os(_labels, labels);
}

void KnowledgeGraph::vector_of_node_pairs_to_os(const vector<pair<Node*, Node*> >&  node_pairs, std::ostream& os) const{
    assert(os.good());
    for (auto& node_pair : node_pairs){
        os << get_node_name(node_pair.first->id) << '\t' << get_node_name(node_pair.second->id) << '\n';
    }
}

void KnowledgeGraph::vector_of_labels_to_os(const std::vector<int>& labels, std::ostream& os) const{
    assert(os.good());
    for (auto i : labels){
        os << i << '\n';
    }
}

void KnowledgeGraph::generate_train_set(
    const string& relation_name,
    vector<pair<Node*, Node*> >& node_pairs,
    vector<int>& labels,
    vector<Edge*>& forbiden_edges,
    size_t negative_samples_for_each_pair) const{
    assert(negative_samples_for_each_pair > 0);
    node_pairs.clear();
    labels.clear();
    forbiden_edges.clear();

    auto& relation = relations[get_relation_id(relation_name)];

    //labels
    //    << "%%MatrixMarket matrix array integer general\n"
    //    << "% train set for relation \"" << relation_name << "\"\n"
    //    << relation.size()*relation.size() << ' 1' << endl;
    
    node_pairs.reserve((negative_samples_for_each_pair+1)*relation.size());
    labels.reserve((negative_samples_for_each_pair + 1)*relation.size());
    forbiden_edges.reserve((negative_samples_for_each_pair + 1)*relation.size());

    /* generate  number_of_negative_samples_for_each_pair negatives pairs*/
    size_t change_head = negative_samples_for_each_pair / 2;
    size_t change_tail = negative_samples_for_each_pair - change_head;
    size_t seed = std::chrono::system_clock::now().time_since_epoch().count();
#if _DEBUG > 0     
        seed = 1;
#endif
    std::default_random_engine random_engine((unsigned int)seed);
    std::uniform_int_distribution<size_t> distribution(0, relation.size() - 1);

    for (auto* edge : relation){
        node_pairs.push_back(pair<Node*, Node*>(edge->src, edge->dst));
        labels.push_back(1);
        forbiden_edges.push_back(edge);

        //node_pairs << get_node_name(edge->src->id) << '\t' << get_node_name(edge->dst->id) << endl;
        //labels << 1 << endl;

        for (size_t i = 0; i < change_head; i++){
            size_t index;
            do{
                index = distribution(random_engine);
            } while (relation[index]->src->id == edge->src->id);

            node_pairs.push_back(pair<Node*, Node*>(relation[index]->src, edge->dst));
            labels.push_back(0);
            forbiden_edges.push_back(NULL);
            //node_pairs << get_node_name(relation[index]->src->id) << '\t' << get_node_name(edge->dst->id) << endl;
            //labels << 0 << endl;
        }


        for (size_t i = 0; i < change_tail; i++){
            size_t index;
            do{
                index = distribution(random_engine);
            } while (relation[index]->dst->id == edge->dst->id);

            node_pairs.push_back(pair<Node*, Node*>(edge->src, relation[index]->dst));
            labels.push_back(0);
            forbiden_edges.push_back(NULL);
            //node_pairs << get_node_name(edge->src->id) << '\t' << get_node_name(relation[index]->dst->id) << endl;
            //labels << 0 << endl;
        }
    }
}

void KnowledgeGraph::generate_test_set(Edge* test_edge, size_t relation_id, bool change_src,
    vector<pair<Node*, Node*> >& node_pairs, vector<int>& labels, std::vector<Edge*>& forbiden_edges) const{
    Node* node = change_src? test_edge->dst : test_edge->src;
    node_pairs.clear();
    labels.clear();
    forbiden_edges.clear();
    node_pairs.reserve(nodes.size());
    labels.reserve(nodes.size());
    forbiden_edges.reserve(nodes.size());

    if (change_src)
    {
        auto& relation = node->in_relations[relation_id];
        for (auto another_node : nodes){
            node_pairs.push_back(pair<Node*, Node*>(another_node, node));
            int label = 0;
            Edge* forbiden_edge = NULL;
            if(another_node == test_edge->src){
                label = 2;
            } else{
                for (auto* edge : relation){
                    if (another_node == edge->src){
                        label = 1;
                        forbiden_edge = edge;
                        break;
                    }
                }
            }
            labels.push_back(label);
            forbiden_edges.push_back(forbiden_edge);
        }
    }
    else{
        auto& relation = node->out_relations[relation_id];
        for (auto another_node : nodes){
            node_pairs.push_back(pair<Node*, Node*>(node, another_node));
            int label = 0;
            Edge* forbiden_edge = NULL;
            if(another_node == test_edge->dst){
                label = 2;
            } else {
                for (auto *edge : relation) {
                    if (another_node == edge->dst) {
                        label = 1;
                        forbiden_edge = edge;
                        break;
                    }
                }
            }
            labels.push_back(label);
            forbiden_edges.push_back(forbiden_edge);
        }
    }
}

void KnowledgeGraph::print_relation_statistics(std::ostream &os) const {
    assert(os.good());
    vector<int> relation_indegree(relations.size());
    vector<int> relation_outdegree(relations.size());
    vector<int> relation_indegree_count(relations.size());
    vector<int> relation_outdegree_count(relations.size());
    vector<int> relation_symmetry(relations.size());
    vector<int> relation_symmetry_count(relations.size());
    for (auto* node: nodes) {
        for(auto& relation: node->out_relations){
            size_t relation_id = relation.first;
            relation_outdegree[relation_id] += relation.second.size();
            relation_outdegree_count[relation_id] += relation.second.size()? 1:0;
            for(auto* edge: relation.second){
                relation_symmetry_count[relation_id]++;
                for(auto* another_edge : edge->dst->out_relations[relation_id]){
                    if(another_edge->dst == edge->src){
                        relation_symmetry[relation_id] ++;
                        break;
                    }
                }
            }
        }
        for(auto& relation: node->in_relations){
            size_t relation_id = relation.first;
            relation_indegree[relation_id] += relation.second.size();
            relation_indegree_count[relation_id] += relation.second.size()? 1:0;
        }
    }
    cout << "relation\tindegree\toutdegree\tsymmetry\tnumber\n";
    for(int i = 0; i < relations.size(); i++){
        cout << id2relation[i] << '\t'
             << (double)relation_indegree[i]/relation_indegree_count[i] << '\t'
             << (double)relation_outdegree[i]/relation_outdegree_count[i] << '\t'
             << (double)relation_symmetry[i]/relation_symmetry_count[i] << '\t'
             << relations[i].size()
             << std::endl;
    }
}
