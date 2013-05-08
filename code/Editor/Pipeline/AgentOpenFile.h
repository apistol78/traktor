#ifndef traktor_editor_AgentOpenFile_H
#define traktor_editor_AgentOpenFile_H

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

/*! \brief
 * \ingroup Editor
 */
class T_DLLCLASS AgentOpenFile : public ISerializable
{
	T_RTTI_CLASS;

public:
	AgentOpenFile();

	AgentOpenFile(const Path& basePath, const std::wstring& fileName);

	const Path& getBasePath() const;

	const std::wstring& getFileName() const;

	virtual void serialize(ISerializer& s);

private:
	Path m_basePath;
	std::wstring m_fileName;
};

	}
}

#endif	// traktor_editor_AgentOpenFile_H
