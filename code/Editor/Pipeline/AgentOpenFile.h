#pragma once

#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace editor
	{

/*!
 * \ingroup Editor
 */
class T_DLLCLASS AgentOpenFile : public ISerializable
{
	T_RTTI_CLASS;

public:
	AgentOpenFile() = default;

	explicit AgentOpenFile(const Path& filePath);

	const Path& getFilePath() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Path m_filePath;
};

	}
}

