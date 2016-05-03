#ifndef traktor_ui_custom_NullPropertyItem_H
#define traktor_ui_custom_NullPropertyItem_H

#include "Ui/Custom/PropertyList/PropertyItem.h"

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

/*! \brief Null property item.
 * \ingroup UIC
 */
class T_DLLCLASS NullPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	NullPropertyItem();

protected:
	virtual void paintValue(Canvas& canvas, const Rect& rc) T_OVERRIDE;
};

		}
	}
}

#endif	// traktor_ui_custom_NullPropertyItem_H
