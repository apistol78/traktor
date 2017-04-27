/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_SpriteFactory_H
#define traktor_spark_SpriteFactory_H

#include "Spark/ICharacterFactory.h"

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

/*! \brief Sprite instance factory.
 * \ingroup Spark
 */
class T_DLLCLASS SpriteFactory : public ICharacterFactory
{
	T_RTTI_CLASS;

public:
	SpriteFactory(bool createComponents);

	virtual TypeInfoSet getCharacterTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< Character > create(const Context* context, const ICharacterBuilder* builder, const CharacterData* characterData, const Character* parent, const std::wstring& name) const T_OVERRIDE T_FINAL;

private:
	bool m_createComponents;
};

	}
}

#endif	// traktor_spark_SpriteFactory_H
