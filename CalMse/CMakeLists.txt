# Check the build path.
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "FATAL: Select a new build folder!")
endif()

# Set the minimum CMake version requirement.
cmake_minimum_required(VERSION 3.20)

# Set the root directory name to the project name.
get_filename_component(CURRENT_DIR_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(Project_Name ${CURRENT_DIR_NAME})
project(${Project_Name} CXX)

# Set C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MFC_FLAG 2)

# Check if it is a multi-configuration generator (such as Visual Studio), and set it to include only Debug and Release modes.
if(CMAKE_GENERATOR MATCHES "Visual Studio|Xcode")
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configurations" FORCE)
endif()

# Enable AVX2 instruction set.
include(CheckCXXSourceCompiles)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  check_cxx_source_compiles("
    #include <immintrin.h>
    int main() {
        __m256i a = _mm256_set1_epi32(1);
        __m256i b = _mm256_set1_epi32(2);
        __m256i c = _mm256_add_epi32(a, b);
        return 0;
    }" HAS_AVX2_SUPPORT)
  if(HAS_AVX2_SUPPORT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
  endif()
endif()

# Set the path of OpenCV.
find_package(OpenCV REQUIRED)
set(HEAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${HEAD_PATH} ${OpenCV_INCLUDE_DIRS})

## Set the eigen path.
#set(Eigen3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependence/Eigen/share/eigen3/cmake/")
#find_package(Eigen3 REQUIRED)

## Set the path for HACON.
#set(HALCON_INCLUDE_DIR "C:/Program Files/HALCON/ProgramFiles/HALCON-23.05-Progress/include")
#set(HALCON_LIBRARY "C:/Program Files/HALCON/ProgramFiles/HALCON-23.05-Progress/lib/x64-win64/halconcpp.lib")
#set(HALCON_ARCHITECTURE x64-win64)
#file(TO_CMAKE_PATH "$ENV{HALCONEXAMPLES}" HALCONEXAMPLES)
#list(APPEND CMAKE_MODULE_PATH "${HALCONEXAMPLES}/cmake")
#option(HALCON_XL "Use HALCON XL" OFF)
#include(UseHALCON)
#add_definitions("/D_AFXDLL /D_UNICODE /DUNICODE")

# Searching for header files and source files.
file(GLOB_RECURSE include_files include/*.h**)
file(GLOB_RECURSE sources_files sources/*.cpp)

# Add source files to the executable file.
set(main_file ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_executable(${Project_Name} ${main_file} ${sources_files} ${include_files})

# Link the library to the executable file.
#target_link_libraries(${Project_Name} ${OpenCV_LIBS} Eigen3::Eigen HALCON::CppInt)
target_link_libraries(${Project_Name} ${OpenCV_LIBS})

# Enable folder.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set startup project.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${Project_Name})

# Set Filter.
source_group("app" FILES ${main_file})
source_group("include" FILES ${include_files})
source_group("sources" FILES ${sources_files})

# Set the name of the folder.
set_property(TARGET ${Project_Name} PROPERTY FOLDER ${CURRENT_DIR_NAME})
