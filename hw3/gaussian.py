import numpy as np
U = np.array([0,-1,2,0,0,0,-1,3,0,0,0,-1,0,0,0,0]).reshape(4,4)
A = np.array([[10, -1,  2, 0],
              [-1, 11, -1, 3],
              [2, -1,  10, -1],
              [0,  3,  -1,  8]] )
L = np.array([[10, 0,  0, 0],
 	          [-1, 11, 0, 0],
              [2, -1,  10, 0],
              [0,  3,  -1,  8]] )

U = np.mat(U)
A = np.mat(A)
L = np.mat(L)

b = np.mat(np.array([6,25,-11,15]).reshape(4,1))
x = np.mat(np.zeros((4, 1), dtype=np.float32))

# print(A)
# print(L)
# print(U)

# print(L.I)


# T = L.I * U
# C = L.I * b
# print(T)
# print(C)

r = b - A*x
p = r
print(r)
a = r.T*r / (p.T*A*p)
print(a)
x = x+a[0,0]*p;
print(x)