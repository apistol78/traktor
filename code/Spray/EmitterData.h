/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

namespace traktor::mesh
{

class InstanceMesh;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class Shader;

}

namespace traktor::world
{

class IEntityFactory;

}

namespace traktor::spray
{

class EffectData;
class Emitter;
class ModifierData;
class SourceData;

/*! Emitter persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EmitterData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< Emitter > createEmitter(resource::IResourceManager* resourceManager, const world::IEntityFactory* entityFactory) const;

	virtual void serialize(ISerializer& s) override final;

	void setSource(SourceData* source)  { m_source = source; }

	const SourceData* getSource() const { return m_source; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const resource::Id< mesh::InstanceMesh >& getMesh() const { return m_mesh; }

	const EffectData* getEffect() const { return m_effect; }

private:
	Ref< SourceData > m_source;
	RefArray< ModifierData > m_modifiers;
	resource::Id< render::Shader > m_shader;
	resource::Id< mesh::InstanceMesh > m_mesh;
	Ref< EffectData > m_effect;
	float m_middleAge = 0.2f;
	float m_cullNearDistance = 0.25f;
	float m_cullMeshDistance = 100.0f;
	float m_fadeNearRange = 1.0f;
	float m_viewOffset = 0.0f;
	float m_warmUp = 0.0f;
	bool m_sort = false;
	bool m_worldSpace = true;
	bool m_meshOrientationFromVelocity = true;
};

}
