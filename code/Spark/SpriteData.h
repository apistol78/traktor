/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_SpriteData_H
#define traktor_spark_SpriteData_H

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb2.h"
#include "Resource/Id.h"
#include "Spark/CharacterData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class IComponentData;
class Shape;

/*! \brief SpriteData character.
 * \ingroup Spark
 */
class T_DLLCLASS SpriteData : public CharacterData
{
	T_RTTI_CLASS;

public:
	struct NamedCharacter
	{
		std::wstring name;
		Ref< CharacterData > character;
	};

	const Aabb2& getBounds() const { return m_bounds; }

	const CharacterData* getCharacter(const std::wstring& id) const;

	void place(const std::wstring& name, CharacterData* character);

	void remove(CharacterData* character);

	const AlignedVector< NamedCharacter >& getDictionary() { return m_dictionary; }

	const AlignedVector< NamedCharacter >& getFrame() const { return m_frame; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class CharacterPipeline;
	friend class SpriteFactory;

	RefArray< IComponentData > m_components;
	Aabb2 m_bounds;
	resource::Id< Shape > m_shape;
	AlignedVector< NamedCharacter > m_dictionary;
	AlignedVector< NamedCharacter > m_frame;
};

	}
}

#endif	// traktor_spark_SpriteData_H
