#ifndef traktor_ui_custom_SequencerControl_H
#define traktor_ui_custom_SequencerControl_H

#include <list>
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

class ScrollBar;

		namespace custom
		{

class SequenceItem;

/*! \brief Sequencer control.
 * \ingroup UIC
 */
class T_DLLCLASS SequencerControl : public Widget
{
	T_RTTI_CLASS;

public:
	enum GetSequenceFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfSelectedOnly = 2,
		GfExpandedOnly = 4
	};

	SequencerControl();

	bool create(Widget* parent, int style = WsDoubleBuffer);

	void setSeparator(int32_t separator);

	int32_t getSeparator() const;

	void setTimeScale(int32_t timeScale);

	int32_t getTimeScale() const;

	void setLength(int32_t length);

	int32_t getLength() const;

	void setCursor(int32_t time);

	int32_t getCursor() const;

	void addSequenceItem(SequenceItem* sequenceItem);

	void addSequenceItemBefore(SequenceItem* beforeItem, SequenceItem* sequenceItem);

	void removeSequenceItem(SequenceItem* sequenceItem);

	void removeAllSequenceItems();

	int getSequenceItems(RefArray< SequenceItem >& sequenceItems, int flags);

private:
	struct MouseTrackItem
	{
		Rect rc;
		Ref< SequenceItem > item;
	};

	Ref< ScrollBar > m_scrollBarV;
	Ref< ScrollBar > m_scrollBarH;
	RefArray< SequenceItem > m_sequenceItems;
	MouseTrackItem m_mouseTrackItem;
	int32_t m_separator;
	int32_t m_timeScale;
	int32_t m_length;
	int32_t m_cursor;
	Point m_startPosition;
	int32_t m_moveTrack;
	int32_t m_dropIndex;
	
	void updateScrollBars();

	void eventSize(SizeEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventScroll(ScrollEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_SequencerControl_H
