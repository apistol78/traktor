#ifndef traktor_animation_Joint_H
#define traktor_animation_Joint_H

#include <string>
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief Skeleton bone.
 * \ingroup Animation
 */
class T_DLLCLASS Joint : public ISerializable
{
	T_RTTI_CLASS;

public:
	Joint();

	/*! \brief Set parent joint index.
	 *
	 * \param parent New parent index.
	 */
	void setParent(int32_t parent) { m_parent = parent; }

	/*! \brief Get parent joint index.
	 *
	 * \return Parent index.
	 */
	int32_t getParent() const { return m_parent; }

	/*! \brief Set joint name.
	 *
	 * \param name New joint name.
	 */
	void setName(const std::wstring& name) { m_name = name; }

	/*! \brief Get joint name.
	 *
	 * \return Name of joint.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! \brief Set joint transform.
	 *
	 * Joint transform is relative to parent space.
	 *
	 * \param transform New joint transform.
	 */
	void setTransform(const Transform& transform) { m_transform = transform; }

	/*! \brief Get joint transform.
	 *
	 * Joint transform is relative to parent space.
	 *
	 * \return Joint transform.
	 */
	const Transform& getTransform() const { return m_transform; }

	/*! \brief Set joint radius.
	 *
	 * \param radius New joint radius.
	 */
	void setRadius(float radius) { m_radius = radius; }

	/*! \brief Get joint radius.
	 *
	 * \return Joint radius.
	 */
	const float getRadius() const { return m_radius; }

	/*! \brief Set joint limits enabled.
	 */
	void setEnableLimits(bool enableLimits) { m_enableLimits = enableLimits; }

	/*! \brief Check if joint limits is enabled.
	 */
	bool getEnableLimits() const { return m_enableLimits; }

	/*! \brief Set amount of twist allowed.
	 */
	void setTwistLimit(float twistLimit) { m_twistLimit = twistLimit; }

	/*! \brief Get twist limit.
	 */
	float getTwistLimit() const { return m_twistLimit; }

	/*! \brief Set cone angle limit.
	 */
	void setConeLimit(const Vector2& coneLimit) { m_coneLimit = coneLimit; }

	/*! \brief Get cone angle limit.
	 */
	const Vector2& getConeLimit() const { return m_coneLimit; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_parent;
	std::wstring m_name;
	Transform m_transform;
	float m_radius;
	bool m_enableLimits;
	float m_twistLimit;
	Vector2 m_coneLimit;
};

	}
}

#endif	// traktor_animation_Joint_H
