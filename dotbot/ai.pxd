
cdef class Dot:
    cdef int row, col, color 

cdef class Edge:
     cdef Dot p1, p2
