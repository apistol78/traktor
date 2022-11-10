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
#include "Model/ModelFormat.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelFormat", ModelFormat, Object)

Ref< Model > ModelFormat::readAny(const Path& filePath, const std::wstring& filter)
{
	Ref< Model > md;
	for (const auto formatType : type_of< ModelFormat >().findAllOf(false))
	{
		Ref< ModelFormat > modelFormat = dynamic_type_cast< ModelFormat* >(formatType->createInstance());
		if (!modelFormat || !modelFormat->supportFormat(filePath.getExtension()))
			continue;

		md = modelFormat->read(filePath, filter);
		if (md)
			break;
	}
	return md;
}

bool ModelFormat::writeAny(const Path& filePath, const Model* model)
{
	std::wstring extension = filePath.getExtension();
	for (const auto formatType : type_of< ModelFormat >().findAllOf())
	{
		Ref< ModelFormat > modelFormat = dynamic_type_cast< ModelFormat* >(formatType->createInstance());
		if (!modelFormat)
			continue;

		std::wstring description;
		std::vector< std::wstring > extensions;
		modelFormat->getExtensions(description, extensions);

		for (const auto& ext : extensions)
		{
			if (compareIgnoreCase(extension, ext) == 0)
				return modelFormat->write(filePath, model);
		}
	}
	return false;
}

	}
}
