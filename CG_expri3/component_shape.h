#pragma once
#include<vector>
#include<Windows.h>

/*-----------component_shape.h-----------*/

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