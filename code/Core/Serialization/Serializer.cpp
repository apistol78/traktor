/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

int32_t Serializer::getVersion(const TypeInfo& typeInfo) const
{
	return !m_constructing.empty() ? m_constructing.back() : 0;
}

void Serializer::failure()
{
	m_failure = true;
}

void Serializer::serialize(ISerializable* inner, int32_t version)
{
	if (!inner || m_failure)
		return;

	m_constructing.push_back(version);

	inner->serialize(*this);

	m_constructing.pop_back();
}

}
