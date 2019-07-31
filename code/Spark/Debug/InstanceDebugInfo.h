#pragma once

#include "Core/Math/Aabb2.h"
#include "Core/Serialization/ISerializable.h"
#include "Spark/ColorTransform.h"

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

class CharacterInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS InstanceDebugInfo : public ISerializable
{
	T_RTTI_CLASS;

public:
	InstanceDebugInfo();

	const std::string& getName() const { return m_name; }

	const Aabb2& getBounds() const { return m_bounds; }

	const Matrix33& getLocalTransform() const { return m_localTransform; }

	const Matrix33& getGlobalTransform() const { return m_globalTransform; }

	const ColorTransform& getColorTransform() const { return m_cxform; }

	bool getVisible() const { return m_visible; }

	virtual void serialize(ISerializer& s) override;

protected:
	std::string m_name;
	Aabb2 m_bounds;
	Matrix33 m_localTransform;
	Matrix33 m_globalTransform;
	ColorTransform m_cxform;
	bool m_visible;
};

	}
}
