#include <Windows.h>
#undef min
#undef max
#include <windowsx.h>
#include <utils/window.hpp>

class flash_window : public window
{
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
		caption(L"flash");
		return TRUE;
	}
	void OnDestroy(HWND hwnd)
	{
		PostQuitMessage(0);
	}

public:
	flash_window()
	{

	}
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int)
{
	flash_window main_window;
	main_window.create();
	return main_window.message_loop();
}