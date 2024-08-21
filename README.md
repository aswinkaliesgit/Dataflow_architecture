# A independent C++ inference pipeline with DataFlow and Threading for single Conv2d layer with OpenSource library (DNNL)
The repo contains a independent C++ inference pipeline with DataFlow and Threading for single Conv2d layer with OpenSource library (DNNL) kernel. The Python script is used for validation purpose.
## Machine Requirements:
- Processor Architecture: ARM64
- RAM: Minimum 8GB
- OS: Ubuntu 20.04 
- Storage: Minimum 64GB
# Prequisites
* G++ (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0
* cmake version 3.29.3
* GNU Make 4.2.1
* [cnpy](https://github.com/rogersce/cnpy) 
* [DNNL](https://github.com/oneapi-src/oneDNN)
* Python 3.8.10 
# Install Prequisites
1. Build the DNNL library by referring to the [documentation](https://oneapi-src.github.io/oneDNN/dev_guide_build.html)
2. Build the cnpy library by following the steps in [documentation](https://github.com/rogersce/cnpy?tab=readme-ov-file#installation)  
# Cloning the repo
Use the command below to clone the repo
```
    git clone https://github.com/aswinkaliesgit/dataflow_architecture.git
    cd dnnl_conv2d_thread
```
Update CMake Configuration on successful prequisite installation
Open the CMakeLists.txt file in the root of the project directory and update the following
* PATH_TO_oneDNN  - Replace the <PATH_TO_oneDNN> with path to oneDNN source folder
* PATH_TO_CNPY     - Replace the <PATH_TO_CNPY> with the CNPY source folder's path  

# How to Run C++ Inference 
1. Navigate to the project directory
```
    cd dnnl_conv2d_thread
```
2. Build the cpp inference program
```
    cmake -B build -S .
    make -C build 
``` 
3. Run the program 
```
    ./build/conv_dfa_dnnl
```

# How to Run Python Inference (Used for Validation of C++ Inference Output)
1. Navigate to the project directory
```
    cd dnnl_conv2d_thread
```
2. Run the python inference script to load image, preprocess and dump output files
```
    python inference_without_split.py.py
```

# Comparing Outputs
1. All the output files are stored in outputs/ folder, Manual comparison of files can be done using the compare.py file 
```
    python compare.py <file_1.npy> <file_2.npy>
```
Sample usage
```
    $ python compare.py outputs/cpp_conv_output_merged.npy outputs/py_conv_output.npy 
```
Sample output 
```
    $  python compare.py outputs/cpp_conv_output_merged.npy outputs/py_conv_output.npy 
      Files are identical upto 4 decimals
```