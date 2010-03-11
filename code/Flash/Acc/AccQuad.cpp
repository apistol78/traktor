#include "Core/Math/Matrix44.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/SwfTypes.h"
#include "Flash/Acc/AccQuad.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const Guid c_guidShaderTextured(L"{10426D17-CF0A-4849-A207-24F101A78459}");

struct Vertex
{
	float pos[2];
};

bool s_handleInitialized = false;
render::handle_t s_handleFrameSize;
render::handle_t s_handleTransform;
render::handle_t s_handleCxFormMul;
render::handle_t s_handleCxFormAdd;
render::handle_t s_handleTexture;
render::handle_t s_handleViewSize;
render::handle_t s_handleScaleX;
render::handle_t s_handleStepSize;

		}

bool AccQuad::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!s_handleInitialized)
	{
		s_handleFrameSize = render::getParameterHandle(L"FrameSize");
		s_handleTransform = render::getParameterHandle(L"Transform");
		s_handleCxFormMul = render::getParameterHandle(L"CxFormMul");
		s_handleCxFormAdd = render::getParameterHandle(L"CxFormAdd");
		s_handleTexture = render::getParameterHandle(L"Texture");
		s_handleViewSize = render::getParameterHandle(L"ViewSize");
		s_handleScaleX = render::getParameterHandle(L"ScaleX");
		s_handleStepSize = render::getParameterHandle(L"StepSize");
		s_handleInitialized = true;
	}

	m_shaderTextured = c_guidShaderTextured;
	if (!resourceManager->bind(m_shaderTextured))
		return false;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos)));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 2 * 3 * sizeof(Vertex), false);
	T_ASSERT (m_vertexBuffer);

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	T_ASSERT (vertex);

	vertex->pos[0] = 0.0f; vertex->pos[1] = 0.0f; ++vertex;
	vertex->pos[0] = 1.0f; vertex->pos[1] = 0.0f; ++vertex;
	vertex->pos[0] = 0.0f; vertex->pos[1] = 1.0f; ++vertex;
	vertex->pos[0] = 1.0f; vertex->pos[1] = 1.0f; ++vertex;

	m_vertexBuffer->unlock();

	return true;
}

void AccQuad::destroy()
{
	safeDestroy(m_vertexBuffer);
}

void AccQuad::render(
	render::RenderContext* renderContext,
	const Vector4& frameSize,
	const Vector4& viewSize,
	float scaleX,
	const Matrix33& transform,
	const SwfRect& bounds,
	const SwfCxTransform& cxform,
	render::ITexture* texture
)
{
	Matrix44 m1(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m2(
		bounds.max.x - bounds.min.x, 0.0f, bounds.min.x, 0.0f,
		0.0f, bounds.max.y - bounds.min.y, bounds.min.y, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m = m1 * m2;

	Vector4 stepSize(
		(frameSize.z() - frameSize.x()) * viewSize.z() * Scalar(0.5f),
		(frameSize.w() - frameSize.y()) * viewSize.w() * Scalar(0.5f),
		0.0f,
		0.0f
	);
	stepSize = m.inverse() * stepSize;

	render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccQuad");
	renderBlock->shader = m_shaderTextured;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitive = render::PtTriangleStrip;
	renderBlock->offset = 0;
	renderBlock->count = 2;

	renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
	renderBlock->shaderParams->beginParameters(renderContext);
	renderBlock->shaderParams->setMatrixParameter(s_handleTransform, m);
	renderBlock->shaderParams->setVectorParameter(s_handleFrameSize, frameSize);
	renderBlock->shaderParams->setVectorParameter(s_handleViewSize, viewSize);
	renderBlock->shaderParams->setFloatParameter(s_handleScaleX, scaleX);
	renderBlock->shaderParams->setVectorParameter(s_handleStepSize, stepSize);
	renderBlock->shaderParams->setVectorParameter(s_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
	renderBlock->shaderParams->setVectorParameter(s_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
	renderBlock->shaderParams->setTextureParameter(s_handleTexture, texture);
	renderBlock->shaderParams->endParameters(renderContext);

	renderContext->draw(render::RfOverlay, renderBlock);
}

	}
}
