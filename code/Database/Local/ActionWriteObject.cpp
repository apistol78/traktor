/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/ChunkMemory.h"
#include "Core/Io/ChunkMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Local/ActionWriteObject.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionWriteObject", ActionWriteObject, Action)

ActionWriteObject::ActionWriteObject(const Path& instancePath, const std::wstring& primaryTypeName)
:	m_instancePath(instancePath)
,	m_primaryTypeName(primaryTypeName)
,	m_editObject(false)
,	m_editMeta(false)
{
	m_objectMemory = new ChunkMemory();
}

bool ActionWriteObject::execute(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	m_editObject = fileStore->edit(instanceObjectPath);
	if (!m_editObject)
	{
		log::error << L"Unable to edit instance object, \"" << instanceObjectPath.getPathName() << L"\"." << Endl;
		return false;
	}

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
	{
		log::error << L"Unable to read instance meta data, \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
		return false;
	}

	Ref< IStream > instanceStream = FileSystem::getInstance().open(instanceObjectPath, File::FmWrite);
	if (!instanceStream)
	{
		log::error << L"Unable to open instance object stream, \"" << instanceObjectPath.getPathName() << L"\"." << Endl;
		return false;
	}

	for (size_t offset = 0; offset < m_objectMemory->size(); )
	{
		const auto chunk = m_objectMemory->getChunk(offset);
		if (!chunk.ptr)
			break;

		if (instanceStream->write(chunk.ptr, chunk.size) != chunk.size)
		{
			log::error << L"Unable to write " << (uint32_t)chunk.size << L" byte(s) to file \"" << instanceObjectPath.getPathName() << L"\"." << Endl;
			safeClose(instanceStream);
			FileSystem::getInstance().remove(instanceObjectPath);
			return false;
		}

		offset += chunk.size;
	}

	safeClose(instanceStream);

	if (instanceMeta->getPrimaryType() != m_primaryTypeName)
	{
		m_editMeta = fileStore->edit(instanceMetaPath);
		if (!m_editMeta)
		{
			log::error << L"Unable to edit instance meta, \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
			return false;
		}

		instanceMeta->setPrimaryType(m_primaryTypeName);

		if (!writePhysicalObject(instanceMetaPath, instanceMeta, context.preferBinary()))
		{
			log::error << L"Unable to write instance meta data, \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
			return false;
		}
	}

	return true;
}

bool ActionWriteObject::undo(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (m_editObject)
		fileStore->rollback(instanceObjectPath);
	if (m_editMeta)
		fileStore->rollback(instanceMetaPath);

	m_editObject =
	m_editMeta = false;

	return true;
}

void ActionWriteObject::clean(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (m_editObject)
		fileStore->clean(instanceObjectPath);
	if (m_editMeta)
		fileStore->clean(instanceMetaPath);
}

bool ActionWriteObject::redundant(const Action* action) const
{
	const ActionWriteObject* actionWriteObject = dynamic_type_cast< const ActionWriteObject* >(action);
	if (actionWriteObject)
		return m_instancePath == actionWriteObject->m_instancePath;
	else
		return false;
}

Ref< IStream > ActionWriteObject::getWriteStream() const
{
	return new ChunkMemoryStream(m_objectMemory, false, true);
}

Ref< IStream > ActionWriteObject::getReadStream() const
{
	return new ChunkMemoryStream(m_objectMemory, true, false);
}

	}
}
