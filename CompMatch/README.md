# CompMatch

## Installation
Please ensure that the project has the following structure.
```
CompMatch/
├── build/
├── cmake/
├── dependence/
├── include/
├── sources/
├── .gitignore
├── CMakeLists.txt
├── main.cpp
└── README.md
```

- In the "CompMatch/dependence" path:
  - git clone https://github.com/nlohmann/json.git
  - Download and extract Eigen: [Google Drive](https://drive.google.com/file/d/1lEIkpEgslo4NFGqIhurac8j8H7zFNGsB/view?usp=sharing), [Quark Drive](https://pan.quark.cn/s/8273c5391d7a?pwd=XHYa).
- In the "CompMatch/CMakeLists.txt" file:
  - Set the path for Halcon's include files in line 54: set(HALCON_INCLUDE_DIR "C:/Program Files/HALCON/ProgramFiles/HALCON-23.05-Progress/include")
  - Set the path of halconcpp.libe for Halcon in line 55: set(HALCON_LIBRARY "C:/Program Files/HALCON/ProgramFiles/HALCON-23.05-Progress/lib/x64-win64/halconcpp.lib")


## Usage
After generating the project with CMake, it can be run directly. 
