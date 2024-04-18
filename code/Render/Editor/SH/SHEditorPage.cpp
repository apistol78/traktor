/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Misc/ObjectStore.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Render/SH/SHUtilities.h"
#include "Render/Editor/SH/DirectionalLight.h"
#include "Render/Editor/SH/RenderControl.h"
#include "Render/Editor/SH/RenderControlEvent.h"
#include "Render/Editor/SH/RayLeighPhaseFunction.h"
#include "Render/Editor/SH/SHEditorPage.h"
#include "Ui/Container.h"
#include "Ui/Splitter.h"
#include "Ui/TableLayout.h"

namespace traktor::render
{
	namespace
	{

RandomGeometry s_random;

Vector4 polarToCartesian(float phi, float theta)
{
	return Vector4(
		cos(theta) * sin(phi),
		cos(phi),
		sin(theta) * sin(phi)
	);
}

// Evaluate point directly using source function.
Vector4 Pfs(const SHFunction* function, float phi, float theta)
{
	const Vector4 p = polarToCartesian(phi, theta);
	return p * function->evaluate(phi, theta, p);
}

// Evaluate point using SH.
Vector4 Psh(const SHCoeffs& coefficients, float phi, float theta)
{
	const Vector4 p = polarToCartesian(phi, theta);
	return p * coefficients.evaluate3(phi, theta);
}

SHMatrix concate(const SHMatrix& m1, const SHMatrix& m2)
{
	const int dim = m1.getRows();

	SHMatrix m(dim, dim);
	for (int c = 0; c < dim; ++c)
	{
		for (int r = 0; r < dim; ++r)
		{
			m.w(r, c) = 0.0f;
			for (int i = 0; i < dim; ++i)
				m.w(r, c) += m1.r(r, i) * m2.r(i, c);
		}
	}

	return m;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SHEditorPage", SHEditorPage, editor::IEditorPage)

SHEditorPage::SHEditorPage(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool SHEditorPage::create(ui::Container* parent)
{
	IRenderSystem* renderSystem = m_editor->getObjectStore()->get< IRenderSystem >();
	if (!renderSystem)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_container = new ui::Container();
	m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0_ut, 0_ut));

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(m_container, true, 50_ut, true);

	for (uint32_t i = 0; i < 2; ++i)
	{
		m_renderControls[i] = new RenderControl();
		if (!m_renderControls[i]->create(splitter, renderSystem, database))
			return false;
	}

	m_engine = new SHEngine(3);
	m_engine->generateSamplePoints(20000);

	m_lightFunction = new DirectionalLight(Vector4(0.0f, 1.0f, 0.0f));
	m_engine->generateCoefficients(m_lightFunction, true, m_lightCoefficients);

	//m_phaseFunction = new RayLeighPhaseFunction();
	//m_engine->generateCoefficients(m_phaseFunction, m_phaseCoefficients);

	// m_phaseTransfer = m_engine->generateTransferMatrix(m_phaseFunction);

	m_renderControls[0]->addEventHandler< RenderControlEvent >(this, &SHEditorPage::eventRender1);
	m_renderControls[1]->addEventHandler< RenderControlEvent >(this, &SHEditorPage::eventRender2);

	return true;
}

void SHEditorPage::destroy()
{
	if (m_container)
	{
		m_container->destroy();
		m_container = nullptr;
	}
}

bool SHEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool SHEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"SHEditor.Editor.Randomize")
	{
		m_lightFunction = new DirectionalLight(s_random.nextUnit());
		m_engine->generateCoefficients(m_lightFunction, true, m_lightCoefficients);
		m_renderControls[0]->update();
		m_renderControls[1]->update();
		return true;
	}
	return false;
}

void SHEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void SHEditorPage::eventRender1(RenderControlEvent* event)
{
	PrimitiveRenderer* primitiveRenderer = event->getPrimitiveRenderer();

	const Color4ub colorGrid(0, 0, 0, 120);
	const float unit = 0.25f;

	for (int32_t x = -4; x <= 4; ++x)
	{
		float fx = float(x * unit);
		primitiveRenderer->drawLine(
			Vector4(fx, 0.0f, -4.0f * unit, 1.0f),
			Vector4(fx, 0.0f, 4.0f * unit, 1.0f),
			0.0f,
			colorGrid
		);
		primitiveRenderer->drawLine(
			Vector4(-4.0f * unit, 0.0f, fx, 1.0f),
			Vector4(4.0f * unit, 0.0f, fx, 1.0f),
			0.0f,
			colorGrid
		);
	}

	primitiveRenderer->drawWireFrame(Matrix44::identity(), 1.0f);

	const int32_t azimuthSteps = 40;
	const int32_t zenithSteps = 40;

	for (int32_t i = 0; i < azimuthSteps; ++i)
	{
		for (int32_t j = 0; j < zenithSteps; ++j)
		{
			//Vector4 v[] =
			//{
			//	Psh(
			//		m_engine,
			//		coeffs,
			//		PI * i / float(azimuthSteps),
			//		2.0f * PI * j / float(zenithSteps)
			//	),
			//	Psh(
			//		m_engine,
			//		coeffs,
			//		PI * i / float(azimuthSteps),
			//		2.0f * PI * (j + 1) / float(zenithSteps)
			//	),
			//	Psh(
			//		m_engine,
			//		coeffs,
			//		PI * (i + 1) / float(azimuthSteps),
			//		2.0f * PI * (j + 1) / float(zenithSteps)
			//	),
			//	Psh(
			//		m_engine,
			//		coeffs,
			//		PI * (i + 1) / float(azimuthSteps),
			//		2.0f * PI * j / float(zenithSteps)
			//	)
			//};
			const Vector4 v[] =
			{
				Pfs(
					m_lightFunction,
					PI * i / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				),
				Pfs(
					m_lightFunction,
					PI * i / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Pfs(
					m_lightFunction,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Pfs(
					m_lightFunction,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				)
			};

			primitiveRenderer->drawWireQuad(
				v[0], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[0].length() / 2.0f),
				v[1], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[1].length() / 2.0f),
				v[2], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[2].length() / 2.0f),
				v[3], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[3].length() / 2.0f)
			);
		}
	}

	// primitiveRenderer->drawWireAabb(Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 0.0f), 0xffff00);
}

void SHEditorPage::eventRender2(RenderControlEvent* event)
{
	PrimitiveRenderer* primitiveRenderer = event->getPrimitiveRenderer();

	const Color4ub colorGrid(0, 0, 0, 120);
	const float unit = 0.25f;

	for (int32_t x = -4; x <= 4; ++x)
	{
		const float fx = float(x * unit);
		primitiveRenderer->drawLine(
			Vector4(fx, 0.0f, -4.0f * unit, 1.0f),
			Vector4(fx, 0.0f, 4.0f * unit, 1.0f),
			0.0f,
			colorGrid
		);
		primitiveRenderer->drawLine(
			Vector4(-4.0f * unit, 0.0f, fx, 1.0f),
			Vector4(4.0f * unit, 0.0f, fx, 1.0f),
			0.0f,
			colorGrid
		);
	}

	primitiveRenderer->drawWireFrame(Matrix44::identity(), 1.0f);

	const int32_t azimuthSteps = 40;
	const int32_t zenithSteps = 40;

	for (int32_t i = 0; i < azimuthSteps; ++i)
	{
		for (int32_t j = 0; j < zenithSteps; ++j)
		{
			const Vector4 v[] =
			{
				Psh(
					m_lightCoefficients,
					PI * i / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				),
				Psh(
					m_lightCoefficients,
					PI * i / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Psh(
					m_lightCoefficients,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Psh(
					m_lightCoefficients,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				)
			};
	
			primitiveRenderer->drawWireQuad(
				v[0], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[0].length() / 2.0f),
				v[1], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[1].length() / 2.0f),
				v[2], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[2].length() / 2.0f),
				v[3], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[3].length() / 2.0f)
			);
		}
	}

	// primitiveRenderer->drawWireAabb(Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 0.0f), 0xffff00);
}

//void SHEditorPage::eventRender3(RenderControlEvent* event)
//{
//	PrimitiveRenderer* primitiveRenderer = event->getPrimitiveRenderer();
//
//	primitiveRenderer->pushWorld(event->getMatrices()[0]);
//
//	SHMatrix shRotate = generateRotationSHMatrix(event->getMatrices()[1], 4);
//	SHCoeffs coeffs = m_lightCoefficients.transform(shRotate);
//
//	const int azimuthSteps = 40;
//	const int zenithSteps = 40;
//
//	for (int i = 0; i < azimuthSteps; ++i)
//	{
//		for (int j = 0; j < zenithSteps; ++j)
//		{
//			Vector4 v[] =
//			{
//				Psh(
//					m_engine,
//					coeffs,
//					PI * i / float(azimuthSteps),
//					2.0f * PI * j / float(zenithSteps)
//				),
//				Psh(
//					m_engine,
//					coeffs,
//					PI * i / float(azimuthSteps),
//					2.0f * PI * (j + 1) / float(zenithSteps)
//				),
//				Psh(
//					m_engine,
//					coeffs,
//					PI * (i + 1) / float(azimuthSteps),
//					2.0f * PI * (j + 1) / float(zenithSteps)
//				),
//				Psh(
//					m_engine,
//					coeffs,
//					PI * (i + 1) / float(azimuthSteps),
//					2.0f * PI * j / float(zenithSteps)
//				)
//			};
//			//Vector4 v[] =
//			//{
//			//	Pfs(
//			//		m_lightFunction,
//			//		PI * i / float(azimuthSteps),
//			//		2.0f * PI * j / float(zenithSteps)
//			//	),
//			//	Pfs(
//			//		m_lightFunction,
//			//		PI * i / float(azimuthSteps),
//			//		2.0f * PI * (j + 1) / float(zenithSteps)
//			//	),
//			//	Pfs(
//			//		m_lightFunction,
//			//		PI * (i + 1) / float(azimuthSteps),
//			//		2.0f * PI * (j + 1) / float(zenithSteps)
//			//	),
//			//	Pfs(
//			//		m_lightFunction,
//			//		PI * (i + 1) / float(azimuthSteps),
//			//		2.0f * PI * j / float(zenithSteps)
//			//	)
//			//};
//
//			primitiveRenderer->drawWireQuad(
//				v[0], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[0].length() / 2.0f),
//				v[1], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[1].length() / 2.0f),
//				v[2], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[2].length() / 2.0f),
//				v[3], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[3].length() / 2.0f)
//			);
//		}
//	}
//
//	// primitiveRenderer->drawWireAabb(Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 0.0f), 0xffff00);
//}
//
//void SHEditorPage::eventRender4(RenderControlEvent* event)
//{
//	PrimitiveRenderer* primitiveRenderer = event->getPrimitiveRenderer();
//
//	primitiveRenderer->pushWorld(event->getMatrices()[0]);
//
//	SHMatrix shRotate = generateRotationSHMatrix(event->getMatrices()[1], 4);
//	SHCoeffs coeffs = m_phaseCoefficients.transform(shRotate);
//
//	const int azimuthSteps = 40;
//	const int zenithSteps = 40;
//
//	for (int i = 0; i < azimuthSteps; ++i)
//	{
//		for (int j = 0; j < zenithSteps; ++j)
//		{
//			Vector4 v[] =
//			{
//				Psh(
//					m_engine,
//					coeffs,
//					PI * i / float(azimuthSteps),
//					2.0f * PI * j / float(zenithSteps)
//				),
//				Psh(
//					m_engine,
//					coeffs,
//					PI * i / float(azimuthSteps),
//					2.0f * PI * (j + 1) / float(zenithSteps)
//				),
//				Psh(
//					m_engine,
//					coeffs,
//					PI * (i + 1) / float(azimuthSteps),
//					2.0f * PI * (j + 1) / float(zenithSteps)
//				),
//				Psh(
//					m_engine,
//					coeffs,
//					PI * (i + 1) / float(azimuthSteps),
//					2.0f * PI * j / float(zenithSteps)
//				)
//			};
//			//Vector4 v[] =
//			//{
//			//	Pfs(
//			//		m_lightFunction,
//			//		PI * i / float(azimuthSteps),
//			//		2.0f * PI * j / float(zenithSteps)
//			//	),
//			//	Pfs(
//			//		m_lightFunction,
//			//		PI * i / float(azimuthSteps),
//			//		2.0f * PI * (j + 1) / float(zenithSteps)
//			//	),
//			//	Pfs(
//			//		m_lightFunction,
//			//		PI * (i + 1) / float(azimuthSteps),
//			//		2.0f * PI * (j + 1) / float(zenithSteps)
//			//	),
//			//	Pfs(
//			//		m_lightFunction,
//			//		PI * (i + 1) / float(azimuthSteps),
//			//		2.0f * PI * j / float(zenithSteps)
//			//	)
//			//};
//
//			primitiveRenderer->drawWireQuad(
//				v[0], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[0].length() / 2.0f),
//				v[1], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[1].length() / 2.0f),
//				v[2], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[2].length() / 2.0f),
//				v[3], lerp(Color4ub(0xff7f7f7f), Color4ub(0xffffffff), v[3].length() / 2.0f)
//			);
//		}
//	}
//
//	// primitiveRenderer->drawWireAabb(Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 0.0f), 0xffff00);
//}

}
