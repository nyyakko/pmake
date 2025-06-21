# this project is abandoned

this project is abandoned. you should use its sucessor, [cmaker](https://github.com/nyyakko/cmaker), instead.

# pmake

make c++ projects from templates.

# building & installation

you will need a c++23 compatible compiler.

``cmake --preset release``\
``cmake --build build``\
``cmake --install build`` (you may want to change the [install location](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html))

# usage

``pmake --help``\
``pmake --name example``\
``pmake --name example --kind library --mode static``

