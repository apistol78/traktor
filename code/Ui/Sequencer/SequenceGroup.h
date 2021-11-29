#pragma once

#include "Ui/Sequencer/SequenceItem.h"

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

class IBitmap;

/*! Sequence group.
 * \ingroup UI
 */
class T_DLLCLASS SequenceGroup : public SequenceItem
{
	T_RTTI_CLASS;

public:
	explicit SequenceGroup(const std::wstring& name);

	void expand();

	void collapse();

	bool isExpanded() const;

	bool isCollapsed() const;

	void setRange(int32_t start, int32_t end);

	int32_t getStart() const;

	int32_t getEnd() const;

	virtual void mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) override final;

	virtual void mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) override final;

	virtual void mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) override final;

	virtual void paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset) override final;

private:
	Ref< IBitmap > m_imageExpand;
	Ref< IBitmap > m_imageCollapse;
	bool m_expanded;
	int32_t m_start;
	int32_t m_end;
};

	}
}

