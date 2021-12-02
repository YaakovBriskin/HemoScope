#pragma once

#include <cuda.h>
#include <cuda_runtime.h>
#include <exception>
#include <stdexcept>
#include <sstream>

#define checkCuda(status) checkCudaImpl(status, __FILE__, __LINE__)

// Each result that returned by CUDA functions must be checked
void checkCudaImpl(cudaError_t status, const char* filename, const int line);

// Used to calculate number of blocks as quotient of division data size by block size
int divideCeil(int value, int divisor);
