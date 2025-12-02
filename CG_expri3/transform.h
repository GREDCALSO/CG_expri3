#pragma once
#include<Windows.h>
#include<vector>


namespace GraphicFunc
{
	namespace Transform
	{
		//二维点集的矩阵变换
		POINT ApplyMatrix(const POINT& p, const float M[3][3]);

		//基础矩阵生成
		void GetTranslateMatrix(float tx, float ty, float M[3][3]);
		void GetScaleMatrix(float sx, float sy, float M[3][3]);
		void GetRotateMatrix(float angle, float M[3][3]);

		//绕任意点旋转
		void GetRotateAroundPointMatrix(float angle, float cx, float cy, float M[3][3]);

		//应用到多边形
		void TransformShape(std::vector<POINT>& pts, const float M[3][3]);
	}
}