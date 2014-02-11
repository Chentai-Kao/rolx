import factorization
import feature
import math
import numpy as np
import sys
from snap import *

def computeDescriptionLength(v, g, f):
    """Compute the description length L = M + E.

    Args:
        v: the node feature matrix V
        g, f: the factorization matrix where v = g*f

    Returns:
        A number that is the description length L = M + E
    """
    # representation cost
    b = 64 # number of bits of data
    m = b * v.shape[1] * (v.shape[0] + f.shape[1])
    # errors of (V - GF)
    e = 0
    gf = g.dot(f)
    for i in range(v.shape[0]):
        for j in range(v.shape[1]):
            valueV = v[i, j]
            valueGF = gf[i, j]
            if np.allclose(valueV, 0):
                e += valueGF
            else:
                e += valueV * math.log(valueV / valueGF) - valueV + valueGF
    # return the sum of both errors
    return m + e

def toNumpyMatrix(v):
    """Convert the node feature matrix to numpy matrix.

    Args:
        v: the node feature matrix.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.

    Returns:
        The node feature matrix in an n-by-f numpy matrix.
    """
    n = len(v)
    f = len(v.values()[0])
    m = np.empty([n, f])
    nodeIdx = 0
    for nodeFeatures in v.values():
        featureIdx = 0
        for feature in nodeFeatures:
            m[nodeIdx, featureIdx] = feature
            featureIdx += 1
        nodeIdx += 1
    return m

if __name__ == '__main__':
    # read graph
    fileName = "facebook_dataset/facebook_combined_small.txt"
    graph = LoadEdgeList(PUNGraph, fileName, 0, 1)

    # feature extraction
    v = feature.extractFeatures(graph)

    # convert the node feature matrix to numpy matrix
    v = toNumpyMatrix(v)

    # pick number of roles with minimum error L = M + E
    errors = {}
    for r in range(1, 10):
        g, f = factorization.nonNegativeFactorization(v, r)
        errors[r] = computeDescriptionLength(v, g, f)
        print errors[r]
    numRoles = min(errors, key=errors.get)
    print '---role---'
    print numRoles
