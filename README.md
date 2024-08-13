# Demonstration of DataFlow Architecture using MAT-MUL op

## Description

Perform's Matrix Multiplication using threadpooling in custom processing elements (PEs). This project demonstrates how to allocate resources for computing units and load data into processing elements for computation.

## Installation

### Prerequisites

Ensure you have the following installed on your system:
- CMake (version 3.10 or higher)
- g++ (GNU Compiler Collection)
- Make

### Building the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/aswinkaliesgit/dataflow_architecture.git
   git checkout matrix_multiplication
2. How to Build:
   ```bash
   cd dataflow_architecture
   mkdir build
   cd build
   cmake ..
   make
3. How to Run:
   ```bash
   ./mat_mul 
4. The output will be like this for a 3X4 matrix multiplied by 4X5 matrix
   ```bash
   Time taken without dfa and threads: 351483 micro seconds
   Time taken without threads: 346364 micro seconds
   Time taken with threadpool: 341008 micro seconds
   *******DEVICE SPECS*******
   TOTAL PE ROWS->512
   TOTAL PE COLUMNS->512
   SIZE PER PE->2048 BYTES
   Total PE's required for calculation->250000 PE's
   TEST PASSED
   RESULT VERIFIED SUCCESSFULLY
