#ifndef traktor_ui_custom_SequencerControl_H
#define traktor_ui_custom_SequencerControl_H

#include <list>
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(SequencerControl)

public:
	enum Events
	{
		EiCursorMove = EiUser,
		EiGroupVisible = EiUser + 1
	};

	enum GetSequenceFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfSelectedOnly = 2,
		GfExpandedOnly = 4
	};

	SequencerControl();

	bool create(Widget* parent, int style = WsDoubleBuffer);

	void setSeparator(int separator);

	int getSeparator() const;

	void setLength(int length);

	int getLength() const;

	void setCursor(int time);

	int getCursor() const;

	void addSequenceItem(SequenceItem* sequenceItem);

	void removeSequenceItem(SequenceItem* sequenceItem);

	void removeAllSequenceItems();

	int getSequenceItems(RefList< SequenceItem >& sequenceItems, int flags);

	void addSelectEventHandler(EventHandler* eventHandler);

	void addCursorMoveEventHandler(EventHandler* eventHandler);

	void addGroupVisibleEventHandler(EventHandler* eventHandler);

private:
	struct MouseTrackItem
	{
		Rect rc;
		Ref< SequenceItem > item;
	};

	Ref< ScrollBar > m_scrollBarV;
	Ref< ScrollBar > m_scrollBarH;
	RefList< SequenceItem > m_sequenceItems;
	MouseTrackItem m_mouseTrackItem;
	int m_separator;
	int m_length;
	int m_cursor;
	
	void updateScrollBars();

	void eventSize(Event* e);

	void eventButtonDown(Event* e);

	void eventButtonUp(Event* e);

	void eventMouseMove(Event* e);

	void eventPaint(Event* e);

	void eventScroll(Event* e);
};

		}
	}
}

#endif	// traktor_ui_custom_SequencerControl_H
