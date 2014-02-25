import numpy as np

def nonNegativeFactorization(v, r):
    """Factorize v as w*h, rank r approximation.
    
    Args:
        v: the node feature matrix
        r: the rank for rank-r approximation

    Returns:
        A tuple (w, h) such that v = w*h
    """
    n, f = v.shape
    w = np.random.rand(n, r)
    h = np.random.rand(r, f)
    # Update W
    for iteration in range(50):
        tempProduct = w.dot(h)
        for i in range(n):
            for a in range(r):
                sumU = 0
                for u in range(f):
                    if not np.allclose(tempProduct[i, u], 0):
                        sumU += v[i, u] / tempProduct[i, u] * h[a, u]
                w[i, a] *= sumU
        for i in range(n):
            for a in range(r):
                sumJ = 0
                for j in range(n):
                    sumJ += w[j, a]
                if not np.allclose(sumJ, 0):
                    w[i, a] /= sumJ
        # Update H
        for a in range(r):
            for u in range(f):
                sumI = 0
                for i in range(n):
                    if not np.allclose(tempProduct[i, u], 0):
                        sumI += w[i, a] * v[i, u] / tempProduct[i, u]
                h[a, u] *= sumI
    return w, h

if __name__ == '__main__':
    v = np.matrix('1 2; 3 4')
    r = 3
    w, h = nonNegativeFactorization(v, r)
    print '--v--'
    print v
    print '--w * h--'
    print w.dot(h)
