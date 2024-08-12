# Demonstration of DataFlow Architecture using MUL op

## Description

Perform's matrix Multiplication using custom processing elements (PEs). This project demonstrates how to allocate resources for computing units and load data into processing elements for computation.

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
2. How to Build:
   ```bash
   cd dataflow_architecture
   mkdir build
   cd build
   cmake ..
   make
3. How to Run:
   ```bash
   ./mul
   ./mul-verify
4. The output will be like this for a 3X4 matrix multiplied by 4X5 matrix
   ```bash
   Output while running ./mul
   *******DEVICE SPECS*******
   TOTAL PE ROWS->512
   TOTAL PE COLUMNS->512
   SIZE PER PE->2048 BYTES
   Total PE's required for calculation->15 PE's

   110 120 130 140 150 
   246 272 298 324 350 
   382 424 466 508 550 

   Output while running ./mul-verify
   Result matrix C (A * B):
   110 120 130 140 150 
   246 272 298 324 350 
   382 424 466 508 550 
