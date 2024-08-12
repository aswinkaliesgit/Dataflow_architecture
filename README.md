# Demonstration of DataFlow Architecture using MAT-MUL op

## Description

Perform's Matrix Multiplication using custom processing elements (PEs). This project demonstrates how to allocate resources for computing units and load data into processing elements for computation.

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
   *******DEVICE SPECS*******
   TOTAL PE ROWS->512
   TOTAL PE COLUMNS->512
   SIZE PER PE->2048 BYTES
   Total PE's required for calculation->15 PE's

   PE 0-> (0,0) 
   PE 1-> (0,1) 
   PE 2-> (0,2) 
   PE 3-> (0,3) 
   PE 4-> (0,4) 
   PE 5-> (0,5) 
   PE 6-> (0,6) 
   PE 7-> (0,7) 
   PE 8-> (0,8) 
   PE 9-> (0,9) 
   PE 10-> (0,10) 
   PE 11-> (0,11) 
   PE 12-> (0,12) 
   PE 13-> (0,13) 
   PE 14-> (0,14) 

   TEST PASSED
   RESULT VERIFIED SUCCESSFULLY
