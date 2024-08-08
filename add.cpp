#include "device.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define MATRIX_SIZE 71
#define ONE_BYTE 8

// Function to calculate the computing units required for each input
std::vector<int> calculateResource(int total_layers, int size_a, int size_b)
{
    std::vector<int> node_vec;

    float float_size_a = static_cast<float>(size_a);
    float float_size_b = static_cast<float>(size_b);
    float result_a = std::ceil(float_size_a / (SIZE_PER_PE / sizeof(float)));
    float result_b = std::ceil(float_size_b / (SIZE_PER_PE / sizeof(float)));
    int int_result_a = static_cast<int>(result_a);
    int int_result_b = static_cast<int>(result_b);
    node_vec.push_back(int_result_a);
    node_vec.push_back(2 * int_result_b);
    return node_vec;
}

// Helper function to load input data into processing elements
void loadDataToPeArrays(float *a, float *b, void *pe_arrays[],
                        const std::vector<int> &resource_required)
{
    for (int k = 0; k < resource_required.size(); k++)
    {
        for (int l = 0; l < resource_required[0]; l++)
        {
            float *curr_arr =
                static_cast<float *>(pe_arrays[k * resource_required[0] + l]);
            for (int i = 0; i < (SIZE_PER_PE / sizeof(float)); i++)
            {
                if (k == 0)
                {
                    curr_arr[i] = a[(SIZE_PER_PE / sizeof(float)) * l + i];
                }
                else if (k == 1)
                {
                    curr_arr[i] = b[(SIZE_PER_PE / sizeof(float)) * l + i];
                }
            }
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

    std::vector<int> resource_required = calculateResource(
        3, MATRIX_SIZE * MATRIX_SIZE, MATRIX_SIZE * MATRIX_SIZE);
    
    // Loading the data into the Hardware
    loadDataToPeArrays(a, b, pe_arrays, resource_required);

    // Doing computation and taking the result out from the hardware
    for (int k = 0; k < resource_required[0]; k++)
    {
        float *a_arr = static_cast<float *>(pe_arrays[k]);
        float *b_arr = static_cast<float *>(pe_arrays[k + resource_required[0]]);
        for (int i = 0; i < (SIZE_PER_PE / sizeof(float)); i++)
        {
            b_arr[i] = a_arr[i] + b_arr[i];
        }
    }

    for (int l = 0; l < resource_required[0]; l++)
    {
        float *curr_arr = static_cast<float *>(pe_arrays[resource_required[0] + l]);
        for (int i = 0; i < (SIZE_PER_PE / sizeof(float)); i++)
        {
            if (((SIZE_PER_PE / sizeof(float)) * l + i) <
                (MATRIX_SIZE * MATRIX_SIZE))
            {
                c[(SIZE_PER_PE / sizeof(float)) * l + i] = curr_arr[i];
            }
        }
    }

    // Printing the output
    std::cout<<"*******DEVICE SPECS*******\n";
    std::cout<<"TOTAL PE ROWS->"<<PE_ROWS<<"\n";
    std::cout<<"TOTAL PE COLUMNS->"<<PE_COLUMNS<<"\n";
    std::cout<<"SIZE PER PE->"<<SIZE_PER_PE<<" BYTES"<<"\n";
    std::cout<<"Total PE's required for calculation->"<<(resource_required[resource_required.size()-1])<<" PE's\n\n";

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
