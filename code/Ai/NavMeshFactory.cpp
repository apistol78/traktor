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

const TypeInfoSet NavMeshFactory::getResourceTypes() const
{
	return makeTypeInfoSet< NavMeshResource >();
}

const TypeInfoSet NavMeshFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< NavMesh >();
}

bool NavMeshFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > NavMeshFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< NavMesh > outputNavMesh = new NavMesh();

	Ref< NavMeshResource > resource = instance->getObject< NavMeshResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Reader r(stream);

	uint8_t version;
	r >> version;
	if (version != 2)
		return 0;

	int32_t navDataSize;
	r >> navDataSize;
	if (navDataSize <= 0)
		return 0;

	uint8_t* navData = (uint8_t*)dtAlloc(navDataSize, DT_ALLOC_PERM);
	if (stream->read(navData, navDataSize) != navDataSize)
		return 0;

	bool haveGeometry;
	r >> haveGeometry;

	if (haveGeometry)
	{
		uint32_t numVertices;
		r >> numVertices;

		outputNavMesh->m_navMeshVertices.resize(numVertices);
		for (uint32_t i = 0; i < numVertices; ++i)
		{
			float x, y, z;
			r >> x;
			r >> y;
			r >> z;
			outputNavMesh->m_navMeshVertices[i].set(x, y, z, 1.0f);
		}

		uint32_t numPolygons;
		r >> numPolygons;

		outputNavMesh->m_navMeshPolygons.reserve(numPolygons * 4);
		for (uint32_t i = 0; i < numPolygons; ++i)
		{
			uint8_t numPolygonVertices;
			r >> numPolygonVertices;

			outputNavMesh->m_navMeshPolygons.push_back(numPolygonVertices);

			for (uint32_t j = 0; j < numPolygonVertices; ++j)
			{
				uint16_t polygonIndex;
				r >> polygonIndex;

				outputNavMesh->m_navMeshPolygons.push_back(polygonIndex);
			}
		}
	}

	stream->close();
	stream = 0;

	dtNavMesh* navMesh = dtAllocNavMesh();
	if (!navMesh)
		return 0;

	dtStatus status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status))
		return 0;

	outputNavMesh->m_navMesh = navMesh;

	return outputNavMesh;
}

	}
}
