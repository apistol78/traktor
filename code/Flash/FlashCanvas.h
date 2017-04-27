/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashCanvas_H
#define traktor_flash_FlashCanvas_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/Path.h"
#include "Flash/SwfTypes.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashLineStyle.h"
#include "Flash/FlashFillStyle.h"
#include "Flash/Action/ActionTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashBitmap;

/*! \brief Dynamic canvas.
 * \ingroup Flash
 */
class T_DLLCLASS FlashCanvas : public Object
{
	T_RTTI_CLASS;

public:
	FlashCanvas();

	int32_t getCacheTag() const;

	int32_t getDirtyTag() const;

	/*! \brief Clear canvas. */
	void clear();

	/*! \brief Begin solid fill. */
	void beginFill(const Color4f& color);

	/*! \brief Begin gradient fill. */
	void beginGradientFill(FlashFillStyle::GradientType gradientType, const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords, const Matrix33& gradientMatrix);

	/*! \brief Begin bitmap fill. */
	void beginBitmapFill(FlashBitmap* image, const Matrix33& bitmapMatrix, bool repeat);

	/*! \brief End fill. */
	void endFill();

	/*! \brief Move cursor. */
	void moveTo(float x, float y);

	/*! \brief Add a line from cursor. */
	void lineTo(float x, float y);

	/*! \brief Add a curve from cursor. */
	void curveTo(float controlX, float controlY, float anchorX, float anchorY);

	/*! \brief Get bounds of shapes in canvas. */
	const Aabb2& getBounds() const { return m_bounds; }

	const FlashDictionary& getDictionary() const { return m_dictionary; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< FlashLineStyle >& getLineStyles() const { return m_lineStyles; }

	const AlignedVector< FlashFillStyle >& getFillStyles() const { return m_fillStyles; }

private:
	int32_t m_cacheTag;
	int32_t m_dirtyTag;
	Aabb2 m_bounds;
	FlashDictionary m_dictionary;
	AlignedVector< Path > m_paths;
	AlignedVector< FlashLineStyle > m_lineStyles;
	AlignedVector< FlashFillStyle > m_fillStyles;
	bool m_drawing;
};

	}
}

#endif	// traktor_flash_FlashCanvas_H
