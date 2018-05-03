/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Database/ConnectionString.h"
#include "Database/Compact/BlockFile.h"
#include "Database/Compact/CompactContext.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Compact/CompactGroup.h"
#include "Database/Compact/CompactRegistry.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.CompactDatabase", 0, CompactDatabase, IProviderDatabase)

CompactDatabase::CompactDatabase()
:	m_readOnly(false)
,	m_registryHash(0)
{
}

bool CompactDatabase::create(const ConnectionString& connectionString)
{
	if (!connectionString.have(L"fileName"))
	{
		log::error << L"Unable to create compact database; fileName missing." << Endl;
		return false;
	}

	Path fileName = connectionString.get(L"fileName");
	bool flushAlways = parseString< bool >(connectionString.get(L"flushAlways"));

	Ref< BlockFile > blockFile = new BlockFile();
	if (!blockFile->create(fileName, flushAlways))
	{
		log::error << L"Unable to create compact database \"" << fileName.getPathName() << L"\"; missing or corrupt." << Endl;
		return false;
	}

	uint32_t registryBlockId = blockFile->allocBlockId();
	T_FATAL_ASSERT (registryBlockId == 1);

	Ref< CompactRegistry > registry = new CompactRegistry();

	Ref< CompactGroupEntry > rootGroupEntry = registry->createGroupEntry();
	rootGroupEntry->setName(L"#Root");
	registry->setRootGroup(rootGroupEntry);

	m_readOnly = false;
	m_registryHash = DeepHash(registry).get();

	m_context = new CompactContext(blockFile, registry);

	m_rootGroup = new CompactGroup(m_context);
	if (!m_rootGroup->internalCreate(rootGroupEntry))
		return false;

	return true;
}

bool CompactDatabase::open(const ConnectionString& connectionString)
{
	if (!connectionString.have(L"fileName"))
	{
		log::error << L"Unable to open compact database; fileName missing." << Endl;
		return false;
	}

	Path fileName = connectionString.get(L"fileName");
	bool readOnly = parseString< bool >(connectionString.get(L"readOnly"));
	bool flushAlways = parseString< bool >(connectionString.get(L"flushAlways"));

	Ref< BlockFile > blockFile = new BlockFile();
	if (!blockFile->open(fileName, readOnly, flushAlways))
	{
		log::error << L"Unable to open compact database \"" << fileName.getPathName() << L"\"; missing or corrupt." << Endl;
		return false;
	}

	Ref< IStream > registryStream = blockFile->readBlock(1);
	if (!registryStream)
	{
		log::error << L"Unable to open compact database \"" << fileName.getPathName() << L"\"; corrupt registry." << Endl;
		blockFile->close();
		return false;
	}

	Ref< CompactRegistry > registry = BinarySerializer(registryStream).readObject< CompactRegistry >();
	registryStream->close();

	if (!registry)
	{
		log::error << L"Unable to open compact database \"" << fileName.getPathName() << L"\"; corrupt registry." << Endl;
		blockFile->close();
		return false;
	}

	if ((m_readOnly = readOnly) == false)
		m_registryHash = DeepHash(registry).get();

	m_context = new CompactContext(blockFile, registry);

	m_rootGroup = new CompactGroup(m_context);
	if (!m_rootGroup->internalCreate(registry->getRootGroup()))
		return false;

	return true;
}

void CompactDatabase::close()
{
	if (m_context)
	{
		Ref< BlockFile > blockFile = m_context->getBlockFile();
		T_ASSERT (blockFile);

		if (!m_readOnly && DeepHash(m_context->getRegistry()) != m_registryHash)
		{
			Ref< IStream > registryStream = blockFile->writeBlock(1);
			T_ASSERT (registryStream);

			BinarySerializer(registryStream).writeObject(m_context->getRegistry());

			registryStream->close();
		}

		blockFile->close();

		m_context = 0;
	}
}

Ref< IProviderBus > CompactDatabase::getBus()
{
	return 0;
}

Ref< IProviderGroup > CompactDatabase::getRootGroup()
{
	return m_rootGroup;
}

	}
}
