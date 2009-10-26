#ifndef traktor_spray_Emitter_H
#define traktor_spray_Emitter_H

#include "Resource/Proxy.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"

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
class T_DLLCLASS Emitter : public Serializable
{
	T_RTTI_CLASS(Emitter)

public:
	Emitter();

	EmitterInstance* createInstance(resource::IResourceManager* resourceManager);

	inline Source* getSource() const { return m_source; }

	inline RefArray< Modifier >& getModifiers() { return m_modifiers; }

	inline resource::Proxy< render::Shader >& getShader() { return m_shader; }

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	inline float getMiddleAge() const { return m_middleAge; }

	inline float getWarmUp() const { return m_warmUp; }

	virtual int getVersion() const;

	virtual bool serialize(Serializer& s);

private:
	Ref< Source > m_source;
	RefArray< Modifier > m_modifiers;
	resource::Proxy< render::Shader > m_shader;
	float m_middleAge;
	float m_warmUp;
};

	}
}

#endif	// traktor_spray_Emitter_H
