#include "Spark/ExternalFactory.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteFactory.h"
#include "Spark/SpriteInstance.h"
#include "Spark/TextFactory.h"
#include "Spark/Editor/CharacterAdapter.h"
#include "Spark/Editor/CharacterAdapterBuilder.h"
#include "Spark/Editor/Context.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Context", Context, Object)

Context::Context(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

bool Context::setSprite(Sprite* sprite)
{
	m_root = 0;
	m_adapters.clear();

	CharacterAdapterBuilder characterBuilder(m_root, m_adapters);
	characterBuilder.addFactory(new ExternalFactory(m_resourceManager));
	characterBuilder.addFactory(new SpriteFactory(m_resourceManager, 0, false));
	characterBuilder.addFactory(new TextFactory(m_resourceManager));

	if (characterBuilder.create(sprite, 0, L"<< Root >>") == 0)
		return false;

	return true;
}

CharacterAdapter* Context::hitTest(const Vector2& position) const
{
	for (RefArray< CharacterAdapter >::const_iterator i = m_adapters.begin(); i != m_adapters.end(); ++i)
	{
		// Don't hit test against root character.
		if (!(*i)->getParent())
			continue;

		// Don't hit test against external characters.
		if ((*i)->isChildOfExternal())
			continue;

		Vector2 localPosition = (*i)->getCharacterInstance()->getTransform().inverse() * position;

		if ((*i)->getCharacterInstance()->getBounds().inside(localPosition))
			return (*i);
	}
	return 0;
}

CharacterAdapter* Context::getRoot()
{
	return m_root;
}

const RefArray< CharacterAdapter >& Context::getAdapters() const
{
	return m_adapters;
}

	}
}
