import numpy as np

def localFeatures(graph, v):
    # TODO
    pass

def egonetFeatures(graph, v):
    # TODO
    pass

def neighborhoodFeatures(graph):
    """Generate neighborhood features.

    Args:
        graph: a graph.

    Returns:
        An n-by-3 matrix. n nodes, 3 features each.
        1st feature is the local feature.
        2nd and 3rd features are egonet features.
    """
    n = graph.GetNodes()
    v = np.zeros((n, 3)) # n nodes, 3 features
    # 1st feature
    localFeatures(graph, v)
    # 2nd and 3rd feature
    egonetFeatures(graph, v)
    return v

def recursiveFeatures(graph, v):
    """Generate recursive features.

    Args:
        graph: a graph.
        v: the n-by-f node feature matrix. Modify this matrix in-place.
    """
    # TODO
    pass

def extractFeatures(graph):
    v = neighborhoodFeatures(graph)
    recursiveFeatures(graph, v)

    v = np.matrix('1 2; 3 4')
    return v
