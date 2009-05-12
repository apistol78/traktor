#ifndef traktor_animation_LwsDocument_H
#define traktor_animation_LwsDocument_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

namespace traktor
{

class Stream;

	namespace animation
	{

class LwsGroup;

class LwsDocument : public Object
{
	T_RTTI_CLASS(LwsDocument)

public:
	static LwsDocument* parse(Stream* stream);

	LwsGroup* getRootGroup();

private:
	Ref< LwsGroup > m_rootGroup;
};

	}
}

#endif	// traktor_animation_LwsDocument_H
