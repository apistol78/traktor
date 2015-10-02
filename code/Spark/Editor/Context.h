#ifndef traktor_spark_Context_H
#define traktor_spark_Context_H

#include "Core/Math/Vector2.h"
#include "Ui/EventSubject.h"

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class CharacterAdapter;
class Sprite;

class Context : public ui::EventSubject
{
	T_RTTI_CLASS;

public:
	Context(resource::IResourceManager* resourceManager);

	bool setSprite(Sprite* sprite);

	CharacterAdapter* hitTest(const Vector2& position) const;

	CharacterAdapter* getRoot();

	const RefArray< CharacterAdapter >& getAdapters() const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< CharacterAdapter > m_root;
	RefArray< CharacterAdapter > m_adapters;
};

	}
}

#endif	// traktor_spark_Context_H
