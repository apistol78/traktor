#ifndef traktor_spray_EmitterData_H
#define traktor_spray_EmitterData_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class InstanceMesh;

	}

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

class Emitter;
class ModifierData;
class SourceData;

/*! \brief Emitter persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EmitterData : public ISerializable
{
	T_RTTI_CLASS;

public:
	EmitterData();

	Ref< Emitter > createEmitter(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

	const SourceData* getSource() const { return m_source; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const resource::Id< mesh::InstanceMesh >& getMesh() const { return m_mesh; }

private:
	Ref< SourceData > m_source;
	RefArray< ModifierData > m_modifiers;
	resource::Id< render::Shader > m_shader;
	resource::Id< mesh::InstanceMesh > m_mesh;
	float m_middleAge;
	float m_cullNearDistance;
	float m_fadeNearRange;
	float m_warmUp;
	bool m_sort;
	bool m_worldSpace;
	bool m_meshOrientationFromVelocity;
};

	}
}

#endif	// traktor_spray_EmitterData_H
