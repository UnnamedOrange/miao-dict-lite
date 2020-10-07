#pragma once

#include "utils/direct_ui.hpp"
#include "unpainted_button.hpp"

namespace direct_ui
{
	class logic_word_pad : virtual public logic_unpainted_button
	{
	public:
		std::u8string word;
	private:
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

		friend class dep_widget<logic_word_pad>;
	};
#if _MSVC_LANG
	template <>
	class dep_widget<logic_word_pad> : virtual public logic_word_pad, virtual public unpainted_button
	{
	public:
		virtual void on_paint() const override
		{
			std::shared_ptr<scene> s = ancestor.lock();
			{
				IDWriteTextFormat* text_format{};
				s->pDWriteFactory->CreateTextFormat(L"Segoe UI", nullptr,
					DWRITE_FONT_WEIGHT_NORMAL,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					28.f,
					L"",
					&text_format);
				text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
				text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

				ID2D1SolidColorBrush* brush_text{};
				pRenderTarget->CreateSolidColorBrush(
					color(0xff000000), &brush_text);

				auto to_draw = code_conv<char8_t, wchar_t>::convert(word);
				s->pRenderTarget->DrawTextW(to_draw.c_str(), to_draw.length(), text_format,
					D2D1::RectF(0, 0, cx, cy), brush_text);

				text_format->Release();
				brush_text->Release();
			}
		}
	};
	using word_pad = dep_widget<logic_word_pad>;
	static_assert(has_implimented_dep_widget<logic_word_pad>);
#endif
}