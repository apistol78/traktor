/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/MemoryStream.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCompare.h"
#include "Core/Log/Log.h"
#include "SolutionBuilder/Utilities.h"

namespace traktor
{
	namespace sb
	{

bool writeFileIfMismatch(const std::wstring& fileName, const AlignedVector< uint8_t >& data)
{
	bool needToWrite = false;

	Ref< File > existingFile = FileSystem::getInstance().get(fileName);
	if (existingFile)
	{
		if (existingFile->getSize() == data.size())
		{
			if (!data.empty())
			{
				Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
				if (file)
				{
					MemoryStream dataStream(&data[0], data.size());
					if (!StreamCompare(file, &dataStream).execute())
						needToWrite = true;
					file->close();
				}
				else
					needToWrite = true;
			}
		}
		else
			needToWrite = true;
	}
	else
		needToWrite = true;

	if (needToWrite)
	{
		Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
		if (!file)
			return false;

		if (!data.empty())
			file->write(&data[0], int(data.size()));

		file->close();
	}

	return true;
}

	}
}
