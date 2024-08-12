#include <iostream>
#include <iomanip>

#define ROWS_A 3  // Number of rows in matrix A
#define COLS_A 4  // Number of columns in matrix A (should match the number of rows in matrix B)
#define ROWS_B 4  // Number of rows in matrix B (should match the number of columns in matrix A)
#define COLS_B 5  // Number of columns in matrix B

void matrix_multiply(const float (&A)[ROWS_A][COLS_A], const float (&B)[ROWS_B][COLS_B], float (&C)[ROWS_A][COLS_B])
{
    for (int i = 0; i < ROWS_A; ++i)
    {
        for (int j = 0; j < COLS_B; ++j)
        {
            C[i][j] = 0.0f;
            for (int k = 0; k < COLS_A; ++k)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main()
{
    // Initialize matrices A and B
    float A[ROWS_A][COLS_A];
    float B[ROWS_B][COLS_B];
    float C[ROWS_A][COLS_B] = {0}; // Result matrix

    // Fill matrix A with values i * COLS_A + j
    for (int i = 0; i < ROWS_A; ++i)
    {
        for (int j = 0; j < COLS_A; ++j)
        {
            A[i][j] = static_cast<float>(i * COLS_A + j + 1);
        }
    }

    // Fill matrix B with values i * COLS_B + j
    for (int i = 0; i < ROWS_B; ++i)
    {
        for (int j = 0; j < COLS_B; ++j)
        {
            B[i][j] = static_cast<float>(i * COLS_B + j + 1);
        }
    }

    // Perform matrix multiplication
    matrix_multiply(A, B, C);

    // Print the result
    std::cout << "\nResult matrix C (A * B):" << std::endl;
    for (int i = 0; i < ROWS_A; ++i)
    {
        for (int j = 0; j < COLS_B; ++j)
        {
            std::cout  << C[i][j] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
