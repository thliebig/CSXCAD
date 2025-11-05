# -*- coding: utf-8 -*-
"""
Created on Tue Sep 13 20:34:40 2016

@author: thliebig
"""

# This little example demos the use of CSXCAD as little electrostatic
# field solver using a very basic finit difference (FD) method with successive
# over relaxation (SOR)
#
# it requires python-numba

from pylab import *
import time
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from CSXCAD import ContinuousStructure
from CSXCAD import CSProperties

from numba import jit

@jit
def iterate(C, V, n_iter, SOR = 1.8):
    Nx, Ny, Nz = V.shape
    for n in range(n_iter):
        for i in range(1,Nx-1):
            for j in range(1,Ny-1):
                for k in range(1,Nz-1):
                    if  C[i,j,k] == 0:
                        continue
                    new_val = V[i-1,j, k] + V[i,j-1, k] + V[i, j, k-1] + V[i+1, j, k] + V[i, j+1, k] + V[i, j, k+1]
                    V[i,j,k]  = SOR*(C[i,j,k]*new_val-V[i,j,k]) + V[i,j,k]


def Setup():
    CSX = ContinuousStructure()

    exc_1 = CSX.AddExcitation('excite_1', exc_type=1, exc_val=[5,0,0])
    exc_1.AddBox([-9.5, -5, -5], [-10.5, 5, 5])

    exc_2 = CSX.AddExcitation('excite_2', exc_type=1, exc_val=[-5,0,0])
    exc_2.AddBox([9.5, -5, -5], [10.5, 5, 5])

    metal = CSX.AddMetal('metal')
    metal.AddSphere(center=[0,0,0], radius=2.5)

    dx = 0.5

    grid = CSX.GetGrid()
    grid.SetLines('x', [-30, 30])
    grid.SmoothMeshLines('x', dx)
    grid.SetLines('y', [-15, 15])
    grid.SmoothMeshLines('y', dx)
    grid.SetLines('z', [-15, 15])
    grid.SmoothMeshLines('z', dx)

    x = grid.GetLines('x')
    y = grid.GetLines('y')
    z = grid.GetLines('z')

    Nc = len(x)*len(y)*len(z)
    print('Number of cells:', Nc)

    z0 = int(len(z)/2)

    X,Y,Z = meshgrid(x,y,z,indexing='ij')

    V = zeros(X.shape)      # voltage field
    C = ones(X.shape) * 1/6 # coefficients (for a simple homogeneous mesh only)

    CSX.Update()
    # calculate / update coefficients
    for ix, vx in enumerate(x):
        for iy, vy in enumerate(y):
            for iz, vz in enumerate(z):
                prop =  CSX.GetPropertyByCoordPriority([vx, vy, vz])
                if prop is None:
                    continue

                if prop.GetType() == CSProperties.PropertyType.EXCITATION:
                    exc = prop.GetExcitation()
                    V[ix,iy,iz] = exc[0]
                    C[ix,iy,iz] = 0       # make it a hard source (voltage source)
                elif prop.GetType() == CSProperties.PropertyType.METAL:
                    C[ix,iy,iz] = 0       # set coeff to zero --> metals will always stay at 0V or ground potential

    N_iter = 51
    t0 = time.time()
    old_E = 0
    for _ in range(20):
        iterate(C, V, 5)
        E = np.sum(V**2)
        dE = abs(E-old_E)/E
        print(f'Energy: {E:.1f} (Diff: {dE*100:.1f}%)')
        if dE < 0.01:
            print('Convergence reached')
            break
        old_E = E
    ts = time.time()-t0

    print('Speed: {:.1f} MC/s'.format(Nc*1e-6/ts*N_iter))

    Xs = np.squeeze(X[:,:,z0])
    Ys = np.squeeze(Y[:,:,z0])
    Vs = np.squeeze(V[:,:,z0])

    fig = plt.figure(facecolor='white')
    ax = fig.add_subplot(111, projection='3d')
    surf = ax.plot_surface(Xs, Ys, Vs, rstride=1, cstride=1, cmap=cm.jet)
    fig.colorbar(surf, shrink=0.5, aspect=5, label='electric potential (V)')
    ax.set_xlim([x[0], x[-1]])
    ax.set_ylim([y[0], y[-1]])
    plt.axis('equal')

    Ex, Ey, Ez = -1 * np.array(np.gradient(V)) / dx
    Eabs = np.sqrt(Ex**2 + Ey**2 + Ez**2)
    fig = plt.figure(facecolor='white')

    ax = fig.add_subplot(111)
    pc = ax.pcolor(Xs, Ys, Eabs[:,:,z0])
    plt.colorbar(pc, shrink=0.5, aspect=5, label='electric field (V/m)')
    plt.axis('equal')
    ax.set_xlim([x[0], x[-1]])
    ax.set_ylim([y[0], y[-1]])
    plt.show()


def main():
    Setup()

if __name__ == '__main__':
    main()
