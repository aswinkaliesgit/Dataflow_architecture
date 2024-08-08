#include "Device.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define MATRIX_SIZE 32
#define ONE_BYTE 8
//Function to calculate the computing units required for each input
std::vector<int> CalculateResource(int totalLayers, int sizeA, int sizeB)
{
  std::vector<int> nodeVec;

  float floatSizeA = static_cast<float>(sizeA);
  float floatSizeB = static_cast<float>(sizeB);
  float resultA = std::ceil(floatSizeA / (SIZE_PER_PE / sizeof(float)));
  float resultB = std::ceil(floatSizeB / (SIZE_PER_PE / sizeof(float)));
  int intResultA = static_cast<int>(resultA);
  int intResultB = static_cast<int>(resultB);
  nodeVec.push_back(intResultA);
  nodeVec.push_back(2 * intResultB);
  nodeVec.push_back(3 * intResultA);
  return nodeVec;
}
//Helper function to load input data into processing elements
void LoadDataToPeArrays(float *A, float *B, void *peArrays[],
                        const std::vector<int> &resourceRequired)
{
  for (int k = 0; k < resourceRequired.size(); k++)
  {
    for (int l = 0; l < resourceRequired[0]; l++)
    {
      float *currArr =
          static_cast<float *>(peArrays[k * resourceRequired[0] + l]);
      for (int i = 0; i < (SIZE_PER_PE / sizeof(float)); i++)
      {
        if (k == 0)
        {
          currArr[i] = A[(SIZE_PER_PE / sizeof(float)) * l + i];
        }
        else if (k == 1)
        {
          currArr[i] = B[(SIZE_PER_PE / sizeof(float)) * l + i];
        }
      }
    }
  }
}

int main()
{
  //Allocating memory to the Hardware and slicing it to make many PE's
  const int totalPe = (PE_ROWS * PE_COLUMNS);

  void *peArrays[totalPe];
  for (int i = 0; i < totalPe; i++)
  {
    peArrays[i] = std::malloc(SIZE_PER_PE * ONE_BYTE);
  }

  float *A = static_cast<float *>(
      std::malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
  float *B = static_cast<float *>(
      std::malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
  float *C = static_cast<float *>(
      std::malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
  for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i)
  {
    A[i] = i + 1;
    B[i] = i + 1;
    C[i] = 0.0f;
  }

  std::vector<int> resourceRequired = CalculateResource(
      3, MATRIX_SIZE * MATRIX_SIZE, MATRIX_SIZE * MATRIX_SIZE);
  //Loading the data into the Hardware
  LoadDataToPeArrays(A, B, peArrays, resourceRequired);
 //Doing computation and taking the result out from the hardware
  for (int k = 0; k < resourceRequired[0]; k++)
  {
    float *aArr = static_cast<float *>(peArrays[k]);
    float *bArr = static_cast<float *>(peArrays[k + resourceRequired[0]]);
    for (int i = 0; i < (SIZE_PER_PE / sizeof(float)); i++)
    {
      bArr[i] = aArr[i] + bArr[i];
    }
  }

  for (int l = 0; l < resourceRequired[0]; l++)
  {
    float *currArr = static_cast<float *>(peArrays[resourceRequired[0] + l]);
    for (int i = 0; i < (SIZE_PER_PE / sizeof(float)); i++)
    {
      if (((SIZE_PER_PE / sizeof(float)) * l + i) <
          (MATRIX_SIZE * MATRIX_SIZE))
      {
        C[(SIZE_PER_PE / sizeof(float)) * l + i] = currArr[i];
      }
    }
  }
 //Printing the output
  for (int i = 0; i < MATRIX_SIZE; ++i)
  {
    for (int j = 0; j < MATRIX_SIZE; ++j)
    {
      std::cout << C[i * MATRIX_SIZE + j] << " ";
    }
    std::cout << "\n";
  }

  for (int i = 0; i < totalPe; i++)
  {
    std::free(peArrays[i]);
  }
  std::free(A);
  std::free(B);
  std::free(C);

  return 0;
}
