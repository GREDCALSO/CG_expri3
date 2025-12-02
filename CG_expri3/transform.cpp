#include"transform.h"

namespace GraphicFunc
{
	namespace Transform
	{
		POINT ApplyMatrix(const POINT& p, const float M[3][3])
		{
			POINT r;
			r.x = M[0][0] * p.x + M[0][1] * p.y + M[0][2] * 1;
			r.y = M[1][0] * p.x + M[1][1] * p.y + M[1][2] * 1;
			return r;
		}

		void GetTranslateMatrix(float tx, float ty, float M[3][3])
		{
			float t[3][3] = {
				1,0,tx,
				0,1,ty,
				0,0,1
			};
			memcpy(M, t, sizeof(t));
		}

		void GetScaleMatrix(float sx, float sy, float M[3][3])
		{
			float t[3][3] = {
				sx,0,0,
				0,sy,0,
				0,0,1
			};
			memcpy(M, t, sizeof(t));
		}

		void GetRotateMatrix(float angle, float M[3][3])
		{
			float c = cosf(angle);
			float s = sinf(angle);
			float t[3][3] = {
				c,-s,0,
				s, c,0,
				0, 0,1
			};
			memcpy(M, t, sizeof(t));
		}

		void GetRotateAroundPointMatrix(float angle, float cx, float cy, float M[3][3])
		{
			float T1[3][3], R[3][3], T2[3][3];

			GetTranslateMatrix(-cx, -cy, T1);
			GetRotateMatrix(angle, R);
			GetTranslateMatrix(cx, cy, T2);

			// M = T2 * R * T1
			float temp[3][3];
			float result[3][3];

			// temp = R * T1
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					temp[i][j] = R[i][0] * T1[0][j] + R[i][1] * T1[1][j] + R[i][2] * T1[2][j];

			// result = T2 * temp
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					result[i][j] = T2[i][0] * temp[0][j] + T2[i][1] * temp[1][j] + T2[i][2] * temp[2][j];

			memcpy(M, result, sizeof(result));
		}

		void TransformShape(std::vector<POINT>& pts, const float M[3][3])
		{
			for (auto& p : pts)
				p = ApplyMatrix(p, M);
		}
	}
}