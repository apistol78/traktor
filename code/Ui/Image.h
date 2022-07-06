#pragma once

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

class IBitmap;

/*! Static image.
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
		WsNearestFilter = (WsUser << 3)
	};

	bool create(Widget* parent, IBitmap* image = nullptr, int style = WsNone);

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

	bool setImage(IBitmap* image, bool transparent = false);

	IBitmap* getImage() const;

	bool isTransparent() const;

	bool scaling() const;

private:
	Ref< IBitmap > m_image;
	bool m_transparent = false;
	bool m_scale = false;
	bool m_keepAspect = false;
	bool m_nearest = false;

	void eventPaint(PaintEvent* event);
};

	}
}

