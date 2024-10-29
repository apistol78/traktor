/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor::editor
{

/*! Most-recently-used. */
class MRU : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Called when a file has been successfully used, i.e. opened or saved. */
	void usedFile(const Path& filePath);

	/*! Get array of most recently used files. */
	bool getUsedFiles(AlignedVector< Path >& outFilePaths) const;

	/*! Get most recently used file; return empty string if no file used. */
	Path getMostRecentlyUseFile() const;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< std::wstring > m_filePaths;
};

}
