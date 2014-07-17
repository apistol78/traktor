#ifndef traktor_ui_custom_FilePropertyItem_H
#define traktor_ui_custom_FilePropertyItem_H

#include "Ui/Custom/PropertyList/PropertyItem.h"
#include "Core/Io/Path.h"

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

class Edit;

		namespace custom
		{

class MiniButton;

/*! \brief File property item.
 * \ingroup UIC
 */
class T_DLLCLASS FilePropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	FilePropertyItem(const std::wstring& text, const Path& path);

	void setPath(const Path& path);

	const Path& getPath() const;

protected:
	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void mouseButtonDown(MouseEvent* event);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	Ref< Edit > m_editor;
	Ref< MiniButton > m_buttonEdit;
	Path m_path;

	void eventEditFocus(FocusEvent* event);

	void eventClick(ButtonClickEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_FilePropertyItem_H
