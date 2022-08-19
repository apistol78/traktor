#pragma once

#include "Ui/Container.h"

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

class ChildEvent;
class IBitmap;

/*! Dialog
 * \ingroup UI
 */
class T_DLLCLASS Dialog : public Container
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsCenterParent = WsUser,
		WsCenterDesktop = (WsUser << 1),
		WsDefaultFixed = WsSystemBox | WsMinimizeBox | WsCloseBox | WsCaption,
		WsDefaultResizable = WsResizable | WsSystemBox | WsMinimizeBox | WsMaximizeBox | WsCloseBox | WsCaption
	};

	bool create(Widget* parent, const std::wstring& text, int width, int height, int style = WsDefaultResizable, Layout* layout = 0);

	void setIcon(IBitmap* icon);

	virtual DialogResult showModal();

	virtual void endModal(DialogResult result);

	bool isModal() const;

	virtual bool acceptLayout() const override;

private:
	bool m_modal = false;

	void eventChild(ChildEvent* event);
};

	}
}

