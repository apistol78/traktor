#ifndef MRU_H
#define MRU_H

#include <vector>
#include <Core/Serialization/ISerializable.h>
#include <Core/Io/Path.h>

/*! \brief Most-recently-used. */
class MRU : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	/*! \brief Called when a file has been successfully used, ie. opened or saved. */
	void usedFile(const traktor::Path& filePath);

	/*! \brief Get array of most recently used files. */
	bool getUsedFiles(std::vector< traktor::Path >& outFilePaths) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::vector< std::wstring > m_filePaths;
};

#endif	// MRU_H
