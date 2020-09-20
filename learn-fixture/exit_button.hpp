#pragma once

#include "utils/direct_ui.hpp"

namespace direct_ui
{
	class logic_exit_button : virtual public logic_widget
	{
	private:
		static constexpr real r = 8;
	private:
		bool is_mouse_hover{};
		int is_mouse_down{};
	public:
		std::function<void()> callback{ [] {} };
	public:
		logic_exit_button()
		{
			resize(r * 2, r * 2);
		}
	private:
		static constexpr color color_hover{ 232u, 17u, 35u };
		static constexpr color color_leave{ 255u, 255u, 255u };
		real hover_ratio{};
	public:
		virtual void on_update(std::chrono::steady_clock::duration elapsed) override
		{
			auto sec = std::chrono::duration_cast<std::chrono::duration<real>>(elapsed);
			real dt = sec.count();
			{
				real hover_target = is_mouse_hover;
				constexpr real speed = 5;
				real step = (hover_target ? 1 : -1) * speed * dt;
				hover_ratio += step;
				hover_ratio = std::max(0.f, std::min(1.f, hover_ratio));
				if (std::abs(hover_ratio - hover_target) > 1e-6)
					require_update();
			}
		}
		virtual bool on_hittest(real x, real y) override
		{
			real dx = x - cx / 2;
			real dy = y - cy / 2;
			real dis = dx * dx + dy * dy;
			return dis <= r * r;
		}
		virtual void on_mouse_hover() override
		{
			is_mouse_hover = true;
			require_update();
		}
		virtual void on_mouse_leave() override
		{
			is_mouse_hover = false;
			require_update();
		}
		virtual void on_left_down(real x, real y) override
		{
			is_mouse_down++;
			require_update();
		}
		virtual void on_left_up(real x, real y) override
		{
			is_mouse_down--;
			require_update();
			if (is_mouse_hover)
				callback();
		}

		friend class dep_widget<logic_exit_button>;
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_exit_button> : virtual public logic_exit_button, virtual public dep_widget_base
	{
	public:
		virtual void on_paint() const override
		{
			std::shared_ptr<scene> s = ancestor.lock();
			{
				ID2D1SolidColorBrush* brush;
				pRenderTarget->CreateSolidColorBrush(
					color::linear_interpolation(color_leave, color_hover, hover_ratio),
					&brush);
				pRenderTarget->FillEllipse(
					{ {cx / 2, cy / 2}, r, r },
					brush);
				brush->Release();
			}
		}
	};
	using exit_button = dep_widget<logic_exit_button>;
	static_assert(has_implimented_dep_widget<logic_exit_button>);
#endif
}