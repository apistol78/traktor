#ifndef traktor_spray_BoxSource_H
#define traktor_spray_BoxSource_H

#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Range.h"
#include "Spray/Source.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Box particle source.
 * \ingroup Spray
 */
class T_DLLCLASS BoxSource : public Source
{
	T_RTTI_CLASS;

public:
	BoxSource();

	virtual bool bind(resource::IResourceManager* resourceManager);

	virtual void emit(
		EmitterUpdateContext& context,
		const Transform& transform,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const;

	virtual bool serialize(ISerializer& s);

	inline const Vector4& getPosition() const { return m_position; }

	inline const Vector4& getExtent() const { return m_extent; }

private:
	Vector4 m_position;
	Vector4 m_extent;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_BoxSource_H
