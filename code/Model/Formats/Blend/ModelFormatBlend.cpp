/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Model/Model.h"
#include "Model/Formats/Blend/ModelFormatBlend.h"

namespace traktor::model
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
	const std::wstring scratchPath = OS::getInstance().getWritableFolderPath() + L"/Traktor/Blender/" + threadFolder;
	if (!FileSystem::getInstance().makeAllDirectories(scratchPath))
	{
		log::error << L"Unable to create scratch path \"" << scratchPath << L"\"." << Endl;
		return nullptr;
	}

	// Create export script.
	Ref< IStream > file = FileSystem::getInstance().open(scratchPath + L"/__export__.py", File::FmWrite);
	if (!file)
	{
		log::error << L"Unable to create __export__.py" << Endl;
		return nullptr;
	}

	FileOutputStream os(file, new Utf8Encoding());
	os << L"import bpy" << Endl;
	os << L"bpy.ops.export_scene.fbx(" << Endl;
	os << L"	filepath=\"" << scratchPath << L"/" << intermediate << L"\"," << Endl;
	os << L"	axis_forward=\"Z\"," << Endl;
	os << L"	axis_up=\"Y\"," << Endl;
	os << L"	use_selection=False," << Endl;
	os << L"	use_visible=True," << Endl;
	os << L"	global_scale=0.01," << Endl;
	os << L"	use_custom_props=True," << Endl;
	os << L"	use_mesh_modifiers=True," << Endl;
	os << L"	add_leaf_bones=True," << Endl;
	os << L"	primary_bone_axis=\"Y\"," << Endl;
	os << L"	secondary_bone_axis=\"-X\"," << Endl;
	os << L"	bake_anim=True" << Endl;
	os << L")" << Endl;
	os.close();

	file = nullptr;

	// Execute export script through headless blender process.
	Path blenderPath;
	if (!OS::getInstance().whereIs(L"blender", blenderPath))
	{
		// No path found; try to run executable without path in case system knowns
		// something we don't.
		blenderPath = L"blender.exe";
	}

	const Path filePathAbs = FileSystem::getInstance().getAbsolutePath(filePath);
	const std::wstring commandLine = L"\"" + blenderPath.getPathNameOS() + L"\" -b \"" + filePathAbs.getPathNameOS() + L"\" -P " + scratchPath + L"/__export__.py";

	Ref< IProcess > process = OS::getInstance().execute(
		commandLine,
		scratchPath,
		nullptr,
		OS::EfMute
	);
	if (!process)
	{
		log::error << L"Unable to spawn \"" << commandLine << L"\"." << Endl;
		return nullptr;
	}
	if (!process->wait())
	{
		log::error << L"Unknown error when waiting for process to terminate." << Endl;
		return nullptr;
	}
	if (process->exitCode() != 0)
	{
		log::error << L"Blender terminated with error code " << process->exitCode() << Endl;
		return nullptr;
	}

	// Import intermediate model.
	Ref< Model > model = ModelFormat::readAny(scratchPath + L"/" + intermediate, filter);
	if (!model)
	{
		log::error << L"Unable to read intermediate export, \"" << scratchPath << L"/" << intermediate << L"\", from blender." << Endl;
		return nullptr;
	}

	// Remove unused materials.
	AlignedVector< int32_t > materialsToRemove(model->getMaterialCount());
	for (uint32_t i = 0; i < model->getMaterialCount(); ++i)
		materialsToRemove[i] = i;
	for (const auto& polygon : model->getPolygons())
	{
		if (polygon.getMaterial() != model::c_InvalidIndex)
			materialsToRemove[polygon.getMaterial()] = c_InvalidIndex;
	}
	auto it = std::remove(materialsToRemove.begin(), materialsToRemove.end(), c_InvalidIndex);
	materialsToRemove.erase(it, materialsToRemove.end());
	for (auto it = materialsToRemove.rbegin(); it != materialsToRemove.rend(); ++it)
		model->removeMaterial(*it);

	return model;
}

bool ModelFormatBlend::write(const Path& filePath, const Model* model) const
{
	return false;
}

}
