#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedRefArray, 512 > s_allocBoxedRefArray;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.RefArray", BoxedRefArray, Boxed)

BoxedRefArray::BoxedRefArray()
{
}

int32_t BoxedRefArray::size() const
{
	return int32_t(m_arr.size());
}

bool BoxedRefArray::empty() const
{
	return m_arr.empty();
}

void BoxedRefArray::set(int32_t index, ITypedObject* object)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		m_arr[index] = object;
}

ITypedObject* BoxedRefArray::get(int32_t index)
{
	if (index >= 0 && index < int32_t(m_arr.size()))
		return m_arr[index];
	else
		return nullptr;
}

void BoxedRefArray::push_back(ITypedObject* object)
{
	m_arr.push_back(object);
}

void BoxedRefArray::pop_back()
{
	m_arr.pop_back();
}

ITypedObject* BoxedRefArray::front()
{
	return m_arr.front();
}

ITypedObject* BoxedRefArray::back()
{
	return m_arr.back();
}

std::wstring BoxedRefArray::toString() const
{
	StringOutputStream ss;
	ss << L"[" << int32_t(m_arr.size()) << L"]";
	return ss.str();
}

void* BoxedRefArray::operator new (size_t size)
{
	return s_allocBoxedRefArray.alloc();
}

void BoxedRefArray::operator delete (void* ptr)
{
	s_allocBoxedRefArray.free(ptr);
}

}
