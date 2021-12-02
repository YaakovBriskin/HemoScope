#include "UtilsCUDA.h"

// Each result that returned by CUDA functions must be checked
void checkCudaImpl(cudaError_t status, const char* filename, const int line)
{
	if (status != cudaSuccess)
	{
		std::stringstream errorMessage;
		errorMessage << "CUDA ERROR: in file \"" << filename << "\" line " << line << ": " << cudaGetErrorString(status);
		throw std::runtime_error(errorMessage.str());
	}
}

// Used to calculate number of blocks as quotient of division data size by block size
int divideCeil(int value, int divisor)
{
	return (value + divisor - 1) / divisor;
}
