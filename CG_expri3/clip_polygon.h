#pragma once
#include<vector>
#include<Windows.h>
#include"component_shape.h"

namespace GraphicFunc
{
	namespace Clip
	{
		// Sutherland–Hodgman：对单个任意多边形（subject）裁剪到矩形 [left_top, right_bottom]
				// 返回裁剪后的顶点（若为空，表示裁剪为空）
		std::vector<POINT> SutherlandHodgmanClip(const std::vector<POINT>& subject,
												 const POINT& left_top,
												 const POINT& right_bottom);

		// Weiler–Atherton：对单个任意多边形（subject）裁剪到矩形 [left_top, right_bottom]
		// 返回一个向量：可能包含多个闭合多边形（当裁剪产生若干不相连区域时）
		std::vector<std::vector<POINT>> WeilerAthertonClip(const std::vector<POINT>& subject,
														   const POINT& left_top,
														   const POINT& right_bottom);
	}
}