#ifndef traktor_ui_custom_CheckPropertyItem_H
#define traktor_ui_custom_CheckPropertyItem_H

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

/*! \brief Check property item.
 * \ingroup UIC
 */
class T_DLLCLASS CheckPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	CheckPropertyItem(const std::wstring& text, bool checked);

	void setChecked(bool checked);
	
	bool isChecked() const;

protected:
	virtual void mouseButtonDown(MouseButtonDownEvent* event) T_OVERRIDE;

	virtual void paintValue(Canvas& canvas, const Rect& rc) T_OVERRIDE;

private:
	bool m_checked;
};

		}
	}
}

#endif	// traktor_ui_custom_CheckPropertyItem_H
