from distutils.core import setup
from Cython.Build import cythonize

setup(
    name = 'DotBot',
    ext_modules = cythonize('dotbot.pyx'),
)
