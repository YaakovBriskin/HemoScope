#pragma once

#include "Utils.h"
#include "Point3D.h"

const size_t VERTEX_NUM = 8;

struct ValuesOnRectPrism
{
	byte val000;
	byte val001;
	byte val010;
	byte val100;
	byte val011;
	byte val101;
	byte val110;
	byte val111;
};

class Interpolation3D
{
public:

	/// <summary>
	/// This function is implemented according to trilinear interpolation algorithm:
	/// https://en.wikipedia.org/wiki/Trilinear_interpolation
	/// </summary>
	/// <param name="pt000">point on the lower grid bound</param>
	/// <param name="pt111">point on the upper grid bound</param>
	/// <param name="pt">intermediate point within the cube</param>
	/// <param name="vals">values in 8 vertices of the cube around intermediate point</param>
	/// <returns>weighted average of values in 8 vertices - depends on point position</returns>
	byte getInterpolated(Point3D pt000, Point3D pt111, Point3D pt, ValuesOnRectPrism vals)
	{
		float x0 = pt000.x;
		float y0 = pt000.y;
		float z0 = pt000.z;
		float x1 = pt111.x;
		float y1 = pt111.y;
		float z1 = pt111.z;

		float v000 = (float)vals.val000;
		float v001 = (float)vals.val001;
		float v010 = (float)vals.val010;
		float v100 = (float)vals.val100;
		float v011 = (float)vals.val011;
		float v101 = (float)vals.val101;
		float v110 = (float)vals.val110;
		float v111 = (float)vals.val111;

		float c[VERTEX_NUM];
		c[0] = -v000 * x1 * y1 * z1 + v001 * x1 * y1 * z0 + v010 * x1 * y0 * z1 + v100 * x0 * y1 * z1 - v011 * x1 * y0 * z0 - v101 * x0 * y1 * z0 - v110 * x0 * y0 * z1 + v111 * x0 * y0 * z0;
		c[1] = v000 * y1 * z1 - v001 * y1 * z0 - v010 * y0 * z1 - v100 * y1 * z1 + v011 * y0 * z0 + v101 * y1 * z0 + v110 * y0 * z1 - v111 * y0 * z0;
		c[2] = v000 * x1 * z1 - v001 * x1 * z0 - v010 * x1 * z1 - v100 * x0 * z1 + v011 * x1 * z0 + v101 * x0 * z0 + v110 * x0 * z1 - v111 * x0 * z0;
		c[3] = v000 * x1 * y1 - v001 * x1 * y1 - v010 * x1 * y0 - v100 * x0 * y1 + v011 * x1 * y0 + v101 * x0 * y1 + v110 * x0 * y0 - v111 * x0 * y0;
		c[4] = -v000 * z1 + v001 * z0 + v010 * z1 + v100 * z1 - v011 * z0 - v101 * z0 - v110 * z1 + v111 * z0;
		c[5] = -v000 * y1 + v001 * y1 + v010 * y0 + v100 * y1 - v011 * y0 - v101 * y1 - v110 * y0 + v111 * y0;
		c[6] = -v000 * x1 + v001 * x1 + v010 * x1 + v100 * x0 - v011 * x1 - v101 * x0 - v110 * x0 + v111 * x0;
		c[7] = v000 - v001 - v010 - v100 + v011 + v101 + v110 - v111;

		float norm = (x0 - x1) * (y0 - y1) * (z0 - z1);
		for (int i = 0; i < VERTEX_NUM; i++)
		{
			c[i] /= norm;
		}

		float result = c[0] +
					   c[1] * pt.x + c[2] * pt.y + c[3] * pt.z +
					   c[4] * pt.x * pt.y + c[5] * pt.x * pt.z + c[6] * pt.y * pt.z +
					   c[7] * pt.x * pt.y * pt.z;

		return (byte)result; // round result before casting for integer types T
	}
};
