import numpy as np

def localFeatures(graph, v):
    """Generate local features.

    Args:
        graph: a graph.
        v: the node feature matrix.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
            This variable is modified in-place.
    """
    # TODO Need to consider in-degree and out-degree for directed graph
    for node in graph.Nodes():
        v[node.GetId()].append(node.GetInDeg())

def egonetFeatures(graph, v):
    """Generate egonet features.

    Args:
        graph: a graph.
        v: the node feature matrix.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
            This variable is modified in-place.
    """
    for node in graph.Nodes():
        # construct the egonet (self + neighbors)
        egonet = []
        egonet.append(node.GetId())
        for neighbor in node.GetInEdges():
            egonet.append(neighbor)
        # compute features
        numWithinEgonetEdges = node.GetInDeg()
        numAroundEgonetEdges = 0
        for neighbor in node.GetInEdges():
            neighborNode = graph.GetNI(neighbor)
            for candidate in neighborNode.GetInEdges():
                if candidate in egonet:
                    numWithinEgonetEdges += 1
                else:
                    numAroundEgonetEdges += 1
        # store the features
        v[node.GetId()].append(numWithinEgonetEdges)
        v[node.GetId()].append(numAroundEgonetEdges)

def neighborhoodFeatures(graph, v):
    """Generate neighborhood features.

    Args:
        graph: a graph.
        v: the node feature matrix.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
            This variable is modified in-place.
    """
    localFeatures(graph, v)
    egonetFeatures(graph, v)

def generateRecursiveFeatures(graph, existingFeatures):
    """Generate recursive features from existing features.

    Args:
        graph: a graph.
        existingFeatures: the node feature matrix of existing features.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
            This variable is modified in-place.
    Returns:
        the node feature matrix of generated features.
        A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
    """
    # create the dict of new features
    newFeatures = {}
    for node in existingFeatures.keys():
        newFeatures[node] = []
    # generate mean and sum for every feature of retained features
    numRetainedFeatures = len(existingFeatures.values()[0])
    for featureIdx in range(numRetainedFeatures):
        for node in graph.Nodes():
            # collect feature value of neighbors
            neighborFeatures = []
            for neighbor in node.GetInEdges():
                f = existingFeatures[neighbor][featureIdx]
                neighborFeatures.append(f)
            # compute mean and sum
            count = len(neighborFeatures)
            featureSum = sum(neighborFeatures)
            featureMean = 0 if count == 0 else float(featureSum) / count
            # add to new feature
            newFeatures[node.GetId()].append(featureSum)
            newFeatures[node.GetId()].append(featureMean)
    return newFeatures

def appendFeatures(dstFeatures, srcFeatures):
    """Append features to a node feature matrix.

    Args:
        dstFeatures: the node feature matrix to append to.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
            This variable is modified in-place.
        srcFeatures: the node feature matrix to append from.
            Same type as dstFeatures.
    """
    assert set(dstFeatures.keys()) == set(srcFeatures.keys())
    for node, features in srcFeatures.iteritems():
        for f in features:
            dstFeatures[node].append(f)

def recursiveFeatures(graph, v):
    """Generate recursive features.

    Args:
        graph: a graph.
        v: the node feature matrix.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
            This variable is modified in-place.
    """
    featureGraph = TUNGraph.New() # the s-friend feature graph
    s = 0 # feature similarity threshold
    retainedFeatures = v # features to be recursive generated
    while True:
        newFeatures = generateRecursiveFeatures(graph, retainedFeatures)
        retainedFeatures = pruneRecursiveFeatures(
                featureGraph, v, newFeatures, s)
        if len(retainedFeatures) == 0:
            break
        appendFeatures(v, retainedFeatures)
        s += 1

def extractFeatures(graph):
    # create node feature matrix
    v = {}
    for node in graph.Nodes():
        v[node.GetId()] = []
    # compute features
    neighborhoodFeatures(graph, v)
    recursiveFeatures(graph, v)
    return v
