#ifndef traktor_spark_CharacterInstance_H
#define traktor_spark_CharacterInstance_H

#include "Core/Object.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderContext;

	}

	namespace spark
	{

/*! \brief Character instance base class.
 * \ingroup Spark
 */
class T_DLLCLASS CharacterInstance : public Object
{
	T_RTTI_CLASS;

public:
	CharacterInstance(const CharacterInstance* parent);

	/*! \brief
	 */
	void setName(const std::wstring& name);

	/*! \brief
	 */
	const std::wstring& getName() const;

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

	/*! \brief Get bounds.
	 */
	virtual Aabb2 getBounds() const = 0;

	/*! \brief Update character instance.
	 */
	virtual void update() = 0;

	/*! \brief Render character instance.
	 */
	virtual void render(render::RenderContext* renderContext) const = 0;

private:
	const CharacterInstance* m_parent;
	std::wstring m_name;
	Matrix33 m_transform;
};

	}
}

#endif	// traktor_spark_CharacterInstance_H
