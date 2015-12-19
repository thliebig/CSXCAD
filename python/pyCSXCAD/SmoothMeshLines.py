# -*- coding: utf-8 -*-
"""
Created on Fri Dec 18 21:26:54 2015

@author: thorsten
"""

import numpy as np
import pylab as plt

def MeshLinesSymmetric(l, rel_tol=1e-6):
    l = np.array(l)
    N = len(l)
    rng = l[-1]-l[0]
    if N%2==0: # even
        mid_val = 0.5*(l[0] + l[-1])
    else:      # off
        mid_val = l[(N-1)/2]

    for n in range(int(N/2)):
        if np.abs(mid_val - np.sum((l[n], l[N-n-1])))>rng*rel_tol:
            return False

    return True

def Unique(l, tol=1e-7):
    l = np.unique(l)
    dl = np.diff(l)
    idx = np.where(dl<np.mean(dl)*tol)[0]
    if len(idx)>0:
        l = np.delete(l, idx)

    return l

def SmoothRange(start, stop, start_res, stop_res, max_res, ratio):
    assert ratio>1
    rng = (stop-start)

    # very small range
    if rng<max_res and rng<start_res*ratio and rng<stop_res*ratio:
        return Unique([start, stop])

    # very large range and easy start/stop res
    if start_res>=(max_res/ratio) and stop_res>=(max_res/ratio):
        N = np.ceil(rng/max_res)
        return np.linspace(start, stop, N+1)

    def one_side_taper(start_res, ratio, max_res):
        res = start_res
        pos = 0
        N = 0
        while res<max_res and pos<rng:
            res *= ratio
            pos += res
            N += 1
        if pos>rng:
            l = np.zeros(N+1)
            for n in range(N+1):
                l[n] = np.sum(start_res * ratio ** np.arange(1,n+1))
            return l * rng/pos

        _ratio = np.e**((np.log(max_res) - np.log(start_res))/(N))

        l = [0]
        pos = 0
        res = start_res
        for n in range(N):
            res *= _ratio
            pos += res
            l.append(pos)

        while pos<rng:
            pos += max_res
            l.append(pos)

        return np.array(l) * rng/l[-1]

    # need to taper start
    if start_res<(max_res/ratio) and stop_res>=(max_res/ratio):
        return start + one_side_taper(start_res, ratio, max_res)

    # need to taper stop
    if start_res>=(max_res/ratio) and stop_res<(max_res/ratio):
        return np.sort(stop - one_side_taper(stop_res, ratio, max_res))

    # test if max taper on both sides is possible
    pos1 = 0
    N1 = 0
    res = start_res
    while res<max_res:
        res *= ratio
        pos1 += res
        N1 += 1
    ratio1 = np.e**((np.log(max_res) - np.log(start_res))/(N1))
    pos1 = np.sum(start_res * ratio1 ** np.arange(1,N1+1))

    pos2 = 0
    N2 = 0
    res = stop_res
    while res<max_res:
        res *= ratio
        pos2 += res
        N2 += 1
    ratio2 = np.e**((np.log(max_res) - np.log(stop_res))/(N2))
    pos2 = np.sum(stop_res * ratio2 ** np.arange(1,N2+1))

    if (pos1+pos2)<rng:
        l = [0]
        for n in range(1,N1+1):
            l.append(l[-1]+start_res*ratio1**n)
        r = [0]
        for n in range(1,N2+1):
            r.append(r[-1]+stop_res*ratio2**n)

        left = rng-pos1-pos2
        N = int(np.ceil(left/max_res))

        for n in range(N):
            l.append(l[-1]+max_res)

        length = l[-1]+r[-1]
        c = Unique(np.r_[np.array(l), length-np.array(r)])
        return start + c *rng/length

    l = [0]
    r = [0]
    while l[-1]+r[-1]<rng:
        if start_res<stop_res:
            start_res *= ratio
            l.append(l[-1]+start_res)
        else:
            stop_res  *= ratio
            r.append(r[-1]+start_res)

    length = l[-1]+r[-1]
    c = Unique(np.r_[np.array(l), length-np.array(r)])
    return start + c *rng/length


def SmoothMeshLines(lines, max_res, ratio=1.5, **kw):
    out_l = Unique(lines)
    dl = np.diff(out_l)

    while len(np.where(dl>max_res)[0])>0:
        N = len(out_l)
        dl[dl<=max_res] = np.max(dl)*2
        idx = np.argmin(dl)
        dl = np.diff(out_l)
#        print("dl", dl)
#        print(idx)
        if idx>0:
            start_res = dl[idx-1]
        else:
            start_res = max_res
        if idx<len(dl)-1:
            stop_res = dl[idx+1]
        else:
            stop_res = max_res
#        print(out_l[idx], out_l[idx+1], start_res, stop_res, max_res, ratio)
        l = SmoothRange(out_l[idx], out_l[idx+1], start_res, stop_res, max_res, ratio)
        out_l = Unique(np.r_[out_l, l])
        dl = np.diff(out_l)

        if len(out_l)==N:
            break

    return Unique(out_l)


if __name__ == "__main__":

#    print(SmoothRange(0., 1., 10., 50., 25., 1.5))
#    print(SmoothRange(0., 100., 1., 50., 25., 1.5))
#    print(SmoothRange(0., 100., 50., 1., 25., 1.5))
#    print(SmoothRange(0., 135., 5., 1.5, 25., 1.5))
#    print(SmoothRange(0., 10., 6., 6.5, 25., 1.5))

    l = [-100, -50,  50, 100]
    l = [ -50., 100., 0., 0.381, 0.762, 1.143, 1.524]
#
    ol = SmoothMeshLines(l, 4.996540966666666)
    print(ol)