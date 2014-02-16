import cython

cdef class Point:
    cdef int row, col

cdef class ConvexHull:
    cdef Point p1, p2

@cython.locals(
        minr=cython.int,
        minc=cython.int, maxr=cython.int,
        maxc=cython.int,
        r=cython.int,
        c=cython.int)
cpdef ConvexHull find_convex_hull(object path)

@cython.locals(row=cython.int, col=cython.int)
cpdef object find_encircled_dots(object path)
