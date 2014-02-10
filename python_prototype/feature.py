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
    # generate mean and sum for every feature of existing features
    numExistingFeatures = len(existingFeatures.values()[0])
    for featureIdx in range(numExistingFeatures):
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

def getIthFeature(v, i):
    """Get the i-th feature of all nodes.

    Args:
        v: the node feature matrix.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
        i: which feature to get.

    Returns:
        A list of the i-th feature of all nodes.
    """
    features = []
    for f in v.values():
        features.append(f[i])
    return features

def isSimilar(bin1, bin2, s):
    """Checks whether log bins bin1 and bin2 are similar (s-friend).
      s-friend: all vertex agree within threshold s

    Args:
        bin1, bin2: the two bins to compare, each is a list of integer values.
            e.g. bin1 = [1, 0, 2, 2, ...]
        s: similarity threshold.

    Returns:
        True if bin1 and bin2 are similar. False otherwise.
    """
    assert len(bin1) == len(bin2)
    for i in range(len(bin1)):
        if abs(bin1[i] - bin2[i]) > s:
            return False
    return True

def pruneRecursiveFeatures(v, newFeatures, s):
    """Prune newly generated recursive features.

    Args:
        v: the node feature matrix.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
        newFeatures: the newly generated recursive features to prune.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
        s: similarity threshold.

    Returns:
        Features in new features that are retained.
        A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
        If no feature is retained, this will be an empty dictionary.
    """
    # vertical logarithmic binning
    logFeatures = verticalLogBinning(v)
    # construct feature graph (s-friend)
    numFeatures = len(logFeatures.values()[0])
    featureGraph = TUNGraph.New() # the s-friend feature graph
    for i in range(numFeatures):
        featureI = getIthFeature(v, i)
        for j in range(i + 1, numFeatures):
            featureJ = getIthFeature(v, j)
            if isSimilar(featureI, featureJ, s):
                addEdge(featureGraph, i, j)
    # TODO summarize connected component (use CnCom)
    # TODO return retained features

def recursiveFeatures(graph, v):
    """Generate recursive features.

    Args:
        graph: a graph.
        v: the node feature matrix.
            A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
            This variable is modified in-place.
    """
    s = 0 # feature similarity threshold
    retainedFeatures = v # features to be recursive generated
    while True:
        newFeatures = generateRecursiveFeatures(graph, retainedFeatures)
        retainedFeatures = pruneRecursiveFeatures(v, newFeatures, s)
        if len(retainedFeatures) == 0:
            break
        appendFeatures(v, retainedFeatures)
        s += 1

def extractFeatures(graph):
    """Extract features (neighborhood + recursive) from graph.

    Args:
        graph: a graph.

    Returns:
        the node feature matrix.
        A dictionary {node1: [f1, f2, ...], node2: [f1, f2, ...], ...}.
    """
    # create node feature matrix
    v = {}
    for node in graph.Nodes():
        v[node.GetId()] = []
    # compute features
    neighborhoodFeatures(graph, v)
    recursiveFeatures(graph, v)
    return v
