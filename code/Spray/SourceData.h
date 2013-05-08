#ifndef traktor_spray_SourceData_H
#define traktor_spray_SourceData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
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

	namespace spray
	{

class Source;

/*! \brief Particle persistent source data.
 * \ingroup Spray
 */
class T_DLLCLASS SourceData : public ISerializable
{
	T_RTTI_CLASS;

public:
	SourceData();

	virtual Ref< Source > createSource(resource::IResourceManager* resourceManager) const = 0;

	virtual void serialize(ISerializer& s);

	float getConstantRate() const { return m_constantRate; }

	float getVelocityRate() const { return m_velocityRate; }

private:
	float m_constantRate;
	float m_velocityRate;
};

	}
}

#endif	// traktor_spray_SourceData_H
