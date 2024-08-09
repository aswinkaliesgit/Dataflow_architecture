#include "device.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define MATRIX_SIZE 8
#define ONE_BYTE 8

// Function to calculate the computing units required for each input
int calculate_resource(int size)
{
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
  for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i)
  {
    a[i] = i + 1;
    b[i] = i + 1;
    c[i] = 0.0f;
  }

  int resource_required = calculate_resource(MATRIX_SIZE);

  // Loading the data into the Hardware
  load_data_to_pe_arrays(a, b, pe_arrays, resource_required);

  // Doing computation and taking the result out from the hardware matrix_a + matrix_b -> storing the result in the memory location of matrix_b
  for (int k = 0; k < resource_required; k++)
  {
    float *a_arr = static_cast<float *>(pe_arrays[k]);
    float *b_arr = a_arr + ((SIZE_PER_PE / 2) / sizeof(float));
    for (int i = 0; i < (FLOATS_PER_PE / 2); i++)
    {
      b_arr[i] = a_arr[i] + b_arr[i];
    }
  }

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

  // Printing the output
  std::cout << "*******DEVICE SPECS*******\n";
  std::cout << "TOTAL PE ROWS->" << PE_ROWS << "\n";
  std::cout << "TOTAL PE COLUMNS->" << PE_COLUMNS << "\n";
  std::cout << "SIZE PER PE->" << SIZE_PER_PE << " BYTES" << "\n";
  std::cout << "Total PE's required for calculation->" << (resource_required) << " PE's\n\n";

  for (int i = 0; i < MATRIX_SIZE; ++i)
  {
    for (int j = 0; j < MATRIX_SIZE; ++j)
    {
      std::cout << c[i * MATRIX_SIZE + j] << " ";
    }
    std::cout << "\n";
  }

  for (int i = 0; i < total_pe; i++)
  {
    std::free(pe_arrays[i]);
  }
  std::free(a);
  std::free(b);
  std::free(c);

  return 0;
}
