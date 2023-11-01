/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <map>
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Editor/App/ClearCachesTool.h"
#include "I18N/Text.h"

namespace traktor::editor
{
	namespace
	{

bool clearDirectory(const std::wstring& path)
{
	RefArray< File > files = FileSystem::getInstance().find(path + L"/*.*");
	for (auto file : files)
	{
		Path filePath = file->getPath();
		if (filePath.getFileName() == L"." || filePath.getFileName() == L"..")
			continue;
		if (!file->isDirectory())
		{
			if (!FileSystem::getInstance().remove(filePath))
				return false;
		}
		else
		{
			if (!clearDirectory(filePath.getPathName()))
				return false;
			if (!FileSystem::getInstance().removeDirectory(filePath))
				return false;
		}
	}
	return true;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ClearCachesTool", 0, ClearCachesTool, IEditorTool)

std::wstring ClearCachesTool::getDescription() const
{
	return i18n::Text(L"EDITOR_CLEAR_CACHES");
}

Ref< ui::IBitmap > ClearCachesTool::getIcon() const
{
	return nullptr;
}

bool ClearCachesTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool ClearCachesTool::launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param)
{
	const std::wstring cachePath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.InstanceCache.Path");
	log::info << L"Clearing pipeline instance cache \"" << cachePath << L"\"..." << Endl;
	if (!clearDirectory(cachePath))
	{
		log::error << L"Failed to clear path \"" << cachePath << L"\"." << Endl;
		return false;
	}

	if (editor->getSettings()->getProperty< bool >(L"Pipeline.FileCache", false))
	{
		const std::wstring cachePath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.FileCache.Path");
		log::info << L"Clearing pipeline output cache \"" << cachePath << L"\"..." << Endl;
		if (!clearDirectory(cachePath))
		{
			log::error << L"Failed to clear path \"" << cachePath << L"\"." << Endl;
			return false;
		}
	}

	{
		const std::wstring cachePath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");
		log::info << L"Clearing model cache \"" << cachePath << L"\"..." << Endl;
		if (!clearDirectory(cachePath))
		{
			log::error << L"Failed to clear path \"" << cachePath << L"\"." << Endl;
			return false;
		}
	}

	{
		const std::wstring connectionString = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.Db");

		std::vector< std::wstring > pairs;
		std::map< std::wstring, std::wstring > cs;

		if (Split< std::wstring >::any(connectionString, L";", pairs) == 0)
		{
			log::error << L"Unable to clear pipeline db; incorrect connection string." << Endl;
			return false;
		}

		for (std::vector< std::wstring >::const_iterator i = pairs.begin(); i != pairs.end(); ++i)
		{
			size_t p = i->find(L'=');
			if (p == 0 || p == i->npos)
			{
				log::error << L"Unable to clear pipeline db; incorrect connection string." << Endl;
				return false;
			}

			cs[trim(i->substr(0, p))] = i->substr(p + 1);
		}

		const std::wstring file = cs[L"fileName"];
		log::info << L"Clearing pipeline db \"" << file << L"\"..." << Endl;
		if (!FileSystem::getInstance().remove(Path(file)))
		{
			 log::error << L"Failed to clear file \"" << file << L"\"." << Endl;
			return false;
		}

		// \tbd need to discard pipeline db from memory as well.
	}

	log::info << L"Caches cleared successfully." << Endl;
	return true;
}

}
