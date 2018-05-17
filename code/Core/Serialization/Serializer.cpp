/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.Serializer", Serializer, ISerializer);

Serializer::Serializer()
:	m_versionPointer(0)
,	m_failure(false)
{
}

Ref< ISerializable > Serializer::readObject()
{
	if (getDirection() != SdRead)
		return 0;

	Ref< ISerializable > object;
	*this >> Member< ISerializable* >(L"object", object);
	return !m_failure ? object : 0;
}

bool Serializer::writeObject(const ISerializable* o)
{
	if (getDirection() != SdWrite)
		return false;

	Ref< ISerializable > mutableObject = const_cast< ISerializable* >(o);
	*this >> Member< ISerializable* >(L"object", mutableObject);
	return !m_failure;
}

int32_t Serializer::getVersion() const
{
	T_ASSERT (m_versionPointer > 0);
	return m_versions[m_versionPointer - 1].v;
}

int32_t Serializer::getVersion(const TypeInfo& typeInfo) const
{
	T_ASSERT (m_versionPointer > 0);
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
	for (const TypeInfo* ti = &type_of(inner); ti != 0; ti = ti->getSuper())
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
