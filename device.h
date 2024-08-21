#ifndef DEVICE_H
#define DEVICE_H
//The device here is a grid of 64*64

#define PE_ROWS       64
#define PE_COLUMNS    64
// Size is Equal to 1 MB 
#define SIZE_PER_PE 125000
#define FLOATS_PER_PE 250000

#include <cnpy.h>
#include<thread>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <dnnl.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace dnnl;

float* conv_output_data[8];
// function to save the output from dnnl memory to npy
void save_to_npy(memory &output_mem, memory::dims &output_dims,
                 const std::string &filename);
// function to save the output from float array to npy
void save_to_npy(float *final_output_data, const std::string &filename,
                 memory::dims &output_dims);
// util functions to read and write data to and from dnnl memory
inline void write_to_dnnl_memory(const void *handle, dnnl::memory &mem);
inline void read_from_dnnl_memory(void *handle, const dnnl::memory &mem);
// function to print the shapes of operation and save the output
void print_and_save(memory &output_mem, std::string node_name,
                    std::string op_type, memory::dims &ip_dims,
                    memory::dims &op_dims);
// function to initialize the PE array
void intialize_pe_array(void *pe_arrays[], int total_pe);
// util functions to write data to dnnl memory

#endif // DEVICE_H