#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.Serializer", Serializer, Object)

Ref< Serializable > Serializer::readObject()
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
	Ref< Serializable > mutableObject(const_cast< Serializable* >(o));

	if (getDirection() != SdWrite)
		return false;

	if (!(*this >> MemberRef< Serializable >(L"object", mutableObject)))
		return false;

	return true;
}

Ref< Serializable > Serializer::getCurrentObject()
{
	return !m_constructing.empty() ? m_constructing.back().first : 0;
}

Ref< Serializable > Serializer::getOuterObject()
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

bool Serializer::serialize(Serializable* inner, int version, Serializable* outer)
{
	if (!inner)
		return false;

	if (outer)
		m_constructing.push_back(std::make_pair(
			outer,
			outer->getVersion()
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
