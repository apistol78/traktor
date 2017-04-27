/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_SequenceGroup_H
#define traktor_ui_custom_SequenceGroup_H

#include "Ui/Custom/Sequencer/SequenceItem.h"

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

class IBitmap;

		namespace custom
		{

/*! \brief Sequence group.
 * \ingroup UIC
 */
class T_DLLCLASS SequenceGroup : public SequenceItem
{
	T_RTTI_CLASS;

public:
	SequenceGroup(const std::wstring& name);

	void expand();

	void collapse();

	bool isExpanded() const;

	bool isCollapsed() const;

	void setVisible(bool visible);

	bool isVisible() const;

	void setRange(int start, int end);

	int getStart() const;

	int getEnd() const;

	virtual void mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) T_OVERRIDE;

	virtual void mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) T_OVERRIDE;

	virtual void mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) T_OVERRIDE;

	virtual void paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset) T_OVERRIDE;

private:
	Ref< IBitmap > m_imageExpand;
	Ref< IBitmap > m_imageCollapse;
	Ref< IBitmap > m_imageVisible;
	Ref< IBitmap > m_imageHidden;
	bool m_expanded;
	bool m_visible;
	int m_start;
	int m_end;
};

		}
	}
}

#endif	// traktor_ui_custom_SequenceGroup_H
