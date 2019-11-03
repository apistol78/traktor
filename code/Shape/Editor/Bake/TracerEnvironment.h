#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

class IProbe;

class T_DLLCLASS TracerEnvironment : public Object
{
	T_RTTI_CLASS;

public:
	explicit TracerEnvironment(const IProbe* environment);

	const IProbe* getEnvironment() const { return m_environment; }

private:
	Ref< const IProbe > m_environment;
};

	}
}