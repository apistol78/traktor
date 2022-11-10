/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Model/Model.h"
#include "Model/Formats/Tmd/ModelFormatTmd.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatTmd", 0, ModelFormatTmd, ModelFormat)

void ModelFormatTmd::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Traktor Model";
	outExtensions.push_back(L"tmd");
}

bool ModelFormatTmd::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"tmd") == 0;
}

Ref< Model > ModelFormatTmd::read(const Path& filePath, const std::wstring& filter) const
{
	Ref< IStream > stream = FileSystem::getInstance().open(filePath, File::FmRead | File::FmMapped);
	if (stream)
		return BinarySerializer(stream).readObject< Model >();
	else
		return nullptr;
}

bool ModelFormatTmd::write(const Path& filePath, const Model* model) const
{
	Ref< IStream > stream = FileSystem::getInstance().open(filePath, File::FmWrite);
	if (!stream)
		return false;

	BufferedStream bs(stream);
	if (!BinarySerializer(&bs).writeObject(model))
		return false;
	bs.flush();

	return true;
}

	}
}
