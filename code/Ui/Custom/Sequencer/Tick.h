/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_Tick_H
#define traktor_ui_custom_Tick_H

#include "Ui/Custom/Sequencer/Key.h"

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
		namespace custom
		{

/*! \brief Sequencer tick.
 * \ingroup UIC
 */
class T_DLLCLASS Tick : public Key
{
	T_RTTI_CLASS;

public:
	Tick(int time, bool movable);

	void setTime(int time);

	int getTime() const;

	virtual void move(int offset) T_OVERRIDE;

	virtual void getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const T_OVERRIDE;

	virtual void paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset) T_OVERRIDE;

private:
	int m_time;
	bool m_movable;
};

		}
	}
}

#endif	// traktor_ui_custom_Tick_H
