# Dataflow_architecture

## Description

Perform's matrix addition using custom processing elements (PEs). This project demonstrates how to allocate resources for computing units and load data into processing elements for computation.

## Installation

### Prerequisites

Ensure you have the following installed on your system:
- CMake (version 3.10 or higher)
- g++ (GNU Compiler Collection)
- Make

### Building the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/aswinkaliesgit/Dataflow_architecture.git
2. Create a Build Directory and run:

   Create a directory named `build` to hold the build files:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ./add
3. The output will be like this for a 8*8 matrix you can change the matrix size in the macro defined in add.cpp file
   ```bash
   *******DEVICE SPECS*******
   TOTAL PE ROWS->64
   TOTAL PE COLUMNS->64
   SIZE PER PE->1024 BYTES
   Total PE's required for calculation->2 PE's

   2 4 6 8 10 12 14 16 
   18 20 22 24 26 28 30 32 
   34 36 38 40 42 44 46 48 
   50 52 54 56 58 60 62 64 
   66 68 70 72 74 76 78 80 
   82 84 86 88 90 92 94 96 
   98 100 102 104 106 108 110 112 
   114 116 118 120 122 124 126 128 
