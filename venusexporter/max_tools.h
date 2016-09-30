#pragma once

#include "BaseInc.h"

namespace max_tools
{
	namespace math
	{
		// convert
		inline float3 cv_color3(const Color & color)
		{
			return float3(color.r, color.g, color.b);
		}
		inline float3 cv_point3(const Point3 & point)
		{
			return float3(point.x, point.z, point.y);
		}

		inline float3 cv_uvvert(const UVVert & uv)
		{
			return float3(uv.x, 1.0f - uv.y, uv.z);
		}

		inline void TransformPoint3(Point3 & point)
		{
			swap(point.y, point.z);
		}

		inline void TransformUV(UVVert & uv)
		{
			uv.y = 1.0f - uv.y;
		}

		inline void TransformQuat(Quat & quat)
		{
			swap(quat.y, quat.z);
			quat.x = -quat.x;
			quat.y = -quat.y;
			quat.z = -quat.z;
			quat.w = -quat.w;
		}

		inline void TransformMatrix3(const Matrix3 & matrix, float3 & pos, quat4 & rot, float3 & scl)
		{
			AffineParts ap;
			decomp_affine(matrix, &ap);
			TransformPoint3(ap.t);
			TransformPoint3(ap.k);
			TransformQuat(ap.q);
			pos.set(ap.t.x, ap.t.y, ap.t.z);
			rot.set(ap.q.x, ap.q.y, ap.q.z, ap.q.w);
			scl.set(ap.k.x, ap.k.y, ap.k.z);
		}

		inline float4x4 Matrix3ToMatrix4F(const Matrix3 & matrix3)
		{
			AffineParts ap;
			decomp_affine(matrix3, &ap);
			float_32 TRANS[4][3] =
			{
				{1, 0, 0},
				{0, 0, 1},
				{0, 1, 0},
				{0, 0, 0},
			};
			Matrix3 matrTrans(TRANS);
			const Matrix3 matrix = matrTrans * matrix3 * matrTrans;
			return float4x4(matrix[0].x, matrix[0].y, matrix[0].z, 0,
				matrix[1].x, matrix[1].y, matrix[1].z, 0,
				matrix[2].x, matrix[2].y, matrix[2].z, 0,
				matrix[3].x, matrix[3].y, matrix[3].z, 1);
		}

		inline float3 cv_point3(const Color & color)
		{
			return float3(color.r, color.g, color.b);
		}
	}
}