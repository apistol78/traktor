#include "Spark/ExternalFactory.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteData.h"
#include "Spark/SpriteFactory.h"
#include "Spark/TextFactory.h"
#include "Spark/Editor/CharacterAdapter.h"
#include "Spark/Editor/CharacterAdapterBuilder.h"
#include "Spark/Editor/EditContext.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.EditContext", EditContext, Object)

EditContext::EditContext(Context* context)
:	m_context(context)
,	m_gridSpacing(40)
{
}

bool EditContext::setSprite(SpriteData* sprite)
{
	m_root = 0;
	m_adapters.clear();

	Ref< CharacterAdapterBuilder > characterBuilder = new CharacterAdapterBuilder(m_root, m_adapters);
	characterBuilder->addFactory(new ExternalFactory());
	characterBuilder->addFactory(new SpriteFactory(false));
	characterBuilder->addFactory(new TextFactory());

	if (characterBuilder->create(m_context, sprite, 0, L"<< Root >>") == 0)
		return false;

	return true;
}

CharacterAdapter* EditContext::hitTest(const Vector2& position) const
{
	for (RefArray< CharacterAdapter >::const_iterator i = m_adapters.begin(); i != m_adapters.end(); ++i)
	{
		// Don't hit test against root character.
		if (!(*i)->getParent())
			continue;

		// Don't hit test against external characters.
		if ((*i)->isChildOfExternal())
			continue;

		Vector2 localPosition = (*i)->getCharacter()->getTransform().inverse() * position;

		if ((*i)->getCharacter()->getBounds().inside(localPosition))
			return (*i);
	}
	return 0;
}

CharacterAdapter* EditContext::getRoot()
{
	return m_root;
}

const RefArray< CharacterAdapter >& EditContext::getAdapters() const
{
	return m_adapters;
}

RefArray< CharacterAdapter > EditContext::getSelectedAdapters() const
{
	RefArray< CharacterAdapter > selectedAdapters;
	for (RefArray< CharacterAdapter >::const_iterator i = m_adapters.begin(); i != m_adapters.end(); ++i)
	{
		if ((*i)->isSelected())
			selectedAdapters.push_back(*i);
	}
	return selectedAdapters;
}

void EditContext::setGridSpacing(int32_t gridSpacing)
{
	m_gridSpacing = gridSpacing;
}

int32_t EditContext::getGridSpacing() const
{
	return m_gridSpacing;
}

Context* EditContext::getContext() const
{
	return m_context;
}

	}
}
