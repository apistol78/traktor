#pragma once

#include <vector>
#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace sb
	{

/*! Most-recently-used. */
class MRU : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Called when a file has been successfully used, ie. opened or saved. */
	void usedFile(const Path& filePath);

	/*! Get array of most recently used files. */
	bool getUsedFiles(std::vector< Path >& outFilePaths) const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::vector< std::wstring > m_filePaths;
};

	}
}

