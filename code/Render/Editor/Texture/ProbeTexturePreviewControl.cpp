/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Random.h"
#include "Core/Misc/SafeDestroy.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IEditor.h"
#include "Render/ICubeTexture.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Editor/Texture/CubeMap.h"
#include "Render/Editor/Texture/ProbeTexturePreviewControl.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const resource::Id< Shader > c_idProbeTexturePreview(Guid(L"{2F69EAE9-FA20-3244-9B8C-C803E538C19F}"));

#pragma pack(1)
struct Vertex
{
	float xyz[3];
};
#pragma pack()

const Vertex c_cubeVertices[] =
{
	{ -1, -1, 1 },
	{ -1, -1, -1 },
	{ -1, 1, -1 },
	{ -1, 1, 1 },
	{ 1, -1, 1 },
	{ 1, -1, -1 },
	{ 1, 1, -1 },
	{ 1, 1, 1 },
};

const uint16_t c_cubeIndices[] =
{
	3, 1, 0,
	3, 2, 1,
	1, 4, 0,
	1, 5, 4,
	2, 5, 1,
	2, 6, 5,
	2, 3, 7,
	6, 2, 7,
	3, 0, 4,
	7, 3, 4,
	5, 7, 4,
	6, 7, 5,
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProbeTexturePreviewControl", ProbeTexturePreviewControl, ui::Widget)

ProbeTexturePreviewControl::ProbeTexturePreviewControl(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool ProbeTexturePreviewControl::create(ui::Widget* parent)
{
	if (!Widget::create(parent, ui::WsNone))
		return false;

	m_renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!m_renderSystem)
		return false;

	m_resourceManager = new resource::ResourceManager(m_editor->getOutputDatabase(), true);
	m_resourceManager->addFactory(new ShaderFactory(m_renderSystem));

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 24;
	desc.stencilBits = 0;
	desc.multiSample = 0;
	desc.waitVBlanks = 0;
	desc.syswin = getIWidget()->getSystemWindow();

	m_renderView = m_renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	AlignedVector< render::VertexElement > vertexDecl(1);
	vertexDecl[0] = render::VertexElement(render::DuPosition, render::DtFloat3, 0);

	m_vertexBuffer = m_renderSystem->createVertexBuffer(vertexDecl, sizeof(c_cubeVertices), false);
	std::memcpy(m_vertexBuffer->lock(), c_cubeVertices, sizeof(c_cubeVertices));
	m_vertexBuffer->unlock();

	m_indexBuffer = m_renderSystem->createIndexBuffer(render::ItUInt16, sizeof(c_cubeIndices), false);
	std::memcpy(m_indexBuffer->lock(), c_cubeIndices, sizeof(c_cubeIndices));
	m_indexBuffer->unlock();

	if (!m_resourceManager->bind(c_idProbeTexturePreview, m_shader))
		return false;

	addEventHandler< ui::SizeEvent >(this, &ProbeTexturePreviewControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &ProbeTexturePreviewControl::eventPaint);

	return true;
}

void ProbeTexturePreviewControl::destroy()
{
	safeDestroy(m_resourceManager);
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
	safeClose(m_renderView);
	Widget::destroy();
}

ui::Size ProbeTexturePreviewControl::getPreferedSize() const
{
	return ui::Size(
		ui::dpi96(256),
		ui::dpi96(256)
	);
}

void ProbeTexturePreviewControl::setCubeMap(const CubeMap* cubeMap, float filterAngle)
{
	Ref< CubeMap > cubeOutput = new CubeMap(cubeMap->getSize(), drawing::PixelFormat::getARGBF32());

	// Calculate filter radius in pixel space.
	int32_t filterRadius = int32_t(std::sin(filterAngle) * cubeMap->getSize());
	if (filterRadius <= 0)
		filterRadius = 1;
	else if (filterRadius > 16) //m_maxFilterRadius)
		filterRadius = 16; //m_maxFilterRadius;

	// Create output probe by filtering input.
	Random random;
	for (int32_t side = 0; side < 6; ++side)
	{
		for (int32_t y = 0; y < cubeMap->getSize(); ++y)
		{
			for (int32_t x = 0; x < cubeMap->getSize(); ++x)
			{
				Color4f accum(0.0f, 0.0f, 0.0f, 0.0f);
				float accumWeight = 0.0f;

				for (int32_t dky = -filterRadius; dky <= filterRadius; ++dky)
				{
					float fdky = dky / float(filterRadius);
					for (int32_t dkx = -filterRadius; dkx <= filterRadius; ++dkx)
					{
						float fdkx = dkx / float(filterRadius);
						float f = 1.0f - std::sqrt(fdkx * fdkx + fdky * fdky);
						if (f <= 0.0f)
							continue;

						Vector4 d = cubeMap->getDirection(side, x + dkx, y + dky);

						int32_t sampleSide, sampleX, sampleY;
						cubeMap->getPosition(d, sampleSide, sampleX, sampleY);

						Color4f sample;
						cubeMap->getSide(sampleSide)->getPixelUnsafe(sampleX, sampleY, sample);

						accum += sample * Scalar(f);
						accumWeight += f;
					}
				}
				accum /= Scalar(accumWeight);

				cubeOutput->getSide(side)->setPixelUnsafe(x, y, accum);
			}
		}
	}

	CubeTextureCreateDesc ctcd;

	ctcd.side = cubeMap->getSize();
	ctcd.mipCount = 1;
	ctcd.format = TfR8G8B8A8;
	ctcd.sRGB = false;
	ctcd.immutable = true;

	Ref< drawing::Image > side[6];
	for (int32_t i = 0; i < 6; ++i)
	{
		side[i] = cubeOutput->getSide(i)->clone();
		side[i]->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());

		ctcd.initialData[i].data = side[i]->getData();
		ctcd.initialData[i].pitch = getTextureRowPitch(ctcd.format, ctcd.side, 0);
		ctcd.initialData[i].slicePitch = getTextureMipPitch(ctcd.format, ctcd.side, ctcd.side, 0);
	}

	safeDestroy(m_texture);

	m_texture = m_renderSystem->createCubeTexture(ctcd);
}

void ProbeTexturePreviewControl::eventSize(ui::SizeEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = event->getSize();

	m_renderView->reset(sz.cx, sz.cy);
	m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void ProbeTexturePreviewControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView || !m_renderView->begin(render::EtCyclop))
		return;

	Color4f clearColor(0.1f, 0.1f, 0.1f, 0.0f);

	m_renderView->clear(
		render::CfColor | render::CfDepth,
		&clearColor,
		1.0f,
		128
	);

	ui::Size sz = getInnerRect().getSize();
	float aspect = float(sz.cx) / sz.cy;

	m_shader->setMatrixParameter(L"World", translate(0.0f, 0.0f, 5.0f) * rotateY(deg2rad(45.0f)) * rotateX(deg2rad(45.0f)));
	m_shader->setMatrixParameter(L"Projection", perspectiveLh(deg2rad(80.0f), aspect, 0.1f, 10.0f));
	m_shader->setTextureParameter(L"Texture", m_texture);
	m_shader->draw(m_renderView, m_vertexBuffer, m_indexBuffer, render::Primitives(render::PtTriangles, 0, 12, 0, 7));

	m_renderView->end();
	m_renderView->present();

	event->consume();
}

	}
}
