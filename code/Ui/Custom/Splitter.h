/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_Splitter_H
#define traktor_ui_custom_Splitter_H

#include "Ui/Widget.h"

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

/*! \brief Horizontal or vertical splitter.
 * \ingroup UIC
 */
class T_DLLCLASS Splitter : public Widget
{
	T_RTTI_CLASS;

public:
	/*! \brief Create splitter control.
	 *
	 * \param parent Parent widget.
	 * \param vertical Orientation of splitter.
	 * \param position Initial position of splitter.
	 * \param relative If position is scaled relatively when Splitter is resized.
	 * \param border Clamping border, distance from extents in pixels.
	 */
	bool create(Widget* parent, bool vertical = true, int position = 100, bool relative = false, int border = 16);

	virtual void update(const Rect* rc = 0, bool immediate = false) T_OVERRIDE;
	
	virtual Size getMinimumSize() const T_OVERRIDE;
	
	virtual Size getPreferedSize() const T_OVERRIDE;
	
	virtual Size getMaximumSize() const T_OVERRIDE;
	
	void setPosition(int position);
	
	int getPosition() const;
	
	Ref< Widget > getLeftWidget() const;
	
	Ref< Widget > getRightWidget() const;
	
private:
	bool m_vertical;
	int m_position;
	bool m_negative;
	bool m_relative;
	int m_border;
	bool m_drag;

	int getAbsolutePosition() const;

	void setAbsolutePosition(int position);

	void eventMouseMove(MouseMoveEvent* event);
	
	void eventButtonDown(MouseButtonDownEvent* event);
	
	void eventButtonUp(MouseButtonUpEvent* event);
	
	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_Splitter_H
