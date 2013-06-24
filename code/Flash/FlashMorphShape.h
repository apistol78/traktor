#ifndef traktor_flash_FlashMorphShape_H
#define traktor_flash_FlashMorphShape_H

#include "Flash/Path.h"
#include "Flash/SwfTypes.h"
#include "Flash/FlashCharacter.h"
#include "Flash/FlashFillStyle.h"
#include "Flash/FlashLineStyle.h"

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

/*! \brief Flash morph shape.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMorphShape : public FlashCharacter
{
	T_RTTI_CLASS;

public:
	FlashMorphShape();

	FlashMorphShape(uint16_t id);

	bool create(const Aabb2& shapeBounds, const SwfShape* startShape, const SwfShape* endShape, const SwfStyles* startStyles, const SwfStyles* endStyles);

	virtual Ref< FlashCharacterInstance > createInstance(
		ActionContext* context,
		FlashCharacterInstance* parent,
		const std::string& name,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const;

	const Aabb2& getShapeBounds() const { return m_shapeBounds; }

	const std::list< Path >& getPaths() const { return m_paths; }

	const AlignedVector< FlashFillStyle >& getFillStyles() const { return m_fillStyles; }

	const AlignedVector< FlashLineStyle >& getLineStyles() const { return m_lineStyles; }

	virtual void serialize(ISerializer& s);

private:
	Aabb2 m_shapeBounds;
	std::list< Path > m_paths;
	AlignedVector< FlashFillStyle > m_fillStyles;
	AlignedVector< FlashLineStyle > m_lineStyles;
};

	}
}

#endif	// traktor_flash_FlashMorphShape_H
