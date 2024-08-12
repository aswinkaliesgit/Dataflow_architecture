#include <iostream>
#include <vector>
#include <cmath>

#define MATRIX_SIZE 8 // Define the matrix size here

int main() {
    // Define and initialize matrices a, b, c, and result
    std::vector<std::vector<float>> a(MATRIX_SIZE, std::vector<float>(MATRIX_SIZE));
    std::vector<std::vector<float>> b(MATRIX_SIZE, std::vector<float>(MATRIX_SIZE));
    std::vector<std::vector<float>> c(MATRIX_SIZE, std::vector<float>(MATRIX_SIZE));
    std::vector<std::vector<float>> result(MATRIX_SIZE, std::vector<float>(MATRIX_SIZE, 0.0f));

    // Initialize matrices a, b, and c with some values
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            a[i][j] = i*MATRIX_SIZE + j + 1;
            b[i][j] = i*MATRIX_SIZE + j + 1;
            c[i][j] = 2.0f;
        }
    }

    // Calculate result = (a + b) * c
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
                result[i][j]  = a[i][j] + b[i][j];
                result[i][j]  = result[i][j]*c[i][j];
        }
    }

    // Print the result matrix
    std::cout << "Resultant Matrix (a + b * c):" << std::endl;
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            std::cout << result[i][j] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
