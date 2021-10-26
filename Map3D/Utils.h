#pragma once

typedef unsigned char byte;

const size_t PIXELS_IN_MM = 2600;

size_t mm2pixels(float mm)
{
	return (size_t)std::roundf(PIXELS_IN_MM * mm);
}

float pixels2mm(size_t pixels)
{
	return (float)pixels / PIXELS_IN_MM;
}

const byte WHITE = 255;

// Size of the kernel to find corners
const size_t CORNER_DETECTION_KERNEL_SIZE = 3;

// Kernels to process image - used also to skip unwanted pixels on seams
const size_t FINE_SMOOTHING_KERNEL_SIZE = 5;
const size_t DEEP_SMOOTHING_KERNEL_SIZE = 51;

// Examine gray level to find possible capillary corner - performed on raw image
bool isValidGrayLevelOriginal(byte val)
{
	const byte glMin = 50;
	const byte glMax = 90;
	return (glMin <= val) && (val <= glMax);
}

// Examine gray level to find limit of capillary - performed on processed image
bool isValidGrayLevelProcessed(byte val)
{
	const byte glMin = 0;
	const byte glMax = 120;
	return (glMin <= val) && (val <= glMax);
}

const size_t MIN_FOUND_CAPILLARIES = 3;
const size_t DESCRIBED_CAPILLARIES = 10;
const size_t MIN_PIXELS_IN_CAPILLARY = 20;
