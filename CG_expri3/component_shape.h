#pragma once
#include<vector>
#include<Windows.h>

enum class ShapeType
{
	Line,
	Polygon
};

struct Shape
{
	ShapeType type;
	std::vector<POINT> vertices;
};