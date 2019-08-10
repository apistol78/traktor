#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedColor4f, 16 > s_allocBoxedColor4f;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Color4f", BoxedColor4f, Boxed)

BoxedColor4f::BoxedColor4f(const Color4f& value)
:	m_value(value)
{
}

BoxedColor4f::BoxedColor4f()
:	m_value(1.0f, 1.0f, 1.0f, 1.0f)
{
}

BoxedColor4f::BoxedColor4f(float red, float green, float blue)
:	m_value(red, green, blue, 1.0f)
{
}

BoxedColor4f::BoxedColor4f(float red, float green, float blue, float alpha)
:	m_value(red, green, blue, alpha)
{
}

std::wstring BoxedColor4f::toString() const
{
	StringOutputStream ss;
	ss << m_value.getRed() << L", " << m_value.getGreen() << L", " << m_value.getBlue() << L", " << m_value.getAlpha();
	return ss.str();
}

void* BoxedColor4f::operator new (size_t size)
{
	return s_allocBoxedColor4f.alloc();
}

void BoxedColor4f::operator delete (void* ptr)
{
	s_allocBoxedColor4f.free(ptr);
}

}
