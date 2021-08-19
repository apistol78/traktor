#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/SystemMeshFactory.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class InternalBuffer : public Buffer
{
public:
	explicit InternalBuffer(uint32_t bufferSize)
	:	Buffer(bufferSize)
	,	m_data(bufferSize)
	{
	}

	virtual void destroy() override final {}
	virtual void* lock() override final { return &m_data[0]; }
	virtual void unlock() override final {}
	virtual const IBufferView* getBufferView() const override final { return nullptr; }

private:
	AlignedVector< uint8_t > m_data;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SystemMeshFactory", SystemMeshFactory, MeshFactory)

Ref< Mesh > SystemMeshFactory::createMesh(
	const AlignedVector< VertexElement >& vertexElements,
	uint32_t vertexBufferSize,
	IndexType indexType,
	uint32_t indexBufferSize
) const
{
	Ref< Buffer > vertexBuffer;
	Ref< Buffer > indexBuffer;

	if (vertexBufferSize > 0)
		vertexBuffer = new InternalBuffer(vertexBufferSize);

	if (indexBufferSize > 0)
		indexBuffer = new InternalBuffer(indexBufferSize);

	Ref< Mesh > mesh = new Mesh();
	mesh->setVertexElements(vertexElements);
	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexType(indexType);
	mesh->setIndexBuffer(indexBuffer);
	return mesh;
}

	}
}
