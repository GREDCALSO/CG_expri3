#pragma once

#include<Windows.h>
#include"renderer.h"
#include"component_shapesize.h"
#include<d2d1.h>

#define ID_CB_SELECTGRAPHICS 1001
#define ID_BTN_CLEANCANVAS 2001
#define ID_LB_HINTTITLE 3001
#define ID_LB_OPERATIONMODE 3002

extern GraphicMode graphic_modes;

//提示文本，宽200，高20
POINT pos_label_HintTitle = { 1100.0f,30.0f };
ComponentShapeSize size_label_HintTitle = { 120.0f,20.0f };

//选择图形框位置，宽200，高200
POINT pos_combo_SelectGraphics = { 1060.0f,60.0f };
ComponentShapeSize size_combo_SelectGraphics = { 200.0f,200.0f };

//清屏按钮位置，宽100，高30
POINT pos_btn_CleanCanvas = { 1110.0f,100.0f };
ComponentShapeSize size_btn_CleanCanvas = { 100.0f,30.0f };

//操作模式提示文本位置，宽800，高20
POINT pos_label_OperationMode = { 0.0f,0.0f };
ComponentShapeSize size_label_OperationMode = { 1280.0f,16.0f };

HWND label_OperationMode = nullptr;
HWND label_HintTitle = nullptr;
HWND combo_SelectGraphics = nullptr;
HWND btn_CleanCanvas = nullptr;

void CreateUIControls(HWND hwndParent)
{
	//操作模式提示文本
	label_OperationMode = CreateWindowExW(
		0,
		L"STATIC",
		L"操作提示",
		WS_CHILD | WS_VISIBLE | SS_LEFT,
		pos_label_OperationMode.x,
		pos_label_OperationMode.y,
		size_label_OperationMode.width,
		size_label_OperationMode.height,
		hwndParent,
		(HMENU)ID_LB_OPERATIONMODE,
		GetModuleHandleW(nullptr),
		nullptr
	);

	//提示文本
	label_HintTitle = CreateWindowExW(
		0,
		L"STATIC",
		L"选择要绘制的图形",
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		pos_label_HintTitle.x,
		pos_label_HintTitle.y,
		size_label_HintTitle.width,
		size_label_HintTitle.height,
		hwndParent,
		(HMENU)ID_LB_HINTTITLE,
		GetModuleHandleW(nullptr),
		nullptr
	);

	//清屏按钮
	btn_CleanCanvas = CreateWindowExW(
		0,
		L"BUTTON",
		L"清屏",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		pos_btn_CleanCanvas.x,
		pos_btn_CleanCanvas.y,
		size_btn_CleanCanvas.width,
		size_btn_CleanCanvas.height,
		hwndParent,
		(HMENU)ID_BTN_CLEANCANVAS,
		GetModuleHandleW(nullptr),
		nullptr
	);

	//创建下拉框，选择绘制图形
	combo_SelectGraphics = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		L"COMBOBOX",
		L"下拉框选",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
		pos_combo_SelectGraphics.x,
		pos_combo_SelectGraphics.y,
		size_combo_SelectGraphics.width,
		size_combo_SelectGraphics.height,
		hwndParent,
		(HMENU)ID_CB_SELECTGRAPHICS,
		GetModuleHandleW(nullptr),
		nullptr
	);

	//检查下拉框是否创建完成
	if (!combo_SelectGraphics)
	{
		DWORD err = GetLastError();
		wchar_t buf[128];
		wsprintfW(buf, L"CreateWindowExW(COMBOBOX) failed, GetLastError=%lu\n", err);
		OutputDebugStringW(buf);
		return;
	}

	//给下拉框添加选项
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 0, (LPARAM)L"直线（Bresenham算法）");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 1, (LPARAM)L"直线（中点算法）");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 2, (LPARAM)L"圆（Bresenham算法）");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 3, (LPARAM)L"圆（中点算法）");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 4, (LPARAM)L"矩形");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 5, (LPARAM)L"画笔");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 6, (LPARAM)L"B样条曲线");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 7, (LPARAM)L"填充（扫描线法）");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 8, (LPARAM)L"填充（栅栏填充法）");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 9, (LPARAM)L"任意多边形");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 10, (LPARAM)L"曲线（使用D2D库）");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 11, (LPARAM)L"——平移——");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 12, (LPARAM)L"——缩放——");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 13, (LPARAM)L"——旋转——");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 14, (LPARAM)L"CS直线裁切");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 15, (LPARAM)L"中点分割直线裁切");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 16, (LPARAM)L"SH多边形裁切");
	SendMessageW(combo_SelectGraphics, CB_ADDSTRING, 17, (LPARAM)L"WA多边形裁切");

	SendMessageW(combo_SelectGraphics, CB_SETCURSEL, 0, 0); //设置选中项
}