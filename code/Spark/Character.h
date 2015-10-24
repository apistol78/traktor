#ifndef traktor_spark_Character_H
#define traktor_spark_Character_H

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
class T_DLLCLASS Character : public Object
{
	T_RTTI_CLASS;

public:
	Character(const Character* parent);

	/*! \brief
	 */
	const Character* getParent() const;

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

	/*! \brief Set visibility.
	 */
	void setVisible(bool visible);

	/*! \brief Get visibility.
	 */
	bool getVisible() const;

	/*! \name Events */
	//@{

	virtual void eventKey(wchar_t unicode);

	virtual void eventKeyDown(int32_t keyCode);

	virtual void eventKeyUp(int32_t keyCode);

	virtual void eventMouseDown(const Vector2& position, int32_t button);

	virtual void eventMouseUp(const Vector2& position, int32_t button);

	virtual void eventMouseMove(const Vector2& position, int32_t button);

	virtual void eventMouseWheel(const Vector2& position, int32_t delta);

	virtual void eventViewResize(int32_t width, int32_t height);

	//@}

	/*! \brief Get bounds.
	 */
	virtual Aabb2 getBounds() const = 0;

	/*! \brief Update character instance.
	 */
	virtual void update() = 0;

	/*! \brief Render character instance.
	 */
	virtual void render(render::RenderContext* renderContext) const = 0;

protected:
	const Character* m_parent;
	std::wstring m_name;
	Matrix33 m_transform;
	bool m_visible;
};

	}
}

#endif	// traktor_spark_Character_H
