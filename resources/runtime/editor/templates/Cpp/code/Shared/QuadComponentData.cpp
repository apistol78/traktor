#include <Core/Serialization/ISerializer.h>
#include <Render/VertexBuffer.h>
#include <Render/VertexElement.h>
#include <Resource/Member.h>
#include "Shared/QuadComponent.h"
#include "Shared/QuadComponentData.h"

using namespace traktor;

T_IMPLEMENT_RTTI_EDIT_CLASS(L"QuadComponentData", 0, QuadComponentData, world::IEntityComponentData)

Ref< QuadComponent > QuadComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	// Bind external shader from pre-built resources.
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	// Setup vertex declaration.
	std::vector< render::VertexElement > vertexElements(1);
	vertexElements[0] = render::VertexElement(render::DuPosition, render::DtFloat2, 0, 0);

	// Create vertex buffer.
	Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 4 * 2 * sizeof(float), false);
	if (!vertexBuffer)
		return 0;

	// Lock and fill vertex buffer.
	float* p = static_cast< float* >(vertexBuffer->lock());
	*p++ = -1.0f; *p++ = -1.0f;
	*p++ =  1.0f; *p++ = -1.0f;
	*p++ = -1.0f; *p++ =  1.0f;
	*p++ =  1.0f; *p++ =  1.0f;
	vertexBuffer->unlock();

	return new QuadComponent(shader, vertexBuffer);
}

void QuadComponentData::serialize(traktor::ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
}
