/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Guid.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Editor/App/InstanceClipboardData.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.InstanceClipboardData", 0, InstanceClipboardData, ISerializable)

InstanceClipboardData::InstanceClipboardData()
{
}

void InstanceClipboardData::addInstance(const std::wstring& name, ISerializable* object, const Guid& id)
{
	Instance instance;
	instance.name = name;
	instance.object = object;
	instance.originalId = id;
	m_instances.push_back(instance);
}

void InstanceClipboardData::serialize(ISerializer& s)
{
	s >> MemberStlList< Instance, MemberComposite< Instance > >(L"instances", m_instances);
}

void InstanceClipboardData::Instance::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> MemberRef< ISerializable >(L"object", object);
	s >> Member< Guid >(L"originalId", originalId);
	s >> Member< Guid >(L"pasteId", pasteId);
}

	}
}
