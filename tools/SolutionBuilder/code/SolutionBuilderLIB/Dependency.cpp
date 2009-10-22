#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include "Dependency.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"Dependency", Dependency, Serializable)

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

bool Dependency::serialize(Serializer& s)
{
	return s >> Member< bool >(L"linkWithProduct", m_linkWithProduct);
}
