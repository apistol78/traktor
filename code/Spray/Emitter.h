#ifndef traktor_spray_Emitter_H
#define traktor_spray_Emitter_H

#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class Shader;

	}

	namespace spray
	{

class EmitterInstance;

class Source;
class Modifier;

/*! \brief Emitter
 * \ingroup Spray
 */
class T_DLLCLASS Emitter : public ISerializable
{
	T_RTTI_CLASS;

public:
	Emitter();

	bool bind(resource::IResourceManager* resourceManager);

	Ref< EmitterInstance > createInstance();

	inline Ref< Source > getSource() const { return m_source; }

	inline RefArray< Modifier >& getModifiers() { return m_modifiers; }

	inline resource::Proxy< render::Shader >& getShader() { return m_shader; }

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	inline float getMiddleAge() const { return m_middleAge; }

	inline float getCullNearDistance() const { return m_cullNearDistance; }

	inline float getFadeNearRange() const { return m_fadeNearRange; }

	inline float getWarmUp() const { return m_warmUp; }

	virtual bool serialize(ISerializer& s);

private:
	Ref< Source > m_source;
	RefArray< Modifier > m_modifiers;
	resource::Proxy< render::Shader > m_shader;
	float m_middleAge;
	float m_cullNearDistance;
	float m_fadeNearRange;
	float m_warmUp;
};

	}
}

#endif	// traktor_spray_Emitter_H
