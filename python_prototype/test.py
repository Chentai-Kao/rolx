import factorization
import feature
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
    n, r = v.shape # size of v
    b = 16 # number of bits of data
    # TODO
    # representation cost
    m = 0
    # errors of (V - GF)
    e = 0
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
    numRoles = min(errors, key=errors.get)
