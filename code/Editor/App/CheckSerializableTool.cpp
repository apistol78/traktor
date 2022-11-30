/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstdlib>
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmArray.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/App/CheckSerializableTool.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/BackgroundWorkerDialog.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.CheckSerializableTool", 0, CheckSerializableTool, IEditorTool)

std::wstring CheckSerializableTool::getDescription() const
{
	return i18n::Text(L"EDITOR_CHECK_SERIALIZABLE_CLASSES");
}

Ref< ui::IBitmap > CheckSerializableTool::getIcon() const
{
	return nullptr;
}

bool CheckSerializableTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool CheckSerializableTool::launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param)
{
	ui::BackgroundWorkerDialog dialog;
	dialog.create(parent, i18n::Text(L"EDITOR_CHECK_SERIALIZABLE_CLASSES"), L"Checking serializable classes...", false);
	
	Thread* checkThread = ThreadManager::getInstance().create(
		[]() {
			AutoArrayPtr< uint8_t, AllocFreeAlign > memory((uint8_t*)Alloc::acquireAlign(1024 * 1024, 16, T_FILE_LINE));
			for (auto type : type_of< ISerializable >().findAllOf(false))
			{
				if (!type->isInstantiable())
					continue;

				Ref< ISerializable > object = checked_type_cast< ISerializable* >(type->createInstance());
				if (!object)
				{
					log::error << L"Class \"" << type->getName() << L"\" not valid; unable to instanciate object." << Endl;
					continue;
				}

				Ref< Reflection > rf = Reflection::create(object);

				// Add default item to array members so we also check compound members.
				RefArray< ReflectionMember > arrayMembers;
				rf->findMembers(RfpMemberType(type_of< RfmArray >()), arrayMembers);
				for (auto arrayMember : arrayMembers)
					checked_type_cast< RfmArray* >(arrayMember)->insertDefault();

				// Randomize memory.
				for (uint32_t j = 0; j < 1024 * 1024; ++j)
					memory[j] = std::rand() & 255;

				const ISerializable* object0 = checked_type_cast< ISerializable* >(rf->clone(memory.ptr()));
				if (!object0)
					continue;

				const uint32_t hash0 = DeepHash(object0).get();

				// Randomize memory again.
				for (uint32_t j = 0; j < 1024 * 1024; ++j)
					memory[j] = std::rand() & 255;

				const ISerializable* object1 = checked_type_cast< ISerializable* >(rf->clone(memory.ptr()));
				if (!object1)
					continue;

				const uint32_t hash1 = DeepHash(object1).get();

				// If all members are properly initialized then the hashes must match.
				if (hash0 != hash1)
				{
					log::error << L"Class \"" << type->getName() << L"\" not valid; uninitialized members not allowed." << Endl;
					continue;
				}

				log::info << L"Class \"" << type->getName() << L"\" ok." << Endl;
			}
		},
		L"Serializable Checker"
	);

	if (checkThread)
	{
		checkThread->start();
		dialog.execute(checkThread, nullptr);
		ThreadManager::getInstance().destroy(checkThread);
	}

	dialog.destroy();
	return true;
}

	}
}
