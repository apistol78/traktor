/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/Serializer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Serializer", Serializer, ISerializer);

Ref< ISerializable > Serializer::readObject()
{
	if (getDirection() != Direction::Read)
		return nullptr;

	Ref< ISerializable > object;
	*this >> Member< ISerializable* >(L"object", object);
	return !m_failure ? object : nullptr;
}

bool Serializer::writeObject(const ISerializable* o)
{
	if (getDirection() != Direction::Write)
		return false;

	Ref< ISerializable > mutableObject = const_cast< ISerializable* >(o);
	*this >> Member< ISerializable* >(L"object", mutableObject);
	return !m_failure;
}

int32_t Serializer::getVersion() const
{
	T_ASSERT(m_versionPointer > 0);
	return m_versions[m_versionPointer - 1].v;
}

int32_t Serializer::getVersion(const TypeInfo& typeInfo) const
{
	T_ASSERT(m_versionPointer > 0);
	const dataVersionMap_t& dv = m_versions[m_versionPointer - 1].dvm;
	dataVersionMap_t::const_iterator it = dv.find(&typeInfo);
	return it != dv.end() ? it->second : 0;
}

void Serializer::failure()
{
	m_failure = true;
}

void Serializer::serialize(ISerializable* inner)
{
	if (!inner || m_failure)
		return;

	dataVersionMap_t dataVersions;
	for (const TypeInfo* ti = &type_of(inner); ti != nullptr; ti = ti->getSuper())
		dataVersions.insert(std::make_pair(
			ti,
			ti->getVersion()
		));

	serialize(inner, dataVersions);
}

void Serializer::serialize(ISerializable* inner, const dataVersionMap_t& dataVersions)
{
	if (!inner || m_failure)
		return;

	dataVersionMap_t::const_iterator it = dataVersions.find(&type_of(inner));

	if (m_versionPointer >= m_versions.size())
		m_versions.resize(m_versionPointer + 16);

	// Push version scope.
	{
		Version& v = m_versions[m_versionPointer++];
		v.dvm = dataVersions;
		v.v = (it != dataVersions.end()) ? it->second : 0;
	}

	inner->serialize(*this);

	// Pop version scope.
	{
		Version& v = m_versions[--m_versionPointer];
		v.dvm.clear();
	}
}

}
