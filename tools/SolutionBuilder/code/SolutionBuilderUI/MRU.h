#ifndef MRU_H
#define MRU_H

#include <vector>
#include <Core/Serialization/Serializable.h>
#include <Core/Io/Path.h>

/*! \brief Most-recently-used. */
class MRU : public traktor::Serializable
{
	T_RTTI_CLASS(MRU)

public:
	/*! \brief Called when a file has been successfully used, ie. opened or saved. */
	void usedFile(const traktor::Path& filePath);

	/*! \brief Get array of most recently used files. */
	bool getUsedFiles(std::vector< traktor::Path >& outFilePaths) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::vector< std::wstring > m_filePaths;
};

#endif	// MRU_H
