#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class Variable;

	}

	namespace runtime
	{

/*! Response from running target when breadcrumbs has been captured.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerBreadcrumbs : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerBreadcrumbs() = default;

	explicit ScriptDebuggerBreadcrumbs(const AlignedVector< uint32_t >& breadcrumbs);

	const AlignedVector< uint32_t >& getBreadcrumbs() const { return m_breadcrumbs; }

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< uint32_t > m_breadcrumbs;
};

	}
}

