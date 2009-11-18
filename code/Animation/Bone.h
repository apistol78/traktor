#ifndef traktor_animation_Bone_H
#define traktor_animation_Bone_H

#include <string>
#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Quaternion.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief Skeleton bone.
 * \ingroup Animation
 */
class T_DLLCLASS Bone : public ISerializable
{
	T_RTTI_CLASS;

public:
	Bone();

	inline void setName(const std::wstring& name) { m_name = name; }

	inline const std::wstring& getName() const { return m_name; }

	inline void setParent(int32_t parent) { m_parent = parent; }

	inline int32_t getParent() const { return m_parent; }

	inline void setPosition(const Vector4& position) { m_position = position; }

	inline const Vector4& getPosition() const { return m_position; }

	inline void setOrientation(const Quaternion& orientation) { m_orientation = orientation; }

	inline const Quaternion& getOrientation() const { return m_orientation; }

	inline void setLength(const Scalar& length) { m_length = length; }

	inline const Scalar& getLength() const { return m_length; }

	inline void setRadius(const Scalar& radius) { m_radius = radius; }

	inline const Scalar& getRadius() const { return m_radius; }

	inline void setEnableLimits(bool enableLimits) { m_enableLimits = enableLimits; }

	inline bool getEnableLimits() const { return m_enableLimits; }

	inline void setTwistLimit(float twistLimit) { m_twistLimit = twistLimit; }

	inline float getTwistLimit() const { return m_twistLimit; }

	inline void setConeLimit(const Vector2& coneLimit) { m_coneLimit = coneLimit; }

	inline const Vector2& getConeLimit() const { return m_coneLimit; }

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_name;
	int32_t m_parent;
	Vector4 m_position;
	Quaternion m_orientation;
	Scalar m_length;
	Scalar m_radius;
	bool m_enableLimits;
	float m_twistLimit;
	Vector2 m_coneLimit;
};

	}
}

#endif	// traktor_animation_Bone_H
