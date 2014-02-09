import numpy as np

def localFeatures(graph):
    """Generate local features.

    Args:
        graph: a graph.
    Returns:
        An n-by-f matrix. n nodes, f features each.
        It includes local features.
    """
    n = graph.GetNodes();
    v = np.zeros((n, 1))
    idx = 0
    # TODO Now only works for undirected graph
    for NI in graph.Nodes():
        v[idx, 0] = NI.GetInDeg()
        idx += 1
    return v

def egonetFeatures(graph, v):
    # TODO
    pass

def neighborhoodFeatures(graph):
    """Generate neighborhood features.

    Args:
        graph: a graph.

    Returns:
        An n-by-f matrix. n nodes, f features each.
        It includes local features and egonet features.
    """
    # 1st feature
    v = localFeatures(graph)
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
    return v
