#ifndef traktor_spark_Sprite_H
#define traktor_spark_Sprite_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Resource/Proxy.h"
#include "Spark/Character.h"
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
	namespace spark
	{

class Context;
class ICharacterBuilder;
class IComponent;
class Shape;
class SpriteData;

/*! \brief Sprite character instance.
 * \ingroup Spark
 */
class T_DLLCLASS Sprite : public Character
{
	T_RTTI_CLASS;

public:
	Sprite(const Context* context, const ICharacterBuilder* builder, const SpriteData* spriteData, const Character* parent);

	const Context* getContext() const;

	void setShape(Shape* shape);

	const resource::Proxy< Shape >& getShape() const;

	Ref< Character > create(const std::wstring& id) const;

	void place(int32_t depth, Character* instance);

	void remove(int32_t depth);

	void setAlpha(float alpha);

	float getAlpha() const;

	void getCharacters(RefArray< Character >& outCharacters) const;

	/*! \brief Set component in character instance.
	 */
	void setComponent(IComponent* component);

	/*! \brief Get component of type.
	 */
	IComponent* getComponent(const TypeInfo& componentType) const;

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

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

	virtual void update() T_OVERRIDE T_FINAL;

	virtual void render(render::RenderContext* renderContext) const T_OVERRIDE T_FINAL;

private:
	friend class SpriteFactory;

	Ref< const Context > m_context;
	Ref< const ICharacterBuilder > m_builder;
	Ref< const SpriteData > m_spriteData;
	resource::Proxy< Shape > m_shape;
	DisplayList m_displayList;
	ColorTransform m_colorTransform;
	SmallMap< const TypeInfo*, Ref< IComponent > > m_components;
	bool m_mousePressed;
	bool m_mouseInside;
};

	}
}

#endif	// traktor_spark_Sprite_H
