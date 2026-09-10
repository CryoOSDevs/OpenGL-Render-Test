# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-src"
  "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-build"
  "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-subbuild/tinygltf-populate-prefix"
  "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-subbuild/tinygltf-populate-prefix/tmp"
  "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-subbuild/tinygltf-populate-prefix/src/tinygltf-populate-stamp"
  "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-subbuild/tinygltf-populate-prefix/src"
  "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-subbuild/tinygltf-populate-prefix/src/tinygltf-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-subbuild/tinygltf-populate-prefix/src/tinygltf-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/andrey-vorobey/Videos/OpenGL Render/build/_deps/tinygltf-subbuild/tinygltf-populate-prefix/src/tinygltf-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
