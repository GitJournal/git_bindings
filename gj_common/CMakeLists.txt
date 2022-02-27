cmake_minimum_required(VERSION 3.15)

project (gitjournal-git)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package(libgit2 REQUIRED)

message( INFO ${LIBGIT2_FOUND} ${LIBGIT2_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${LIBGIT2_INCLUDE_DIRS})
ADD_LIBRARY(gitjournal-git SHARED common.c keygen.c)
TARGET_LINK_LIBRARIES(gitjournal-git ${LIBGIT2_LIBRARIES})
