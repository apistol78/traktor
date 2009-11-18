#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.Serializer", Serializer, ISerializer);

Ref< ISerializable > Serializer::readObject()
{
	Ref< ISerializable > object;

	if (getDirection() != SdRead)
		return 0;

	if (!(*this >> Member< Ref< ISerializable > >(L"object", object)))
		return 0;

	return object;
}

bool Serializer::writeObject(const ISerializable* o)
{
	Ref< ISerializable > mutableObject(const_cast< ISerializable* >(o));

	if (getDirection() != SdWrite)
		return false;

	if (!(*this >> Member< Ref< ISerializable > >(L"object", mutableObject)))
		return false;

	return true;
}

ISerializable* Serializer::getCurrentObject()
{
	return !m_constructing.empty() ? m_constructing.back().first : 0;
}

ISerializable* Serializer::getOuterObject()
{
	return !m_constructing.empty() ? m_constructing.front().first : 0;
}

int Serializer::getVersion() const
{
	return !m_constructing.empty() ? m_constructing.back().second : 0;
}

bool Serializer::serialize(ISerializable* inner, int version, ISerializable* outer)
{
	if (!inner)
		return false;

	if (outer)
		m_constructing.push_back(std::make_pair(
			outer,
			type_of(outer).getVersion()
		));

	m_constructing.push_back(std::make_pair(
		inner,
		version
	));

	bool result = inner->serialize(*this);

	m_constructing.pop_back();
	if (outer)
		m_constructing.pop_back();

	return result;
}

}
