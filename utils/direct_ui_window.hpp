#pragma once

#include "window.hpp"
#include "direct_ui.hpp"

namespace direct_ui
{
	class dui_window : public window
	{
	private:
		inline static scene_factory dui_scene_factory;
		std::shared_ptr<scene> _builtin_scene;
	public:
		const std::shared_ptr<scene>& builtin_scene{ _builtin_scene };
		const std::shared_ptr<scene>& s{ _builtin_scene };

	private:
		virtual INT_PTR WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override final
		{
			switch (message)
			{
			case WM_CREATE:
			{
				_builtin_scene = dui_scene_factory.build_hwnd_scene(hwnd);
				builtin_scene->resize(cwidth(), cheight());
				break;
			}
			case WM_DESTROY:
			{
				_builtin_scene.reset();
				break;
			}
			case WM_PAINT:
			{
				builtin_scene->on_paint();
				ValidateRect(hwnd, nullptr);
				return 0;
			}
			case WM_SIZE:
			{
				HANDLE_WM_SIZE(hwnd, wParam, lParam,
					[this](HWND hwnd, UINT state, int cx, int cy)
					{
						builtin_scene->resize(cx, cy);
					});
				break;
			}
			}
			return DuiWindowProc(hwnd, message, wParam, lParam);
		}
		virtual INT_PTR DuiWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
	};
}