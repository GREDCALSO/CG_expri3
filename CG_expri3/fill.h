#pragma once
#include"renderer.h"
#include"color.h"
#include<vector>
#include<algorithm>
#include<cmath>

namespace GraphicFunc
{
	namespace Fill
	{
		struct ScanlineEdge
		{
			int yMax;        // 该边参与扫描的最高y（包含），采用“上端不计”的约定后这里就是上端-1
			float x;         // 当前扫描线上的交点x
			float invSlope;  // 斜率：1/m = dx/dy
		};

		void FillPolygonScanline(const std::vector<POINT>& vertices, Renderer& renderer, Color fillColor = Color(0.1f, 0.5f, 0.9f, 1.0f));
		void FillRandomShapeScanline(std::vector<POINT>& poly, Renderer& renderer);
		void FillPolygonFence(const std::vector<POINT>& vertices, Renderer& renderer, Color fillColor = Color(0.9f, 0.3f, 0.3f, 1.0f));
		void FillRandomShapeFence(std::vector<POINT>& poly, Renderer& renderer);
	}
}