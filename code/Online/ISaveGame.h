#ifndef traktor_online_ISaveGame_H
#define traktor_online_ISaveGame_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class Serializable;

	namespace online
	{

class T_DLLCLASS ISaveGame : public Object
{
	T_RTTI_CLASS(ISaveGame)

public:
	virtual std::wstring getName() const = 0;

	virtual Ref< Serializable > getAttachment() const = 0;
};

	}
}

#endif	// traktor_online_ISaveGame_H
