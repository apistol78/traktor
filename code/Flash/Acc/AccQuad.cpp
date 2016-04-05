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

const resource::Id< render::Shader > c_idShaderSolid(Guid(L"{2EDC5E1B-562D-9F46-9E3C-474729FB078E}"));
const resource::Id< render::Shader > c_idShaderTextured(Guid(L"{98A59F6A-1D90-144C-B688-4CEF382453F2}"));
const resource::Id< render::Shader > c_idShaderIncrementMask(Guid(L"{16868DF6-A619-5541-83D2-94088A0AC552}"));
const resource::Id< render::Shader > c_idShaderDecrementMask(Guid(L"{D6821007-47BB-D748-9E29-20829ED09C70}"));
const resource::Id< render::Shader > c_idShaderBlit(Guid(L"{34029EA0-112D-D74B-94CA-9C32FF319BB0}"));

#pragma pack(1)
struct Vertex
{
	float pos[2];
};
#pragma pack()

bool s_handleInitialized = false;
render::handle_t s_handleTransform;
render::handle_t s_handleFrameBounds;
render::handle_t s_handleFrameTransform;
render::handle_t s_handleScreenOffsetScale;
render::handle_t s_handleCxFormMul;
render::handle_t s_handleCxFormAdd;
render::handle_t s_handleTexture;
render::handle_t s_handleTextureOffset;

		}

bool AccQuad::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!s_handleInitialized)
	{
		s_handleTransform = render::getParameterHandle(L"Flash_Transform");
		s_handleFrameBounds = render::getParameterHandle(L"Flash_FrameBounds");
		s_handleFrameTransform = render::getParameterHandle(L"Flash_FrameTransform");
		s_handleScreenOffsetScale = render::getParameterHandle(L"Flash_ScreenOffsetScale");
		s_handleCxFormMul = render::getParameterHandle(L"Flash_CxFormMul");
		s_handleCxFormAdd = render::getParameterHandle(L"Flash_CxFormAdd");
		s_handleTexture = render::getParameterHandle(L"Flash_Texture");
		s_handleTextureOffset = render::getParameterHandle(L"Flash_TextureOffset");
		s_handleInitialized = true;
	}

	if (!resourceManager->bind(c_idShaderSolid, m_shaderSolid))
		return false;
	if (!resourceManager->bind(c_idShaderTextured, m_shaderTextured))
		return false;
	if (!resourceManager->bind(c_idShaderIncrementMask, m_shaderIncrementMask))
		return false;
	if (!resourceManager->bind(c_idShaderDecrementMask, m_shaderDecrementMask))
		return false;
	if (!resourceManager->bind(c_idShaderBlit, m_shaderBlit))
		return false;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos)));
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 2 * 3 * sizeof(Vertex), false);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

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
	const Aabb2& bounds,
	const Matrix33& transform,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	const SwfCxTransform& cxform,
	render::ITexture* texture,
	const Vector4& textureOffset,
	bool maskWrite,
	bool maskIncrement,
	uint8_t maskReference
)
{
	Matrix44 m1(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m2(
		bounds.mx.x - bounds.mn.x, 0.0f, bounds.mn.x, 0.0f,
		0.0f, bounds.mx.y - bounds.mn.y, bounds.mn.y, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 m = m1 * m2;

	Ref< render::Shader > shaderSolid, shaderTextured;
	if (!maskWrite)
	{
		shaderSolid = m_shaderSolid;
		shaderTextured = m_shaderTextured;
	}
	else
	{
		if (maskIncrement)
		{
			shaderSolid = m_shaderIncrementMask;
			shaderTextured = m_shaderIncrementMask;
		}
		else
		{
			shaderSolid = m_shaderDecrementMask;
			shaderTextured = m_shaderDecrementMask;
		}
	}

	render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccQuad");
	renderBlock->program = (texture ? shaderTextured : shaderSolid)->getCurrentProgram();
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitive = render::PtTriangleStrip;
	renderBlock->offset = 0;
	renderBlock->count = 2;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setMatrixParameter(s_handleTransform, m);
	renderBlock->programParams->setVectorParameter(s_handleFrameBounds, frameBounds);
	renderBlock->programParams->setVectorParameter(s_handleFrameTransform, frameTransform);
	renderBlock->programParams->setFloatParameter(s_handleScreenOffsetScale, 0.0f);
	renderBlock->programParams->setVectorParameter(s_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
	renderBlock->programParams->setVectorParameter(s_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
	renderBlock->programParams->setStencilReference(maskReference);
	
	if (texture)
	{
		renderBlock->programParams->setTextureParameter(s_handleTexture, texture);
		renderBlock->programParams->setVectorParameter(s_handleTextureOffset, textureOffset);
	}

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(render::RpOverlay, renderBlock);
}

void AccQuad::blit(
	render::RenderContext* renderContext,
	render::ITexture* texture
)
{
	render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccQuad (blit)");
	renderBlock->program = m_shaderBlit->getCurrentProgram();
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitive = render::PtTriangleStrip;
	renderBlock->offset = 0;
	renderBlock->count = 2;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setTextureParameter(s_handleTexture, texture);
	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(render::RpOverlay, renderBlock);
}

	}
}
