#!/usr/bin/python
# coding: utf-8
import scipy.io as sio
from sklearn.linear_model import LogisticRegression
import numpy as np

import matplotlib.pyplot as plt
from sklearn import decomposition

import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=
        "Train binary logistic regression classifier "
        "for Design Matrix X_mat and Binay Label y_txt")


    parser.add_argument('X_mat', help = 
        "Transposed Design Matrix or Feature Matrix in MAT-file form. "
        "The name of the Design Matrix in mat file should be 'X'. "
        "Each sample is stored in a column. "
        "Each row represents a feature. ")
    parser.add_argument('y_txt', help = 
        "Binary Label Vector. "
        "The format should keep inline with 'numpy.loadtxt' method.")
    parser.add_argument("-d","--display", action="store_true", help="plot train result")
    parser.add_argument("--penalty", default="l2", choices=["l1","l2"], help="penalty used for regularization, default is l2")
    parser.add_argument("--balanced", action="store_true", help="class_weight are balanced. See scikit-learn's documentaion for details.")
    parser.add_argument("--max-iter", default='100', type=int, help="Maximal iteration number, default is 100")
    parser.add_argument("--Lambda", default='1', type=float, help='Coefficient for l1 or l2 penalty, default is 1')
    parser.add_argument("--tolerance", default='1e-4', type=float, help='Reach tolerance, iteration will stop, default is 1e-4')
    parser.add_argument("-s","--save", help="save train result to file")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-v", "--verbose", action="count", default=0, help="show more information")
    group.add_argument("-q", "--quiet", action="store_true", help="not display information")
    args = parser.parse_args()

    clf_LR = LogisticRegression(
        C=1/args.Lambda, 
        n_jobs = -1, 
        penalty=args.penalty, 
        max_iter = args.max_iter,
        tol=args.tolerance)
    solver = {'newton-cg', 'lbfgs', 'liblinear', 'sag'}

    if args.verbose >= 2:
        clf_LR.set_params(verbose = 1)
    if args.balanced:
        clf_LR.set_params(class_weight = 'balanced')

    if not args.quiet:
        print(clf_LR)

    # Loading data...
    if not args.quiet:
        print("Loading data...")
    X = sio.loadmat(args.X_mat)['X'].transpose()
    y = np.loadtxt(args.y_txt, dtype=int)
    if args.verbose:
        print('X: ', X.shape)
        print('   ', repr(X))
        print('y: ', y.shape)
        print('   ', repr(y))

    # Train data...
    if not args.quiet:
        print("Train data...")
    clf_LR.fit(X,y)
    # if args.penalty == 'l1':
    #     clf_LR.sparsify()
    if args.verbose:
        print()
        print('Weight:   ', clf_LR.coef_.shape)
        print('          ', repr(clf_LR.coef_))
        print('intercept:', repr(clf_LR.intercept_))

    if args.save:
        if args.verbose:
            print('Saveing to', args.save)
        with open(args.save,'w') as file:
            file.write('%.16e\n'%clf_LR.intercept_[0])
            file.write('%d\n'%clf_LR.coef_.shape[1])
            for coef in clf_LR.coef_[0]:
                file.write('%.16e\n'%coef)

    # X = X.dot(clf_LR.coef_.transpose()) + clf_LR.intercept_
    # np.savetxt('predict_py.txt',X)
    if args.display:
        ax = plt.axes()

        # plt.cla()
        # pca = decomposition.PCA(n_components=2)
        # X = X.toarray()
        # pca.fit(X)
        # print('pca.explained_variance_ratio_=', pca.explained_variance_ratio_) 
        # X = pca.transform(X)

        # ax.scatter(X[:,0], X[:,1], c=y, cmap=plt.cm.rainbow)

        ax.scatter(X, y, c=y, cmap=plt.cm.rainbow)
        ax.set_xlabel('w\'X + c')
        ax.set_ylabel('Label')

        plt.show()