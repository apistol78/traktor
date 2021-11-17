#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedTypeInfo, 16 > s_allocBoxedTypeInfo;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.TypeInfo", BoxedTypeInfo, Boxed)

BoxedTypeInfo::BoxedTypeInfo()
:	m_value(*TypeInfo::find(L"traktor.Object"))
{
}

BoxedTypeInfo::BoxedTypeInfo(const TypeInfo& value)
:	m_value(value)
{
}

std::wstring BoxedTypeInfo::getName() const
{
	return m_value.getName();
}

int32_t BoxedTypeInfo::getSize() const
{
	return (int32_t)m_value.getSize();
}

int32_t BoxedTypeInfo::getVersion() const
{
	return m_value.getVersion();
}

Ref< ITypedObject > BoxedTypeInfo::createInstance() const
{
	return m_value.createInstance();
}

Ref< BoxedTypeInfo > BoxedTypeInfo::find(const std::wstring& name)
{
	const TypeInfo* type = TypeInfo::find(name.c_str());
	return type != nullptr ? new BoxedTypeInfo(*type) : nullptr;
}

Ref< BoxedRefArray > BoxedTypeInfo::findAllOf(const BoxedTypeInfo* typeInfo, bool inclusive)
{
	Ref< BoxedRefArray > boxedTypes = new BoxedRefArray();
	for (const auto type : typeInfo->unbox().findAllOf(inclusive))
		boxedTypes->push_back(new BoxedTypeInfo(*type));
	return boxedTypes;
}

std::wstring BoxedTypeInfo::toString() const
{
	return m_value.getName();
}

void* BoxedTypeInfo::operator new (size_t size)
{
	return s_allocBoxedTypeInfo.alloc();
}

void BoxedTypeInfo::operator delete (void* ptr)
{
	s_allocBoxedTypeInfo.free(ptr);
}

}
