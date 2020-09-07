#include <Windows.h>
#undef min
#undef max
#include <windowsx.h>
#include "utils/window.hpp"

class fixture_window : public window
{
	static constexpr int cx = 500;
	virtual INT_PTR WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override
	{
		switch (message)
		{
			HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);

		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
		return 0;
	}
	BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
	{
		caption(L"fixture");
		SetWindowLongW(hwnd, GWL_STYLE,
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP | WS_SYSMENU);
		SetWindowLongW(hwnd, GWL_EXSTYLE, WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, NULL, 127, LWA_ALPHA);
		width(300);
		height(200);

		return TRUE;
	}
	void OnDestroy(HWND hwnd)
	{
		PostQuitMessage(0);
	}

public:
	fixture_window()
	{

	}
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	fixture_window main_window;
	main_window.create();
	return main_window.message_loop();
}