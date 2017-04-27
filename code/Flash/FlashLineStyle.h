/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashLineStyle_H
#define traktor_flash_FlashLineStyle_H

#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix33.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace flash
	{

class ColorTransform;

/*! \brief Shape line style.
 * \ingroup Flash
 */
class T_DLLCLASS FlashLineStyle
{
public:
	FlashLineStyle();

	bool create(const SwfLineStyle* lineStyle);

	void transform(const ColorTransform& cxform);

	const Color4f& getLineColor() const;

	uint16_t getLineWidth() const;

	void serialize(ISerializer& s);

private:
	Color4f m_lineColor;
	uint16_t m_lineWidth;
};

	}
}

#endif	// traktor_flash_FlashLineStyle_H
