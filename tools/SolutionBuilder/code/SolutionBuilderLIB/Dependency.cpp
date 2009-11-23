#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include "Dependency.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"Dependency", Dependency, ISerializable)

Dependency::Dependency()
:	m_linkWithProduct(true)
{
}

void Dependency::setLinkWithProduct(bool linkWithProduct)
{
	m_linkWithProduct = linkWithProduct;
}

bool Dependency::shouldLinkWithProduct() const
{
	return m_linkWithProduct;
}

bool Dependency::serialize(ISerializer& s)
{
	return s >> Member< bool >(L"linkWithProduct", m_linkWithProduct);
}
