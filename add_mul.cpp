#include "device.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define MATRIX_SIZE 8
#define ONE_BYTE 8
#define FLOATS_PER_PE (SIZE_PER_PE / sizeof(float))

// Function to initialize the input and output Buffers
void initialize_input_data(float *a, float *b, float *c, float *d, float *e)
{
  for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i)
  {
    a[i] = i + 1;
    b[i] = i + 1;
    c[i] = 0.0f;
    d[i] = 2.0;
    e[i] = 0.0f;
  }
}

// Function to calculate the computing units required for each input
int calculate_resource(int size)
{
  // Here 3*size*size is the total number of input matrix
  return static_cast<int>(std::ceil(
      (3 * size * size) / (static_cast<float>(SIZE_PER_PE) / sizeof(float))));
}

// Helper function to load input data into processing elements
void load_data_to_pe_arrays(float *a, float *b, void *pe_arrays[],
                            int resource_required_start,
                            int resource_required_end)
{
  // In this we are loading the data in such way that a part of data from
  // matrix_a and a part of data from matrix_b will be in a single PE
  int t = 0;
  for (int k = resource_required_start; k < resource_required_end; k++)
  {
    float *curr_arr_a = static_cast<float *>(pe_arrays[k]);
    float *curr_arr_b = curr_arr_a + ((SIZE_PER_PE / 2) / sizeof(float));
    for (int i = 0; i < FLOATS_PER_PE / 2; i++)
    {
      curr_arr_a[i] = a[t * (FLOATS_PER_PE / 2) + i];
      curr_arr_b[i] = b[t * (FLOATS_PER_PE / 2) + i];
    }
    t++;
  }
}

// Helper function to do addition between two matrix in processing elements
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

// Helper function to do multiplication between two matrix in processing elements
void pe_mul(void *pe_arrays[], int resource_required)
{
  for (int k = 0; k < resource_required; k++)
  {
    float *a_arr = static_cast<float *>(pe_arrays[k]);
    float *b_arr = a_arr + ((SIZE_PER_PE / 2) / sizeof(float));
    for (int i = 0; i < (FLOATS_PER_PE / 2); i++)
    {
      b_arr[i] = a_arr[i] * b_arr[i];
    }
  }
}

// Helper function to load output data from processing elements
void load_data_from_pe_arrays(float *c, void *pe_arrays[],
                              int resource_required_start,
                              int resource_required_end)
{
  int t = 0;
  for (int l = resource_required_start; l < resource_required_end; l++)
  {
    float *curr_pe_addr = static_cast<float *>(pe_arrays[l]);
    float *curr_arr = curr_pe_addr + ((SIZE_PER_PE / 2) / sizeof(float));
    for (int i = 0; i < (FLOATS_PER_PE / 2); i++)
    {
      if (((FLOATS_PER_PE / 2) * t + i) < (MATRIX_SIZE * MATRIX_SIZE))
      {
        c[(FLOATS_PER_PE / 2) * t + i] = curr_arr[i];
      }
    }
    t++;
  }
}

// Helper function to verify the results
void verify_results(float *e, float *c)
{
  for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i)
  {
      assert(c[i] == e[i]);
  }
}

// Helper function to compute the result without dataflow (matrix_a+matrix_b)*matrix_d
void compute_result_without_dataflow(float *a, float *b, float *d, float *e)
{
  for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i)
  {
    e[i] = (a[i] + b[i]) * d[i];
  }
}

// Print resource utilization and verify results
void print_resource_utilization(int resource_required, float *e, float *c)
{
  std::cout << "*******DEVICE SPECS*******\n";
  std::cout << "TOTAL PE ROWS->" << PE_ROWS << "\n";
  std::cout << "TOTAL PE COLUMNS->" << PE_COLUMNS << "\n";
  std::cout << "SIZE PER PE->" << SIZE_PER_PE << " BYTES"
            << "\n";
  std::cout << "Total PE's required for calculation->" << (resource_required)
            << " PE's\n\n";

  for (int i = 0; i < PE_ROWS; i++)
  {
    for (int j = 0; j < PE_COLUMNS; j++)
    {
      if ((i * PE_ROWS + j) < resource_required)
      {
        std::cout << "PE " << i * PE_ROWS + j + 1 << "-> "
                  << "(" << i << "," << j << ")"
                  << " \n";
      }
    }
  }

  verify_results(e, c);

  std::cout << "\n";
  std::cout << "TEST PASSED\n";
  std::cout << "RESULT VERIFIED SUCCESSFULLY\n";
}

// Helper function to free resources
void free_resources(void *pe_arrays[], int resource_required, float *a, float *b, float *c, float *d, float *e)
{
  for (int i = 0; i < resource_required; i++)
  {
    std::free(pe_arrays[i]);
  }

  std::free(a);
  std::free(b);
  std::free(c);
  std::free(d);
  std::free(e);
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
  float *e = static_cast<float *>(
      std::malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));

  initialize_input_data(a, b, c, d, e);
 
  // Computes the result without dataflow
  compute_result_without_dataflow(a, b, d, e);

  int resource_required = calculate_resource(MATRIX_SIZE);

  // Loading the data into the Hardware
  load_data_to_pe_arrays(a, b, pe_arrays, 0, resource_required - 1);

  // Doing computation  matrix_a + matrix_b -> storing the result in the memory location of matrix_b
  pe_add(pe_arrays, resource_required - 1);
  
  // Copying the data from the Hardware
  load_data_from_pe_arrays(c, pe_arrays, 0,resource_required-1);
  
  // Loading the data into the Hardware
  load_data_to_pe_arrays(c, d, pe_arrays, 0,
                         resource_required - 1);

  // Addition_result * matrix_d -> storing the result in the memory location of matrix_b
  pe_mul(pe_arrays, resource_required - 1);

  load_data_from_pe_arrays(c, pe_arrays, 0,
                           resource_required);

  // Printing resource utilization and verify results
  print_resource_utilization(resource_required, e, c);
  
  // Free allocated resources
  free_resources(pe_arrays, resource_required, a, b, c, d, e);

  return 0;
}
