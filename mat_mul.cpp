#include "device.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#define MATRIX_ROWS_1 3
#define MATRIX_COLUMNS_1 4
#define MATRIX_ROWS_2 4
#define MATRIX_COLUMNS_2 5
#define ONE_BYTE 8

// Helper function to verify the results
void verify_results(float *c, float *d)
{
    for (int i = 0; i < MATRIX_ROWS_1 * MATRIX_COLUMNS_2; ++i)
    {
        assert(d[i] == c[i]);
    }
}

// Helper function to initialize input data
void initialize_input_data(float *a, float *b, float *c, float *d)
{
    for (int i = 0; i < MATRIX_ROWS_1 * MATRIX_COLUMNS_1; ++i)
    {
        a[i] = static_cast<float>(i + 1);
    }

    for (int i = 0; i < MATRIX_ROWS_2 * MATRIX_COLUMNS_2; ++i)
    {
        b[i] = static_cast<float>(i + 1);
    }

    for (int i = 0; i < MATRIX_ROWS_1 * MATRIX_COLUMNS_2; ++i)
    {
        c[i] = 0.0f;
    }

    for (int i = 0; i < MATRIX_ROWS_1 * MATRIX_COLUMNS_2; ++i)
    {
        d[i] = 0.0f;
    }
}

// Helper function to free allocated resources
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

// Helper function to print resource utilization
void print_resource_utilization(int resource_required, float *c, float *d)
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

    verify_results(c, d);

    std::cout << "\n";
    std::cout << "TEST PASSED\n";
    std::cout << "RESULT VERIFIED SUCCESSFULLY\n";
}

// Helper function to calculate the resource required for computation
int calculate_resource() { return MATRIX_ROWS_1 * MATRIX_COLUMNS_2; }

// Helper function to do computation without dataflow
void compute_results_without_dataflow(float *a, float *b, float *d)
{
    for (int i = 0; i < MATRIX_ROWS_1; i++)
    {
        for (int j = 0; j < MATRIX_COLUMNS_2; j++)
        {
            for (int k = 0; k < MATRIX_COLUMNS_1; k++)
            {
                d[i * MATRIX_COLUMNS_2 + j] +=
                    a[i * MATRIX_COLUMNS_1 + k] * b[k * MATRIX_COLUMNS_2 + j];
            }
        }
    }
}

// Helper function to load input data into processing elements
void load_data_to_pe_arrays(float *a, float *b, void *pe_arrays[],
                            int resource_required)
{
    int pe_index = 0;
    // The data is loaded in a way that a row from matrix_a and a column from
    // matrix_b will be in a single PE
    for (int i = 0; i < MATRIX_ROWS_1; i++)
    {
        for (int j = 0; j < MATRIX_COLUMNS_2; j++)
        {
            if (pe_index >= resource_required)
                break;

            float *curr_arr = static_cast<float *>(pe_arrays[pe_index]);

            for (int l = 0; l < MATRIX_COLUMNS_1; l++)
            {
                curr_arr[l] = a[i * MATRIX_COLUMNS_1 + l];
            }

            for (int l = 0; l < MATRIX_ROWS_2; l++)
            {
                curr_arr[MATRIX_COLUMNS_1 + l] = b[l * MATRIX_COLUMNS_2 + j];
            }

            pe_index++;
        }
    }
}

// Helper function to load data from processing elements
void load_data_from_pe_arrays(float *c, void *pe_arrays[], int resource_required, int pe_index)
{
    int t = 0;
    // The data is loaded in a way that a row from matrix_a and a column from
    // matrix_b will be in a single PE
    for (int i = resource_required; i <= resource_required + pe_index; i++)
    {
        float *curr_arr = static_cast<float *>(pe_arrays[i]);
        for (int j = 0; j < (SIZE_PER_PE / (sizeof(float))); j++)
        {
            if ((t * (SIZE_PER_PE / sizeof(float)) + j) < (MATRIX_ROWS_1 * MATRIX_COLUMNS_2))
            {
                c[t * (SIZE_PER_PE / sizeof(float)) + j] = curr_arr[j];
            }
        }
        t++;
    }
}

// Helper function to perform matrix multiplication
int pe_mat_mul(void *pe_arrays[], int resource_required, int no_of_pe_to_store_output)
{
    int pe_index = 0;
    for (int i = 0; i < resource_required; i++)
    {
        float *curr_arr = static_cast<float *>(pe_arrays[i]);

        float temp = 0;
        for (int j = 0; j < MATRIX_COLUMNS_1; j++)
        {
            temp += curr_arr[j] * curr_arr[MATRIX_COLUMNS_1 + j];
        }

        pe_index = i / (SIZE_PER_PE / sizeof(float));
        int local_index = i % (SIZE_PER_PE / sizeof(float));

        if (pe_index < no_of_pe_to_store_output)
        {
            float *result_arr =
                static_cast<float *>(pe_arrays[resource_required + pe_index]);
            result_arr[local_index] = temp;
        }
    }
    return pe_index;
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
        std::malloc(MATRIX_ROWS_1 * MATRIX_COLUMNS_1 * sizeof(float)));
    float *b = static_cast<float *>(
        std::malloc(MATRIX_ROWS_2 * MATRIX_COLUMNS_2 * sizeof(float)));
    float *c = static_cast<float *>(
        std::malloc(MATRIX_ROWS_1 * MATRIX_COLUMNS_2 * sizeof(float)));
    float *d = static_cast<float *>(
        std::malloc(MATRIX_ROWS_1 * MATRIX_COLUMNS_2 * sizeof(float)));

    initialize_input_data(a, b, c, d);

    // matrix multiplication without dataflow of a and b and storing the result in d
    compute_results_without_dataflow(a, b, d);

    int resource_required = calculate_resource();

    load_data_to_pe_arrays(a, b, pe_arrays, resource_required);

    int resource_to_store_output = MATRIX_ROWS_1 * MATRIX_COLUMNS_2;
    int no_of_pe_to_store_output = static_cast<int>(
        std::ceil(static_cast<float>(resource_to_store_output) / (SIZE_PER_PE / sizeof(float))));

    // Computing the matrix multiplication and storing the result in the the new
    // pe locations and copying the result to the c array
    int pe_index = pe_mat_mul(pe_arrays, resource_required, no_of_pe_to_store_output);

    // function to store c the result_arr to c array
    load_data_from_pe_arrays(c, pe_arrays, resource_required, pe_index);

    // Printing the reources and verify results
    print_resource_utilization(resource_required, c, d);

    // Free allocated resources
    free_resources(pe_arrays, resource_required, a, b, c, d);

    return 0;
}
