/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Resource/Proxy.h"
#include "Spray/EmitterData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class InstanceMesh;

}

namespace traktor::render
{

class IRenderSystem;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::spray
{

class Effect;
class IEmitterInstance;
class Modifier;
class Source;

/*! Emitter
 * \ingroup Spray
 */
class T_DLLCLASS Emitter : public Object
{
	T_RTTI_CLASS;

public:
	explicit Emitter(
		const EmitterData* data,
		const Source* source,
		const RefArray< const Modifier >& modifiers,
		const resource::Proxy< render::Shader >& shader,
		const resource::Proxy< mesh::InstanceMesh >& mesh,
		const Effect* effect
	);

	Ref< IEmitterInstance > createInstance(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager, float duration) const;

	const Source* getSource() const { return m_source; }

	const RefArray< const Modifier >& getModifiers() const { return m_modifiers; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	const resource::Proxy< mesh::InstanceMesh >& getMesh() const { return m_mesh; }

	const Effect* getEffect() const { return m_effect; }

	float getMiddleAge() const { return m_data->m_middleAge; }

	float getCullNearDistance() const { return m_data->m_cullNearDistance; }

	float getCullMeshDistance() const { return m_data->m_cullMeshDistance; }

	float getFadeNearRange() const { return m_data->m_fadeNearRange; }

	float getViewOffset() const { return m_data->m_viewOffset; }

	float getWarmUp() const { return m_data->m_warmUp; }

	bool getSort() const { return m_data->m_sort; }

	bool worldSpace() const { return m_data->m_worldSpace; }

	bool meshOrientationFromVelocity() const { return m_data->m_meshOrientationFromVelocity; }

private:
	Ref< const EmitterData > m_data;
	Ref< const Source > m_source;
	RefArray< const Modifier > m_modifiers;
	resource::Proxy< render::Shader > m_shader;
	resource::Proxy< mesh::InstanceMesh > m_mesh;
	Ref< const Effect > m_effect;
};

}
