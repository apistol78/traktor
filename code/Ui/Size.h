/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Size_H
#define traktor_ui_Size_H

namespace traktor
{
	namespace ui
	{
	
/*! \brief Size
 * \ingroup UI
 */
class Size
{
public:
	int cx;
	int cy;

	inline Size();
	
	inline Size(int x, int y);
	
	inline Size(const Size& size);

	inline Size operator - () const;
	
	inline Size operator + (const Size& r) const;
	
	inline Size& operator += (const Size& r);
	
	inline Size operator - (const Size& r) const;
	
	inline Size& operator -= (const Size& r);
};
	
	}
}

#include "Ui/Size.inl"

#endif	// traktor_ui_Size_H
