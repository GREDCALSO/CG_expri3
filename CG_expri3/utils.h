#pragma once
#include<Windows.h>
#include<vector>
#include<cmath>
#include"component_shape.h"
#include"renderer.h"

//检查鼠标点击位置是否选中图形
bool PointInsideShape(POINT mousePos, Shape seletedShape);

void AddShape_line(std::vector<Shape>& gShape, POINT start, POINT end);
void AddShape_polygon(std::vector<Shape>& gShape, const std::vector<POINT>& points);

//计算图形中心点
POINT GetShapeCenter(const Shape& shape);