#pragma once

#include "Ui/PropertyList/PropertyItem.h"

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

/*! \brief Null property item.
 * \ingroup UI
 */
class T_DLLCLASS NullPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	NullPropertyItem();

protected:
	virtual void paintValue(Canvas& canvas, const Rect& rc) override;
};

	}
}

