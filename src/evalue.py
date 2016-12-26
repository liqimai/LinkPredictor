#!/usr/bin/python
# coding: utf-8
import argparse
import subprocess
import os


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=
        "Waiting for adding description")


    parser.add_argument('train_graph', help = "train_graph")
    parser.add_argument('test_graph', help = "test graph")
    parser.add_argument('relation', help = "relation")
    parser.add_argument('--thread', default='4', type=int, help="Thread number")
    parser.add_argument('--length', default='3', type=int, help="Maximum length for MetaPath")

    args = parser.parse_args()

    directory, train_graph_name = os.path.split(args.train_graph)
    metapath_file =  os.path.join(directory, 'metapath.txt')
    mat_file = os.path.join(directory, 'X.mat')
    lable_file = os.path.join(directory,'y.txt')
    node_file = os.path.join(directory, 'node2id.txt')
    relation_file = os.path.join(directory, 'relation2id.txt')
    metapath_list_file = os.path.join(directory, 'metapath_list.txt')

    subprocess.run(
        [
            "MetaPath", 
            args.train_graph,   # <train-set>
            args.relation,      # <relation-name>
            node_file,          # <node2id-file>
            relation_file,      # <relation2id-file>
            str(args.thread),   # <max-thread>
            '1',                # <extract-metapath>
            str(args.length),   # <max-length>
            metapath_file,      # <metapath-file>
            mat_file,           # <mat-file>
            lable_file,         # <label-file>        
            metapath_list_file  # <metapath-list>
        ], 
        check=True)

    classifier_file = os.path.join(directory, 'clf_LR.txt')
    subprocess.run([
        "python",
        "LogisticRegression.py",
        mat_file,
        lable_file,
        '--save',
        classifier_file
        ], check=True)

    # subprocess.run([
    #     "python",
    #     "LogisticRegression.py"
    #     ])
