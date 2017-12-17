/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef MRU_H
#define MRU_H

#include <vector>
#include <Core/Serialization/ISerializable.h>
#include <Core/Io/Path.h>

namespace traktor
{
	namespace sb
	{

/*! \brief Most-recently-used. */
class MRU : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! \brief Called when a file has been successfully used, ie. opened or saved. */
	void usedFile(const Path& filePath);

	/*! \brief Get array of most recently used files. */
	bool getUsedFiles(std::vector< Path >& outFilePaths) const;

	virtual void serialize(ISerializer& s);

private:
	std::vector< std::wstring > m_filePaths;
};

	}
}

#endif	// MRU_H
