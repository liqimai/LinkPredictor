//
// Created by liqimai on 2017/1/5.
//

#include "KnowledgeGraph.h"
#include <iostream>
#include <fstream>
#include <assert.h>
int main(int argc, const char* argv[]){
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << "<graph-file>" << std::endl;
    }
    std::ifstream graph_file(argv[1]);
    assert(graph_file.is_open());
    KnowledgeGraph KG(graph_file);
    KG.print_relation_statistics(std::cout);
    return 0;
}