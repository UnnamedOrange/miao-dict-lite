#pragma once

#include "utils/direct_ui.hpp"
#include "unpainted_button.hpp"

namespace direct_ui
{
	class logic_exit_button : virtual public logic_unpainted_button
	{
	private:
		static constexpr real r = 8;
	public:
		logic_exit_button()
		{
			resize(r * 2, r * 2);
		}
	private:
		static constexpr color color_hover{ 232u, 17u, 35u };
		static constexpr color color_leave{ 255u, 255u, 255u };
		static constexpr color color_down{ 255u, 255u, 255u };
		real hover_ratio{};
		real down_ratio{};
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
			{
				real down_target = is_mouse_down;
				constexpr real speed = 5;
				real step = (down_target ? 1 : -1) * speed * dt;
				down_ratio += step;
				down_ratio = std::max(0.f, std::min(1.f, down_ratio));
				if (std::abs(down_ratio - down_target) > 1e-6)
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

		friend class dep_widget<logic_exit_button>;
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_exit_button> : virtual public logic_exit_button, virtual public unpainted_button
	{
	public:
		virtual void on_paint() const override
		{
			std::shared_ptr<scene> s = ancestor.lock();

			auto bg_color = color::linear_interpolation(color_leave, color_hover, hover_ratio);
			{
				ID2D1SolidColorBrush* brush;
				pRenderTarget->CreateSolidColorBrush(bg_color, &brush);
				pRenderTarget->FillEllipse(
					{ {cx / 2, cy / 2}, r, r },
					brush);
				brush->Release();
			}
			{
				IDWriteTextFormat* text_format{};
				s->pDWriteFactory->CreateTextFormat(L"Segoe MDL2 Assets", nullptr,
					DWRITE_FONT_WEIGHT_NORMAL,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					cx / 2,
					L"",
					&text_format);
				text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
				text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

				ID2D1SolidColorBrush* brush_text{};
				pRenderTarget->CreateSolidColorBrush(
					color::linear_interpolation(bg_color, color_down, down_ratio),
					&brush_text);

				wchar_t icon = 0xef2c;
				s->pRenderTarget->DrawTextW(&icon, 1, text_format,
					D2D1::RectF(0, 0, cx, cy), brush_text);

				text_format->Release();
				brush_text->Release();
			}
		}
	};
	using exit_button = dep_widget<logic_exit_button>;
	static_assert(has_implimented_dep_widget<logic_exit_button>);
#endif
}