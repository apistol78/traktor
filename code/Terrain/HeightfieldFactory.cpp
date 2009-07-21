#include <limits>
#include "Terrain/HeightfieldFactory.h"
#include "Terrain/HeightfieldResource.h"
#include "Terrain/Heightfield.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Reader.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Half.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/JobManager.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const int c_skipHeightTexture = 4;
const int c_skipNormalTexture = 2;

uint32_t nearestLod2(uint32_t value)
{
	for (int i = 31; i >= 0; --i)
	{
		uint32_t lg2 = 1UL << i;
		if (lg2 & value)
			return lg2;
	}
	return 1;
}

void createHeightTextureJob(
	const Heightfield* heightfield,
	uint32_t size,
	uint32_t dim,
	half_t* data,
	uint32_t from,
	uint32_t to
)
{
	for (uint32_t y = from; y < to; ++y)
	{
		float hy = float(y * size) / dim;

		for (uint32_t x = 0; x < dim; ++x)
		{
			float hx = float(x * size) / dim;

			float h = heightfield->getGridHeight(hx, hy);
			data[x + y * dim] = floatToHalf(h);

#if defined(_XBOX)
			swap8in16(data[x + y * dim]);
#endif
		}
	}
}

render::ITexture* createHeightTexture(render::IRenderSystem* renderSystem, const Heightfield* heightfield)
{
	uint32_t size = heightfield->getResource().getSize();
	T_ASSERT (size > 0);

	uint32_t dim = nearestLod2(size);
	dim /= c_skipHeightTexture;
	T_ASSERT (dim > 0);

	AutoArrayPtr< half_t > data = new half_t [dim * dim];
	T_ASSERT (data.ptr());

	Job jobs[] =
	{
		makeStaticFunctor< const Heightfield*, uint32_t, uint32_t, half_t*, uint32_t, uint32_t >(createHeightTextureJob, heightfield, size, dim, data.ptr(), (dim * 0) / 4, (dim * 1) / 4),
		makeStaticFunctor< const Heightfield*, uint32_t, uint32_t, half_t*, uint32_t, uint32_t >(createHeightTextureJob, heightfield, size, dim, data.ptr(), (dim * 1) / 4, (dim * 2) / 4),
		makeStaticFunctor< const Heightfield*, uint32_t, uint32_t, half_t*, uint32_t, uint32_t >(createHeightTextureJob, heightfield, size, dim, data.ptr(), (dim * 2) / 4, (dim * 3) / 4),
		makeStaticFunctor< const Heightfield*, uint32_t, uint32_t, half_t*, uint32_t, uint32_t >(createHeightTextureJob, heightfield, size, dim, data.ptr(), (dim * 3) / 4, (dim * 4) / 4)
	};

	JobManager::getInstance().fork(jobs, sizeof_array(jobs));

	render::SimpleTextureCreateDesc desc;
	desc.width = dim;
	desc.height = dim;
	desc.mipCount = 1;
	desc.format = render::TfR16F;
	desc.immutable = true;
	desc.initialData[0].pitch = dim * sizeof(half_t);
	desc.initialData[0].data = data.ptr();

	return renderSystem->createSimpleTexture(desc);
}

void createNormalTextureJob(
	const Heightfield* heightfield,
	uint32_t size,
	uint32_t dim,
	uint8_t* data,
	uint32_t from,
	uint32_t to
)
{
	const float c_scaleHeight = 100.0f;
	float s = 1.0f / size;

	for (uint32_t y = from; y < to; ++y)
	{
		float hy = float(y * size) / dim;

		for (uint32_t x = 0; x < dim; ++x)
		{
			float hx = float(x * size) / dim;

			float h = heightfield->getGridHeight(hx, hy);
			float h1 = heightfield->getGridHeight(hx + s, hy);
			float h2 = heightfield->getGridHeight(hx, hy + s);

			Vector4 normal = cross(
				Vector4(0.0f, (h - h1) * c_scaleHeight, s),
				Vector4(s, (h - h2) * c_scaleHeight, 0.0f)
			).normalized();

			uint8_t* p = &data[(x + y * dim) * 4];
			p[0] = uint8_t((normal.z() * 0.5f + 0.5f) * 255);
			p[1] = uint8_t((normal.y() * 0.5f + 0.5f) * 255);
			p[2] = uint8_t((normal.x() * 0.5f + 0.5f) * 255);
			p[3] = 0;
		}
	}
}

render::ITexture* createNormalTexture(render::IRenderSystem* renderSystem, const Heightfield* heightfield)
{
	uint32_t size = heightfield->getResource().getSize();
	T_ASSERT (size > 0);

	uint32_t dim = nearestLod2(size);
	dim /= c_skipNormalTexture;
	T_ASSERT (dim > 0);

	AutoArrayPtr< uint8_t > data = new uint8_t [dim * dim * 4];
	T_ASSERT (data.ptr());

	Job jobs[] =
	{
		makeStaticFunctor< const Heightfield*, uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t >(createNormalTextureJob, heightfield, size, dim, data.ptr(), (dim * 0) / 4, (dim * 1) / 4),
		makeStaticFunctor< const Heightfield*, uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t >(createNormalTextureJob, heightfield, size, dim, data.ptr(), (dim * 1) / 4, (dim * 2) / 4),
		makeStaticFunctor< const Heightfield*, uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t >(createNormalTextureJob, heightfield, size, dim, data.ptr(), (dim * 2) / 4, (dim * 3) / 4),
		makeStaticFunctor< const Heightfield*, uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t >(createNormalTextureJob, heightfield, size, dim, data.ptr(), (dim * 3) / 4, (dim * 4) / 4)
	};

	JobManager::getInstance().fork(jobs, sizeof_array(jobs));

	render::SimpleTextureCreateDesc desc;
	desc.width = dim;
	desc.height = dim;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].pitch = dim * 4 * sizeof(uint8_t);
	desc.initialData[0].data = data.ptr();

	return renderSystem->createSimpleTexture(desc);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.HeightfieldFactory", HeightfieldFactory, resource::IResourceFactory)

HeightfieldFactory::HeightfieldFactory(db::Database* database, render::IRenderSystem* renderSystem)
:	m_database(database)
,	m_renderSystem(renderSystem)
{
}

const TypeSet HeightfieldFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Heightfield >());
	return typeSet;
}

bool HeightfieldFactory::isCacheable() const
{
	return true;
}

Object* HeightfieldFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
		return 0;

	Ref< HeightfieldResource > resource = instance->getObject< HeightfieldResource >();
	if (!resource)
		return 0;

	Ref< Stream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	// Allocate heightfield.
	uint32_t size = resource->getSize();
	Ref< Heightfield > heightfield = gc_new< Heightfield >(cref(*resource));

	// Read heights.
	Heightfield::height_t* heights = const_cast< Heightfield::height_t* >(heightfield->getHeights());
	T_ASSERT_M (heights, L"No heights in heightfield");
	Reader(stream).read(heights, size * size, sizeof(Heightfield::height_t));
	stream->close();

	// Create heightfield texture.
	heightfield->setHeightTexture(createHeightTexture(m_renderSystem, heightfield));
	heightfield->setNormalTexture(createNormalTexture(m_renderSystem, heightfield));

	return heightfield;
}

	}
}
