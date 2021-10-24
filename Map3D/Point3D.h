#pragma once

class Point3D
{
public:
	float x;
	float y;
	float z;

public:
	Point3D()
	{
		x = 0.0F;
		y = 0.0F;
		z = 0.0F;
	}

	Point3D(float inputX, float inputY, float inputZ)
	{
		x = inputX;
		y = inputY;
		z = inputZ;
	}
};
