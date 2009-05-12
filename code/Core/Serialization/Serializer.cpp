#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.Serializer", Serializer, Object)

Serializable* Serializer::readObject()
{
	Ref< Serializable > object;

	if (getDirection() != SdRead)
		return 0;

	if (!(*this >> MemberRef< Serializable >(L"object", object)))
		return 0;

	return object;
}

bool Serializer::writeObject(const Serializable* o)
{
	if (getDirection() != SdWrite)
		return false;

	Serializable* mutableObject = const_cast< Serializable* >(o);

	if (!(*this >> Member< Serializable* >(L"object", mutableObject)))
		return false;

	return true;
}

Serializable* Serializer::getCurrentObject()
{
	return !m_constructing.empty() ? m_constructing.back().first : 0;
}

Serializable* Serializer::getOuterObject()
{
	return !m_constructing.empty() ? m_constructing.front().first : 0;
}

int Serializer::getVersion() const
{
	return !m_constructing.empty() ? m_constructing.back().second : 0;
}

bool Serializer::operator >> (const MemberEnumBase& m)
{
	return this->operator >> (*(MemberComplex*)(&m));
}

bool Serializer::serialize(Serializable* o, int version)
{
	if (!o)
		return false;

	m_constructing.push_back(std::make_pair(o, version));
	bool result = o->serialize(*this);
	m_constructing.pop_back();

	return result;
}

}
