#!/usr/bin/env python
# coding: utf-8
import argparse
import subprocess
import os


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Waiting for adding description")

    parser.add_argument('train_graph', help = "train_graph")
    parser.add_argument('test_graph', help = "test graph")
    parser.add_argument('relation', help = "relation")
    parser.add_argument('--thread', default='4', type=int, help="Thread number, default is 4")
    parser.add_argument('--length', default='3', type=int, help="Maximum length for MetaPath, default is 3")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-v", "--verbose", action="count", default=0, help="show more information")
    group.add_argument("-q", "--quiet", action="store_true", help="not display information")

    args = parser.parse_args()

    directory, train_graph_name = os.path.split(args.train_graph)
    relation_directory = os.path.join(
        directory, 
        args.relation.replace('/','_'))

    if not os.path.exists(relation_directory):
        os.mkdir(relation_directory)
    if not os.path.isdir(relation_directory):
        raise TypeError(relation_directory+'is not a directory')
    node_file = os.path.join(directory, 'node2id.txt')
    relation_file = os.path.join(directory, 'relation2id.txt')
    metapath_file =  os.path.join(relation_directory, 'metapath.txt')
    mat_file = os.path.join(relation_directory, 'X.mat')
    lable_file = os.path.join(relation_directory, 'y.txt')
    metapath_list_file = os.path.join(relation_directory, 'metapath_list.txt')
    classifier_file = os.path.join(relation_directory, 'clf_LR.txt')
    result_file = os.path.join(relation_directory, 'result.txt')
    metapath_exe = os.path.join(os.path.split(os.path.realpath(__file__))[0], './MetaPath')
    metapath_args = [
            metapath_exe,
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
        ]
    subprocess.run(metapath_args, check=True)

    LogisticRegression_script = os.path.join(os.path.split(os.path.realpath(__file__))[0], 'LogisticRegression.py')
    subprocess.run([
        "python",
        LogisticRegression_script,
        mat_file,
        lable_file,
        '--save',
        classifier_file
        ], check=True)

    KGtest_exe = os.path.join(os.path.split(os.path.realpath(__file__))[0], './KGtest')
    subprocess.run(
        [
            KGtest_exe,         # KGtest
            node_file,          # <node-file>
            relation_file,      # <relation-file>
            args.train_graph,   # <origin-graph>
            args.test_graph,    # <test-graph>
            args.relation,      # <relation>
            metapath_file,      # <metapath-file>
            classifier_file,    # <classifier-file>
            result_file,        # <result-file>
            str(args.thread),   # <thread-number>
            str(args.verbose)   # <verbosity>
        ], check=True)

