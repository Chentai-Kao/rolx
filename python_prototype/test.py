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
            elif not np.allclose(valueGF, 0):
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

def plotRole(graph, g, imgName):
    """Plot the role of each node.

    Args:
        graph: a graph.
        g: the numpy matrix in v = g*f, dimension of g is n x r.
            Each row of g represents node's membership in each role.
        imgName: the name of output image
    """
    roleToColor = {}
    roleToColor[0] = "white"
    roleToColor[1] = "black"
    roleToColor[2] = "red"
    roleToColor[3] = "green"
    roleToColor[4] = "blue"
    roleToColor[5] = "yellow"
    roleToColor[6] = "magenta"
    roleToColor[7] = "cyan"
    roleToColor[8] = "magenta"
    roleToColor[9] = "brown"
    roleToColor[10] = "gold"
    roleToColor[11] = "gray"
    roleToColor[12] = "pink"

    # collect the role of each node
    color = TIntStrH()
    roles = np.argmax(g, axis=1)
    for i in range(len(roles)):
        role = roles[i]
        color.AddDat(i, roleToColor[role])
    DrawGViz(graph, 0, imgName + '.png', 'Dot', True, color)

if __name__ == '__main__':
    # read graph
    fileName = "dataset/facebook_combined_small.txt"
    #fileName = "dataset/Email-Enron.txt"
    graph = LoadEdgeList(PUNGraph, fileName, 0, 1)

    # feature extraction
    v = feature.extractFeatures(graph)

    # convert the node feature matrix to numpy matrix
    v = toNumpyMatrix(v)

    # pick number of roles with minimum error L = M + E
    minError = sys.float_info.max
    for r in range(1, 10):
        g, f = factorization.nonNegativeFactorization(v, r)
        error = computeDescriptionLength(v, g, f)
        plotRole(graph, g, str(r) + '_roles')
        if error < minError:
            minError = error
            finalG, finalF = g, f
            numRoles = r
    print 'using ' + str(numRoles) + ' roles'
