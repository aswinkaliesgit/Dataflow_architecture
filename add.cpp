#include "device.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#define MATRIX_SIZE 8
#define ONE_BYTE 8
#define FLOATS_PER_PE (SIZE_PER_PE / sizeof(float))

// Function to calculate the computing units required for each input
int calculate_resource(int size)
{
  // 2*size*size represents the number of inputs (matrix_a and matrix_b)
  return static_cast<int>(std::ceil((2 * size * size) / (static_cast<float>(SIZE_PER_PE) / sizeof(float))));
}

// Helper function to load input data into processing elements
void load_data_to_pe_arrays(float *a, float *b, void *pe_arrays[], int resource_required)
{
  // In this we are loading the data in such way that a part of data from matrix_a and a part of data from matrix_b will be in a single PE
  for (int k = 0; k < resource_required; k++)
  {
    float *curr_arr_a =
        static_cast<float *>(pe_arrays[k]);
    float *curr_arr_b =
        curr_arr_a + ((SIZE_PER_PE / 2) / sizeof(float));
    for (int i = 0; i < FLOATS_PER_PE / 2; i++)
    {
      curr_arr_a[i] = a[k * (FLOATS_PER_PE / 2) + i];
      curr_arr_b[i] = b[k * (FLOATS_PER_PE / 2) + i];
    }
  }
}

// Helper function to load data from processing elements into the output array.
void load_data_from_pe_arrays(float *c, void *pe_arrays[], int resource_required)
{
  for (int l = 0; l < resource_required; l++)
  {
    float *curr_pe_addr = static_cast<float *>(pe_arrays[l]);
    float *curr_arr = curr_pe_addr + ((SIZE_PER_PE / 2) / sizeof(float));
    for (int i = 0; i < (FLOATS_PER_PE / 2); i++)
    {
      if (((FLOATS_PER_PE / 2) * l + i) <
          (MATRIX_SIZE * MATRIX_SIZE))
      {
        c[(FLOATS_PER_PE / 2) * l + i] = curr_arr[i];
      }
    }
  }
}

// Helper function to verify the results
void verify_results(float *c, float *d)
{
  for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i)
  {
    assert(std::fabs(c[i] - d[i]) < 0.001);
  }
}

// Helper function to initialize input data
void initialize_input_data(float *a, float *b, float *c, float *d)
{
  for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i)
  {
    a[i] = i + 1;
    b[i] = i + 1;
    c[i] = 0.0f;
    d[i] = 0.0f;
  }
}

// Helper function to do addition without dataflow
void compute_results_without_dataflow(float *a, float *b, float *d)
{
  for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i)
  {
    d[i] = a[i] + b[i];
  }
}

// Function to do addition with dataflow
void pe_add(void *pe_arrays[], int resource_required)
{
  for (int k = 0; k < resource_required; k++)
  {
    float *a_arr = static_cast<float *>(pe_arrays[k]);
    float *b_arr = a_arr + ((SIZE_PER_PE / 2) / sizeof(float));
    for (int i = 0; i < (FLOATS_PER_PE / 2); i++)
    {
      b_arr[i] = a_arr[i] + b_arr[i];
    }
  }
}

// Helper function to print resource utilization and verify the results
void print_resource_utilization(int resource_required, float *d, float *c)
{
  std::cout << "*******DEVICE SPECS*******\n";
  std::cout << "TOTAL PE ROWS->" << PE_ROWS << "\n";
  std::cout << "TOTAL PE COLUMNS->" << PE_COLUMNS << "\n";
  std::cout << "SIZE PER PE->" << SIZE_PER_PE << " BYTES" << "\n";
  std::cout << "Total PE's required for calculation->" << (resource_required) << " PE's\n\n";

  for (int i = 0; i < PE_ROWS; i++)
  {
    for (int j = 0; j < PE_COLUMNS; j++)
    {
      if ((i * PE_ROWS + j) < resource_required)
      {
        std::cout << "PE " << i * PE_ROWS + j + 1 << "-> " << "(" << i << "," << j << ")" << " \n";
      }
    }
  }

  verify_results(d, c);

  std::cout << "\n";
  std::cout << "TEST PASSED\n";
  std::cout << "RESULT VERIFIED SUCCESSFULLY\n";
}

// Helper function to free resources
void free_resources(void *pe_arrays[], int resource_required, float *a, float *b, float *c, float *d)
{
  for (int i = 0; i < resource_required; i++)
  {
    std::free(pe_arrays[i]);
  }

  std::free(a);
  std::free(b);
  std::free(c);
  std::free(d);
}

int main()
{
  // Allocating memory to the Hardware and slicing it to make many PEs
  const int total_pe = (PE_ROWS * PE_COLUMNS);

  void *pe_arrays[total_pe];
  for (int i = 0; i < total_pe; i++)
  {
    pe_arrays[i] = std::malloc(SIZE_PER_PE * ONE_BYTE);
  }

  float *a = static_cast<float *>(
      std::malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
  float *b = static_cast<float *>(
      std::malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
  float *c = static_cast<float *>(
      std::malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
  float *d = static_cast<float *>(
      std::malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));

  // Initializing the matrices
  initialize_input_data(a, b, c, d);

  // Doing addition without dataflow
  compute_results_without_dataflow(a, b, d);

  // Calculating the resource required
  int resource_required = calculate_resource(MATRIX_SIZE);

  // Loading the data into the Hardware
  load_data_to_pe_arrays(a, b, pe_arrays, resource_required);

  // Doing computation and taking the result out from the hardware matrix_a + matrix_b -> storing the result in the memory location of matrix_b
  pe_add(pe_arrays, resource_required);

  load_data_from_pe_arrays(c, pe_arrays, resource_required);
  
  // Printing resource utilization and verify results
  print_resource_utilization(resource_required, d, c);

  return 0;
}
