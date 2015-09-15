#ifndef traktor_spark_CharacterInstance_H
#define traktor_spark_CharacterInstance_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Matrix33.h"

namespace traktor
{
	namespace render
	{

class RenderContext;

	}

	namespace spark
	{

class IComponentInstance;

/*! \brief Character instance base class.
 * \ingroup Spark
 */
class CharacterInstance : public Object
{
	T_RTTI_CLASS;

public:
	CharacterInstance(const CharacterInstance* parent);

	/*! \brief Set local transform.
	 */
	void setTransform(const Matrix33& transform);

	/*! \brief Get local transform.
	 */
	const Matrix33& getTransform() const;

	/*! \brief Get full transform.
	 */
	Matrix33 getFullTransform() const;

	/*! \brief Set local position.
	 */
	void setPosition(const Vector2& position);

	/*! \brief Get local position.
	 */
	Vector2 getPosition() const;

	/*! \brief Set local scale.
	 */
	void setScale(const Vector2& scale);

	/*! \brief Get local scale.
	 */
	Vector2 getScale() const;

	/*! \brief Set local rotation.
	 */
	void setRotation(float rotation);

	/*! \brief Get local rotation.
	 */
	float getRotation() const;

	/*! \brief Add component to character instance.
	 */
	void addComponent(IComponentInstance* component);

	/*! \brief Get active components.
	 */
	const RefArray< IComponentInstance >& getComponents() const;

	/*! \brief Update character instance.
	 */
	virtual void update();

	/*! \brief Render character instance.
	 */
	virtual void render(render::RenderContext* renderContext) const = 0;

private:
	const CharacterInstance* m_parent;
	Matrix33 m_transform;
	RefArray< IComponentInstance > m_components;
};

	}
}

#endif	// traktor_spark_CharacterInstance_H
