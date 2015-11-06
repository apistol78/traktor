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

	void setParent(int32_t parent) { m_parent = parent; }

	int32_t getParent() const { return m_parent; }

	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

	void setTransform(const Transform& transform) { m_transform = transform; }

	const Transform& getTransform() const { return m_transform; }

	void setRadius(float radius) { m_radius = radius; }

	const float getRadius() const { return m_radius; }

	void setEnableLimits(bool enableLimits) { m_enableLimits = enableLimits; }

	bool getEnableLimits() const { return m_enableLimits; }

	void setTwistLimit(float twistLimit) { m_twistLimit = twistLimit; }

	float getTwistLimit() const { return m_twistLimit; }

	void setConeLimit(const Vector2& coneLimit) { m_coneLimit = coneLimit; }

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
