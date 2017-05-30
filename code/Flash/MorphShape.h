/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_MorphShape_H
#define traktor_flash_MorphShape_H

#include "Flash/Path.h"
#include "Flash/SwfTypes.h"
#include "Flash/Character.h"
#include "Flash/FillStyle.h"
#include "Flash/LineStyle.h"

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
class T_DLLCLASS MorphShape : public Character
{
	T_RTTI_CLASS;

public:
	MorphShape();

	MorphShape(uint16_t id);

	bool create(const Aabb2& shapeBounds, const SwfShape* startShape, const SwfShape* endShape, const SwfStyles* startStyles, const SwfStyles* endStyles);

	virtual Ref< CharacterInstance > createInstance(
		ActionContext* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const T_OVERRIDE T_FINAL;

	const Aabb2& getShapeBounds() const { return m_shapeBounds; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< FillStyle >& getFillStyles() const { return m_fillStyles; }

	const AlignedVector< LineStyle >& getLineStyles() const { return m_lineStyles; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Aabb2 m_shapeBounds;
	AlignedVector< Path > m_paths;
	AlignedVector< FillStyle > m_fillStyles;
	AlignedVector< LineStyle > m_lineStyles;
};

	}
}

#endif	// traktor_flash_MorphShape_H
