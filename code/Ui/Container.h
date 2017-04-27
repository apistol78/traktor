/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Container_H
#define traktor_ui_Container_H

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
class SizeEvent;

/*! \brief Layout container.
 * \ingroup UI
 */
class T_DLLCLASS Container : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style = WsNone, Layout* layout = 0);

	virtual void fit();

	virtual void update(const Rect* rc = 0, bool immediate = false) T_OVERRIDE;
	
	virtual Size getMinimumSize() const T_OVERRIDE;
	
	virtual Size getPreferedSize() const T_OVERRIDE;
	
	virtual Size getMaximumSize() const T_OVERRIDE;

	Ref< Layout > getLayout() const;
	
	void setLayout(Layout* layout);
	
private:
	Ref< Layout > m_layout;
	
	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};
	
	}
}

#endif	// traktor_ui_Container_H
