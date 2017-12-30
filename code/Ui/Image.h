/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Image_H
#define traktor_ui_Image_H

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

class Bitmap;

/*! \brief Static image.
 * \ingroup UI
 */
class T_DLLCLASS Image : public Widget
{
	T_RTTI_CLASS;

public:
	enum Styles
	{
		WsTransparent = WsUser,
		WsScale = (WsUser << 1),
		WsScaleKeepAspect = (WsUser << 2),
	};

	Image();

	bool create(Widget* parent, Bitmap* image = 0, int style = WsNone);
	
	virtual Size getMinimumSize() const T_OVERRIDE;
	
	virtual Size getPreferedSize() const T_OVERRIDE;
	
	virtual Size getMaximumSize() const T_OVERRIDE;

	bool setImage(Bitmap* image, bool transparent = false);

	Ref< Bitmap > getImage() const;

	bool isTransparent() const;

	bool scaling() const;

private:
	Ref< Bitmap > m_image;
	bool m_transparent;
	bool m_scale;
	bool m_keepAspect;

	void eventPaint(PaintEvent* event);
};

	}
}

#endif	// traktor_ui_Image_H
