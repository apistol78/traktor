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
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class ISoundPlayer;

	}

	namespace spark
	{

/*! \brief Sprite instance factory.
 * \ingroup Spark
 */
class T_DLLCLASS SpriteFactory : public ICharacterFactory
{
	T_RTTI_CLASS;

public:
	SpriteFactory(resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer, bool createComponents);

	virtual TypeInfoSet getCharacterTypes() const;

	virtual Ref< CharacterInstance > create(const ICharacterBuilder* builder, const Character* character, const CharacterInstance* parent, const std::wstring& name) const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< sound::ISoundPlayer > m_soundPlayer;
	bool m_createComponents;
};

	}
}

#endif	// traktor_spark_SpriteFactory_H
