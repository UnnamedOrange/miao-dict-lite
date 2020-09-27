#pragma once

#include "utils/direct_ui.hpp"

namespace direct_ui
{
	class logic_icon_button : virtual public logic_widget
	{
	private:
		bool is_mouse_hover{};
		int is_mouse_down{};
	public:
		std::function<void()> callback{ [] {} };
	public:
		logic_icon_button()
		{
			resize(16.f, 16.f);
		}
	private:
		real hover_ratio{};
	public:
		wchar_t icon{};
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

		friend class dep_widget<logic_icon_button>;
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_icon_button> : virtual public logic_icon_button, virtual public dep_widget_base
	{
	public:
		virtual void on_paint() const override
		{
			std::shared_ptr<scene> s = ancestor.lock();
			{
				IDWriteTextFormat* text_format{};
				s->pDWriteFactory->CreateTextFormat(L"Segoe MDL2 Assets", nullptr,
					DWRITE_FONT_WEIGHT_NORMAL,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					cx,
					L"",
					&text_format);
				text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
				text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

				ID2D1SolidColorBrush* brush_text{};
				pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x000000), &brush_text);

				s->pRenderTarget->DrawTextW(&icon, 1, text_format,
					D2D1::RectF(0, 0, cx, cy), brush_text);

				text_format->Release();
				brush_text->Release();
			}
		}
	};
	using icon_button = dep_widget<logic_icon_button>;
	static_assert(has_implimented_dep_widget<logic_icon_button>);
#endif
}