#ifndef traktor_ui_custom_GridCell_H
#define traktor_ui_custom_GridCell_H

#include "Ui/Custom/Auto/AutoWidgetCell.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class T_DLLCLASS GridCell : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	virtual int32_t getHeight() const = 0;

	virtual std::wstring getText() const = 0;
};

		}
	}
}

#endif	// traktor_ui_custom_GridCell_H
