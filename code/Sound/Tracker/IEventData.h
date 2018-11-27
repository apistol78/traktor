#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class IEvent;

class T_DLLCLASS IEventData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IEvent > createInstance() const = 0;
};

	}
}
