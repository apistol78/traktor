#include "Sound/Filters/SurroundEnvironment.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SurroundEnvironment", SurroundEnvironment, Object)

SurroundEnvironment::SurroundEnvironment()
:	m_listenerTransform(Matrix44::identity())
{
}

void SurroundEnvironment::setListenerTransform(const Matrix44& listenerTransform)
{
	m_listenerTransform = listenerTransform;
}

const Matrix44& SurroundEnvironment::getListenerTransform() const
{
	return m_listenerTransform;
}

	}
}
