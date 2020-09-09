# -*- coding: utf-8 -*-
"""
Created on Fri Sep  4 15:43:19 2020

@author: Diaa Eddin Habibi
"""
#import sys
import time
from mpi4py import MPI
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

start = time.time()

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

numworkers = size - 1
#
Nx, Ny = [16,16]
X, Y = np.mgrid[:Nx,:Ny]
steps = 100
U = np.zeros([2,Nx,Ny], dtype="float")

U[0] = np.exp(-0.5*(X-(Nx-1)/2)**2/Nx**0.5)*np.exp(-0.5*(Y-(Ny-1)/2)**2/Ny**0.5)


averow = Nx//numworkers
extra = Nx% numworkers
offset = 0
#rows = 0
T = U[0].copy()

if rank == 0:
    for i in range(1,numworkers+1):
        if i <= extra:
            rows = averow + 1
        else:
            rows = averow
        
        if i == 1:
            left = None
        else:
            left = i - 1
            
        if i == numworkers:
            right = None
        else:
            right = i + 1
        
        begin = offset
        end = offset + rows - 1
    #
        comm.send(rows, dest=i, tag=1)
        comm.send(offset, dest=i, tag=2)
        comm.send(left, dest=i, tag=3)
        comm.send(right, dest=i, tag=4)
        comm.Send(T[begin:end+1], dest=i, tag = 10)
        
        # print(f"Sent to task {i}: rows = {rows}, offset = {offset}")
        # print(f"left: {left} || right: {right}")
        # print("###########################")
        
        offset += rows
        
    for i in range(1,numworkers+1):
        rows = comm.recv(source = i, tag = 5)
        offset = comm.recv(source = i,tag = 6)
        
        test = np.zeros([rows, Ny],dtype=float)
        
        begin = offset
        end = offset + rows - 1
        
        comm.Recv(test, source = i, tag = 13)
        
        U[1,begin:end+1] = test.copy()
    
    fig = plt.figure()
    ax = fig.add_subplot(111,projection='3d')
    ax.plot_surface(X,Y,U[1],cmap = "CMRmap",rstride=1, cstride=1, alpha=0.5)
    ax.contour(X,Y,U[1], linestyles="solid")
    plt.show()
        
        
else:
    T = np.zeros([2,Nx,Ny], dtype=float)
    
    neigh = np.zeros([Ny],dtype=float)
    
    rows = comm.recv(source = 0, tag = 1)
    offset = comm.recv(source = 0,tag = 2)
    left = comm.recv(source = 0, tag=3)
    right = comm.recv(source = 0, tag=4)
    #print(f"Task {rank} received: rows = {rows}, offset={offset}, left = {left}, right = {right}")
    
    begin = offset
    end = offset + rows - 1
    
    l = begin
    r = end
    
    if begin == 0:
        begin = 1
    if (offset + rows) == Nx:
        end -= 1
        
    test = np.zeros([rows, Ny],dtype=float)
    
    comm.Recv(test, source = 0, tag = 10)
    T[0,l:r+1] = test.copy()
    
    
    cx = 0.1 
    cy = 0.1
    iz = 0
    
    
    for it in range(100):    
        if left != None:
            comm.Send(T[0,l].copy(), dest=left, tag = 11)
            comm.Recv(neigh, source=left, tag = 12)
            T[0,l-1] = neigh.copy()    
        
        if right != None:
            comm.Send(T[0,r].copy(), dest=right, tag = 12)
            comm.Recv(neigh, source=right, tag = 11)
            T[0,r+1] = neigh.copy()
               
        for ix in range(begin,end+1):
            for iy in range(1,Ny-1):
                T[1,ix,iy] = T[iz,ix,iy]\
                    + cx*(T[iz,ix+1,iy] + T[iz,ix-1,iy] - 2*T[iz,ix,iy])\
                    + cy*(T[iz,ix,iy+1] + T[iz,ix,iy-1] - 2*T[iz,ix,iy])
        T[0] = T[1].copy()
    
    comm.send(rows, dest = 0, tag = 5)
    comm.send(offset, dest = 0, tag = 6)
    comm.Send(T[1,l:r+1].copy(), dest = 0, tag = 13)
#


print("--- %s seconds ---" % (time.time() - start))