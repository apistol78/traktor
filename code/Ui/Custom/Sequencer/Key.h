/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_Key_H
#define traktor_ui_custom_Key_H

#include "Core/Object.h"
#include "Ui/Associative.h"

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

class Point;
class Rect;
class Canvas;

		namespace custom
		{

class Sequence;

/*! \brief Sequence key.
 * \ingroup UIC
 */
class T_DLLCLASS Key
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	virtual void move(int offset) = 0;

	virtual void getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const = 0;

	virtual void paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset) = 0;
};

		}
	}
}

#endif	// traktor_ui_custom_Key_H
