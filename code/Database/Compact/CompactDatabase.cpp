#include "Database/Compact/CompactDatabase.h"
#include "Database/Compact/CompactGroup.h"
#include "Database/Compact/CompactContext.h"
#include "Database/Compact/CompactRegistry.h"
#include "Database/Compact/BlockFile.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.CompactDatabase", CompactDatabase, IProviderDatabase)

bool CompactDatabase::create(const Path& filePath)
{
	Ref< BlockFile > blockFile = gc_new< BlockFile >();
	if (!blockFile->create(filePath))
		return false;

	uint32_t registryBlockId = blockFile->allocBlockId();
	T_ASSERT (registryBlockId == 1);

	Ref< CompactRegistry > registry = gc_new< CompactRegistry >();

	Ref< CompactGroupEntry > rootGroupEntry = registry->createGroupEntry();
	rootGroupEntry->setName(L"#Root");
	registry->setRootGroup(rootGroupEntry);

	m_readOnly = false;
	m_registryHash = DeepHash(registry).get();

	m_context = gc_new< CompactContext >(blockFile, registry);

	m_rootGroup = gc_new< CompactGroup >(m_context);
	if (!m_rootGroup->internalCreate(rootGroupEntry))
		return false;

	return true;
}

bool CompactDatabase::open(const Path& filePath, bool readOnly)
{
	Ref< BlockFile > blockFile = gc_new< BlockFile >();

	if (!blockFile->open(filePath, readOnly))
		return false;

	Ref< Stream > registryStream = blockFile->readBlock(1);
	if (!registryStream)
	{
		blockFile->close();
		return false;
	}

	Ref< CompactRegistry > registry = BinarySerializer(registryStream).readObject< CompactRegistry >();
	registryStream->close();

	if (!registry)
	{
		blockFile->close();
		return false;
	}

	m_readOnly = readOnly;
	m_registryHash = DeepHash(registry).get();

	m_context = gc_new< CompactContext >(blockFile, registry);

	m_rootGroup = gc_new< CompactGroup >(m_context);
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
			Ref< Stream > registryStream = blockFile->writeBlock(1);
			T_ASSERT (registryStream);

			BinarySerializer(registryStream).writeObject(m_context->getRegistry());

			registryStream->close();
		}

		blockFile->close();

		m_context = 0;
	}
}

IProviderBus* CompactDatabase::getBus()
{
	return 0;
}

IProviderGroup* CompactDatabase::getRootGroup()
{
	return m_rootGroup;
}

	}
}
