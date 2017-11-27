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
:	m_failure(false)
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
	T_ASSERT (!m_versions.empty());
	return m_versions.back().v;
}

int32_t Serializer::getVersion(const TypeInfo& typeInfo) const
{
	T_ASSERT (!m_versions.empty());
	const dataVersionMap_t& dv = m_versions.back().dvm;
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

	Version& v = m_versions.push_back();
	v.dvm = dataVersions;
	v.v = (it != dataVersions.end()) ? it->second : 0;

	inner->serialize(*this);

	m_versions.pop_back();
}

}
