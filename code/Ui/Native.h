#ifndef traktor_ui_Native_H
#define traktor_ui_Native_H

#include "Ui/Widget.h"

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

class Layout;

/*! \brief Native peer handle.
 * \ingroup UI
 *
 * A native widget is a proxy widget to use when embedding
 * our own custom widgets inside applications using
 * another toolkit.
 * As long as the native window handle is accessible
 * any other window can be used as a parent to our
 * own widgets.
 */
class T_DLLCLASS Native : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(void* nativeHandle, Layout* layout = 0);

	virtual void update(const Rect* rc = 0, bool immediate = false) T_OVERRIDE;

private:
	Ref< Layout > m_layout;

	void eventSize(SizeEvent* event);
};

	}
}

#endif	// traktor_ui_Native_H
