FILE(GLOB UTILS_H *.h)
FILE(GLOB UTILS_HXX *.hxx)
FILE(GLOB UTILS_CPP *.cpp)

SET(LIBPFS_H ${LIBPFS_H} ${UTILS_H} ${UTILS_HXX} PARENT_SCOPE)
SET(LIBPFS_CPP ${LIBPFS_CPP} ${UTILS_CPP} PARENT_SCOPE)
