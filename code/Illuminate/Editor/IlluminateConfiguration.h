#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace illuminate
	{

class T_DLLCLASS IlluminateConfiguration : public ISerializable
{
	T_RTTI_CLASS;

public:
	IlluminateConfiguration();

	bool traceDirect() const { return m_traceDirect; }

	bool traceIndirect() const { return m_traceIndirect; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_traceDirect;
	bool m_traceIndirect;
};

	}
}