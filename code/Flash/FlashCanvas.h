#ifndef traktor_flash_FlashCanvas_H
#define traktor_flash_FlashCanvas_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/Path.h"
#include "Flash/SwfTypes.h"
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

	void clear();

	void beginFill(const FlashFillStyle& fillStyle);

	void endFill();

	void moveTo(avm_number_t x, avm_number_t y);

	void lineTo(avm_number_t x, avm_number_t y);

	void curveTo(avm_number_t controlX, avm_number_t controlY, avm_number_t anchorX, avm_number_t anchorY);

	const Aabb2& getBounds() const { return m_bounds; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< FlashLineStyle >& getLineStyles() const { return m_lineStyles; }

	const AlignedVector< FlashFillStyle >& getFillStyles() const { return m_fillStyles; }

private:
	int32_t m_cacheTag;
	int32_t m_dirtyTag;
	Aabb2 m_bounds;
	AlignedVector< Path > m_paths;
	AlignedVector< FlashLineStyle > m_lineStyles;
	AlignedVector< FlashFillStyle > m_fillStyles;
	bool m_drawing;
};

	}
}

#endif	// traktor_flash_FlashCanvas_H
