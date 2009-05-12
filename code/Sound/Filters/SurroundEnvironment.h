#ifndef traktor_sound_SurroundEnvironment_H
#define traktor_sound_SurroundEnvironment_H

#include "Core/Object.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SurroundEnvironment : public Object
{
	T_RTTI_CLASS(SurroundEnvironment)

public:
	SurroundEnvironment();

	void setListenerTransform(const Matrix44& listenerTransform);

	const Matrix44& getListenerTransform() const;

private:
	Matrix44 m_listenerTransform;
};

	}
}

#endif	// traktor_sound_SurroundEnvironment_H
