#ifndef traktor_spark_SpriteInstance_H
#define traktor_spark_SpriteInstance_H

#include "Core/RefArray.h"
#include "Resource/Proxy.h"
#include "Spark/CharacterInstance.h"
#include "Spark/DisplayList.h"

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

	namespace spark
	{

class Shape;
class Sprite;

/*! \brief Sprite character instance.
 * \ingroup Spark
 */
class T_DLLCLASS SpriteInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	SpriteInstance(const Sprite* sprite, const CharacterInstance* parent, resource::IResourceManager* resourceManager);

	Ref< CharacterInstance > create(const std::wstring& id) const;

	void place(int32_t depth, CharacterInstance* instance);

	void remove(int32_t depth);

	void getCharacters(RefArray< CharacterInstance >& outCharacters) const;

	virtual Aabb2 getBounds() const;

	virtual void update() T_FINAL;

	virtual void render(render::RenderContext* renderContext) const T_FINAL;

private:
	friend class Sprite;

	Ref< const Sprite > m_sprite;
	Ref< resource::IResourceManager > m_resourceManager;
	resource::Proxy< Shape > m_shape;
	DisplayList m_displayList;
};

	}
}

#endif	// traktor_spark_SpriteInstance_H
