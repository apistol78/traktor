#pragma once

#include <vector>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptBlob;

/*! \brief
 * \ingroup Script
 */
class T_DLLCLASS ScriptResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const std::vector< Guid >& getDependencies() const { return m_dependencies; }

	const IScriptBlob* getBlob() const { return m_blob; }

private:
	friend class ScriptPipeline;

	std::vector< Guid > m_dependencies;
	Ref< const IScriptBlob > m_blob;
};

	}
}

