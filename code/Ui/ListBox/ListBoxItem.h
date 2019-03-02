#pragma once

#include "Ui/Auto/AutoWidgetCell.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS ListBoxItem : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	ListBoxItem();

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	bool setSelected(bool selected);

	bool isSelected() const;

	virtual void paint(Canvas& canvas, const Rect& rect) override final;

private:
	std::wstring m_text;
	bool m_selected;
};

	}
}

