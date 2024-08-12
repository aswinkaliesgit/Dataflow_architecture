# Demonstration of DataFlow Architecture using ADD-MUL op

## Description

Perform's matrix addition followed by matrix dot product using custom processing elements (PEs). This project demonstrates how to allocate resources for computing units and load data into processing elements for computation.

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
   ./add_mul_verify
4. The output will be like this for a 8*8 matrix
   ```bash
   Output while running ./add_mul
   *******DEVICE SPECS*******
   TOTAL PE ROWS->64
   TOTAL PE COLUMNS->64
   SIZE PER PE->256 BYTES
   Total PE's required for calculation->3 PE's

   4 8 12 16 20 24 28 32 
   36 40 44 48 52 56 60 64 
   68 72 76 80 84 88 92 96 
   100 104 108 112 116 120 124 128 
   132 136 140 144 148 152 156 160 
   164 168 172 176 180 184 188 192 
   196 200 204 208 212 216 220 224 
   228 232 236 240 244 248 252 256

   Output while running ./add_mul_verify
   Resultant Matrix (a + b) * c:
   4 8 12 16 20 24 28 32 
   36 40 44 48 52 56 60 64 
   68 72 76 80 84 88 92 96 
   100 104 108 112 116 120 124 128 
   132 136 140 144 148 152 156 160 
   164 168 172 176 180 184 188 192 
   196 200 204 208 212 216 220 224 
   228 232 236 240 244 248 252 256 
