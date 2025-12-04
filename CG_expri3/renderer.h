#pragma once
#include <windows.h>
#include <d2d1.h>
#include "color.h"
#pragma comment(lib, "d2d1.lib")

class Renderer
{
public:
	Renderer() : pFactory(nullptr), pRenderTarget(nullptr), pBrush(nullptr) {}

	ID2D1HwndRenderTarget* GetRenderTarget() const { return pRenderTarget; }
	ID2D1Factory* GetFactory() const { return pFactory; }
	ID2D1SolidColorBrush* GetBrush() const { return pBrush; }

	void Initialize(HWND hwnd)
	{
		D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory);
		RECT rc;
		GetClientRect(hwnd, &rc);
		pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
			&pRenderTarget
		);
		pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBrush);
		pRenderTarget->BeginDraw();
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
		pRenderTarget->EndDraw();
	}

	void Cleanup()
	{
		if (pBrush) pBrush->Release();
		if (pRenderTarget) pRenderTarget->Release();
		if (pFactory) pFactory->Release();
	}

	//（现已不用在图形绘制算法里）单点绘制，begin->end就绘制一个点，非常慢
	void SetPixel(int x, int y, Color color)
	{
		if (!pRenderTarget) return;
		pBrush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
		pRenderTarget->BeginDraw();
		pRenderTarget->FillRectangle(
			D2D1::RectF((float)x, (float)y, (float)x + 1, (float)y + 1), pBrush);
		pRenderTarget->EndDraw();
	}

	void SetMark(int x, int y, D2D1::ColorF COLOR = D2D1::ColorF::LightGray)
	{
		if (!pRenderTarget) return;
		if (!inDrawing) return;
		pBrush->SetColor(D2D1::ColorF(COLOR));
		pRenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 1.4f, 1.4f), pBrush);
	}

	//批量绘制：开始一批绘制
	void Begin()
	{
		if (!pRenderTarget || inDrawing) return;
		pRenderTarget->BeginDraw();
		inDrawing = true;
	}

	//批量绘制：结束一批绘制
	void End()
	{
		if (!pRenderTarget || !inDrawing) return;
		pRenderTarget->EndDraw();
		inDrawing = false;
	}

	bool IsDrawing() const { return inDrawing; }

	//批量绘制下的像素设置：不做 Begin/EndDraw
	void PlotPixel(int x, int y, Color color)
	{
		if (!pRenderTarget) return;
		if (!inDrawing) return; //需要外部已调用 Begin()
		pBrush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
		pRenderTarget->FillRectangle(D2D1::RectF((float)x, (float)y, (float)x + 1, (float)y + 1), pBrush);
	}

	void Present()
	{
		if (!pRenderTarget) return;
		//pRenderTarget->BeginDraw();
		//pRenderTarget->EndDraw();
	}

	void Clear(D2D1::ColorF color = D2D1::ColorF(D2D1::ColorF::White))
	{
		if (!pRenderTarget) return;
		pRenderTarget->BeginDraw();
		pRenderTarget->Clear(color);
		pRenderTarget->EndDraw();
	}

	// RAII 辅助：作用域内自动 Begin/End
	class DrawScope
	{
	public:
		explicit DrawScope(Renderer& r) : rr(r), active(false)
		{
			if (!rr.IsDrawing()) { rr.Begin(); active = true; }
		}
		~DrawScope() { if (active) rr.End(); }
	private:
		Renderer& rr;
		bool active;
	};

private:
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pBrush;
	bool inDrawing;
};
