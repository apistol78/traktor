/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/MemoryStream.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCompare.h"
#include "Core/Log/Log.h"
#include "SolutionBuilder/Utilities.h"

namespace traktor::sb
{

bool writeFileIfMismatch(const std::wstring& fileName, const AlignedVector< uint8_t >& data)
{
	bool needToWrite = false;

	Ref< File > existingFile = FileSystem::getInstance().get(fileName);
	if (existingFile != nullptr && existingFile->getSize() == data.size())
	{
		if (!data.empty())
		{
			Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead | File::FmMapped);
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

	if (needToWrite)
	{
		Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
		if (!file)
			return false;

		if (!data.empty())
			file->write(&data[0], int64_t(data.size()));

		file->close();
	}

	return true;
}

}
