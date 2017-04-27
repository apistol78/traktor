/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_Emitter_H
#define traktor_spray_Emitter_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Resource/Proxy.h"

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

	namespace render
	{

class Shader;

	}

	namespace spray
	{

class Effect;
class EmitterInstance;
class Modifier;
class Source;

/*! \brief Emitter
 * \ingroup Spray
 */
class T_DLLCLASS Emitter : public Object
{
	T_RTTI_CLASS;

public:
	Emitter(
		const Source* source,
		const RefArray< const Modifier >& modifiers,
		const resource::Proxy< render::Shader >& shader,
		const resource::Proxy< mesh::InstanceMesh >& mesh,
		const Effect* effect,
		float middleAge,
		float cullNearDistance,
		float cullMeshDistance,
		float fadeNearRange,
		float viewOffset,
		float warmUp,
		bool sort,
		bool worldSpace,
		bool meshOrientationFromVelocity
	);

	Ref< EmitterInstance > createInstance(float duration) const;

	const Source* getSource() const { return m_source; }

	const RefArray< const Modifier >& getModifiers() const { return m_modifiers; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	const resource::Proxy< mesh::InstanceMesh >& getMesh() const { return m_mesh; }

	const Effect* getEffect() const { return m_effect; }

	float getMiddleAge() const { return m_middleAge; }

	float getCullNearDistance() const { return m_cullNearDistance; }

	float getCullMeshDistance() const { return m_cullMeshDistance; }

	float getFadeNearRange() const { return m_fadeNearRange; }

	float getViewOffset() const { return m_viewOffset; }

	float getWarmUp() const { return m_warmUp; }

	bool getSort() const { return m_sort; }

	bool worldSpace() const { return m_worldSpace; }

	bool meshOrientationFromVelocity() const { return m_meshOrientationFromVelocity; }

private:
	Ref< const Source > m_source;
	RefArray< const Modifier > m_modifiers;
	resource::Proxy< render::Shader > m_shader;
	resource::Proxy< mesh::InstanceMesh > m_mesh;
	Ref< const Effect > m_effect;
	float m_middleAge;
	float m_cullNearDistance;
	float m_cullMeshDistance;
	float m_fadeNearRange;
	float m_viewOffset;
	float m_warmUp;
	bool m_sort;
	bool m_worldSpace;
	bool m_meshOrientationFromVelocity;
};

	}
}

#endif	// traktor_spray_Emitter_H
