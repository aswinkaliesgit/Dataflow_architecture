# Demonstration of DataFlow Architecture using ADD-MUL op

## Description

Perform's Matrix Addition followed by Matrix Dot product using custom processing elements (PEs). This project demonstrates how to allocate resources for computing units and load data into processing elements for computation.

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
   git checkout add-mul
2. How to Build:
   ```bash
   cd dataflow_architecture
   mkdir build
   cd build
   cmake ..
   make
3. How to Run:
   ```bash
   ./add_mul
4. The output will be like this for a 8*8 matrix
   ```bash
   *******DEVICE SPECS*******
   TOTAL PE ROWS->64
   TOTAL PE COLUMNS->64
   SIZE PER PE->256 BYTES
   Total PE's required for calculation->3 PE's

   PE 1-> (0,0) 
   PE 2-> (0,1) 
   PE 3-> (0,2) 

   TEST PASSED
   RESULT VERIFIED SUCCESSFULLY