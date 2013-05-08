#ifndef traktor_editor_MRU_H
#define traktor_editor_MRU_H

#include <vector>
#include "Core/Serialization/ISerializable.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace editor
	{

/*! \brief Most-recently-used. */
class MRU : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! \brief Called when a file has been successfully used, ie. opened or saved. */
	void usedFile(const traktor::Path& filePath);

	/*! \brief Get array of most recently used files. */
	bool getUsedFiles(std::vector< traktor::Path >& outFilePaths) const;

	virtual void serialize(ISerializer& s);

private:
	std::vector< std::wstring > m_filePaths;
};

	}
}

#endif	// traktor_editor_MRU_H
