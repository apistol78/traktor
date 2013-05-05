#include <DetourNavMesh.h>
#include "Ai/NavMesh.h"
#include "Ai/NavMeshFactory.h"
#include "Ai/NavMeshResource.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Database/Database.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshFactory", NavMeshFactory, resource::IResourceFactory)

NavMeshFactory::NavMeshFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet NavMeshFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< NavMeshResource >());
	return typeSet;
}

const TypeInfoSet NavMeshFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< NavMesh >());
	return typeSet;
}

bool NavMeshFactory::isCacheable() const
{
	return true;
}

Ref< Object > NavMeshFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< NavMeshResource > resource = instance->getObject< NavMeshResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	uint8_t version;
	Reader(stream) >> version;
	if (version != 1)
		return 0;

	int32_t navDataSize;
	Reader(stream) >> navDataSize;
	if (navDataSize <= 0)
		return 0;

	uint8_t* navData = (uint8_t*)dtAlloc(navDataSize, DT_ALLOC_PERM);
	if (stream->read(navData, navDataSize) != navDataSize)
		return 0;

	stream->close();
	stream = 0;

	dtNavMesh* navMesh = dtAllocNavMesh();
	if (!navMesh)
		return 0;

	dtStatus status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status))
		return 0;

	Ref< NavMesh > outputNavMesh = new NavMesh();
	outputNavMesh->m_navMesh = navMesh;

	return outputNavMesh;
}

	}
}
