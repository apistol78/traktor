#ifndef traktor_spark_SpriteInstance_H
#define traktor_spark_SpriteInstance_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Resource/Proxy.h"
#include "Spark/CharacterInstance.h"
#include "Spark/ColorTransform.h"
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

	namespace sound
	{

class ISoundPlayer;

	}

	namespace spark
	{

class IComponentInstance;
class Shape;
class Sprite;

/*! \brief Sprite character instance.
 * \ingroup Spark
 */
class T_DLLCLASS SpriteInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	SpriteInstance(const Sprite* sprite, const CharacterInstance* parent, resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer);

	Ref< CharacterInstance > create(const std::wstring& id) const;

	void place(int32_t depth, CharacterInstance* instance);

	void remove(int32_t depth);

	void setAlpha(float alpha);

	float getAlpha() const;

	void getCharacters(RefArray< CharacterInstance >& outCharacters) const;

	/*! \brief Set component in character instance.
	 */
	void setComponent(const TypeInfo& componentType, IComponentInstance* component);

	/*! \brief Get component of type.
	 */
	IComponentInstance* getComponent(const TypeInfo& componentType) const;

	/*! \name Events */
	//@{

	virtual void eventKey(wchar_t unicode) T_FINAL;

	virtual void eventKeyDown(int32_t keyCode) T_FINAL;

	virtual void eventKeyUp(int32_t keyCode) T_FINAL;

	virtual void eventMouseDown(const Vector2& position, int32_t button) T_FINAL;

	virtual void eventMouseUp(const Vector2& position, int32_t button) T_FINAL;

	virtual void eventMouseMove(const Vector2& position, int32_t button) T_FINAL;

	virtual void eventMouseWheel(const Vector2& position, int32_t delta) T_FINAL;

	virtual void eventViewResize(int32_t width, int32_t height) T_FINAL;

	//@}

	virtual Aabb2 getBounds() const;

	virtual void update() T_FINAL;

	virtual void render(render::RenderContext* renderContext) const T_FINAL;

private:
	friend class Sprite;

	Ref< const Sprite > m_sprite;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< sound::ISoundPlayer > m_soundPlayer;
	resource::Proxy< Shape > m_shape;
	DisplayList m_displayList;
	ColorTransform m_colorTransform;
	SmallMap< const TypeInfo*, Ref< IComponentInstance > > m_components;
	bool m_mousePressed;
	bool m_mouseInside;
};

	}
}

#endif	// traktor_spark_SpriteInstance_H
