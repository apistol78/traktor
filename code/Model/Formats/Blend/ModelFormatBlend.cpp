/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/String.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Model/Model.h"
#include "Model/Formats/Blend/ModelFormatBlend.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

Semaphore g_lock;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatBlend", 0, ModelFormatBlend, ModelFormat)

void ModelFormatBlend::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Blender";
	outExtensions.push_back(L"blend");
}

bool ModelFormatBlend::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase(extension, L"blend") == 0;
}

Ref< Model > ModelFormatBlend::read(const Path& filePath, const std::wstring& filter) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(g_lock);

	const std::wstring threadFolder = str(L"%08x", ThreadManager::getInstance().getCurrentThread()->id());
	const std::wstring intermediate = L"intermediate.fbx";

	// Determine working path.
	std::wstring scratchPath = OS::getInstance().getWritableFolderPath() + L"/Traktor/Blender/" + threadFolder;
	if (!FileSystem::getInstance().makeAllDirectories(scratchPath))
		return nullptr;

	// Create export script.
	Ref< IStream > file = FileSystem::getInstance().open(scratchPath + L"/__export__.py", File::FmWrite);
	if (!file)
		return nullptr;

	FileOutputStream os(file, new Utf8Encoding());
	os << L"import bpy" << Endl;
	os << L"bpy.ops.export_scene.fbx(" << Endl;
	os << L"	filepath=\"" << scratchPath << L"/" << intermediate << L"\"," << Endl;
	os << L"	axis_forward=\"Z\"," << Endl;
	os << L"	axis_up=\"Y\"," << Endl;
	os << L"	use_selection=False," << Endl;
	os << L"	global_scale=0.01," << Endl;
	os << L"	use_custom_props=True" << Endl;
	os << L")" << Endl;
	os.close();

	file = nullptr;

	// Execute export script through headless blender process.
	Path blenderPath;
	if (!OS::getInstance().whereIs(L"blender", blenderPath))
		return nullptr;

#if defined(_WIN32)
	std::wstring blender = blenderPath.getPathName();
#else
	std::wstring blender = blenderPath.getPathNameNoVolume();
#endif

	Path filePathAbs = FileSystem::getInstance().getAbsolutePath(filePath);

#if defined(_WIN32)
	std::wstring commandLine = L"\"" + blender + L"\" -b \"" + filePathAbs.getPathName() + L"\" -P " + scratchPath + L"/__export__.py";
#else
	std::wstring commandLine = L"\"" + blender + L"\" -b \"" + filePathAbs.getPathNameNoVolume() + L"\" -P " + scratchPath + L"/__export__.py";
#endif
	Ref< IProcess > process = OS::getInstance().execute(
		commandLine,
		scratchPath,
		nullptr,
		OS::EfMute
	);
	if (!process)
		return nullptr;
	if (!process->wait())
		return nullptr;
	if (process->exitCode() != 0)
		return nullptr;

	// Import intermediate model.
	return ModelFormat::readAny(scratchPath + L"/" + intermediate, filter);
}

bool ModelFormatBlend::write(const Path& filePath, const Model* model) const
{
	return false;
}

	}
}
