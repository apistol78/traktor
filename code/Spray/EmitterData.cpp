/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spray/EffectData.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterData.h"
#include "Spray/SourceData.h"
#include "Spray/ModifierData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EmitterData", 7, EmitterData, ISerializable)

Ref< Emitter > EmitterData::createEmitter(resource::IResourceManager* resourceManager, const world::IEntityFactory* entityFactory) const
{
	if (!m_source)
		return nullptr;

	resource::Proxy< render::Shader > shader;
	resource::Proxy< mesh::InstanceMesh > mesh;

	if (
		!resourceManager->bind(m_shader, shader) &&
		!resourceManager->bind(m_mesh, mesh) &&
		!m_effect
	)
		return nullptr;

	Ref< const Source > source = m_source->createSource(resourceManager);
	if (!source)
		return nullptr;

	RefArray< const Modifier > modifiers;
	for (auto modifierData : m_modifiers)
	{
		Ref< const Modifier > modifier = modifierData->createModifier(resourceManager);
		if (modifier)
			modifiers.push_back(modifier);
	}

	Ref< const Effect > effect;
	if (m_effect)
		effect = m_effect->createEffect(resourceManager, entityFactory);

	return new Emitter(
		source,
		modifiers,
		shader,
		mesh,
		effect,
		m_middleAge,
		m_cullNearDistance,
		m_cullMeshDistance,
		m_fadeNearRange,
		m_viewOffset,
		m_warmUp,
		m_sort,
		m_worldSpace,
		m_meshOrientationFromVelocity
	);
}

void EmitterData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT (s.getVersion< EmitterData >() >= 7);

	s >> MemberRef< SourceData >(L"source", m_source);
	s >> MemberRefArray< ModifierData >(L"modifiers", m_modifiers);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> resource::Member< mesh::InstanceMesh >(L"mesh", m_mesh);
	s >> MemberRef< EffectData >(L"effect", m_effect);
	s >> Member< float >(L"middleAge", m_middleAge, AttributeUnit(UnitType::Seconds));
	s >> Member< float >(L"cullNearDistance", m_cullNearDistance, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"cullMeshDistance", m_cullMeshDistance, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"fadeNearRange", m_fadeNearRange, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"viewOffset", m_viewOffset, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"warmUp", m_warmUp, AttributeUnit(UnitType::Seconds));
	s >> Member< bool >(L"sort", m_sort);
	s >> Member< bool >(L"worldSpace", m_worldSpace);
	s >> Member< bool >(L"meshOrientationFromVelocity", m_meshOrientationFromVelocity);
}

}
