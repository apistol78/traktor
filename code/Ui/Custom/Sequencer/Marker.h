/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_Marker_H
#define traktor_ui_custom_Marker_H

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

/*! \brief Sequencer marker.
 * \ingroup UIC
 */
class T_DLLCLASS Marker : public Key
{
	T_RTTI_CLASS;

public:
	Marker(int32_t time, bool movable);

	void setTime(int32_t time);

	int32_t getTime() const;

	virtual void move(int offset) T_OVERRIDE;

	virtual void getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const T_OVERRIDE;

	virtual void paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset) T_OVERRIDE;

private:
	int32_t m_time;
	bool m_movable;
};

		}
	}
}

#endif	// traktor_ui_custom_Marker_H
