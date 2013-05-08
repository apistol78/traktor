#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

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

int Serializer::getVersion() const
{
	return !m_constructing.empty() ? m_constructing.back().second : 0;
}

ISerializable* Serializer::getCurrentObject()
{
	return !m_constructing.empty() ? m_constructing.back().first : 0;
}

void Serializer::failure()
{
	m_failure = true;
}

void Serializer::serialize(ISerializable* inner, int version)
{
	if (!inner || m_failure)
		return;

	m_constructing.push_back(std::make_pair(
		inner,
		version
	));

	inner->serialize(*this);

	m_constructing.pop_back();
}

}
