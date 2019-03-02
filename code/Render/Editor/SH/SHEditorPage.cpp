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
#include "Ui/QuadSplitter.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

Vector4 polarToCartesian(float phi, float theta)
{
	return Vector4(
		cos(theta) * sin(phi),
		cos(phi),
		sin(theta) * sin(phi)
	);
}

// Evaluate point directly using source function.
Vector4 Pfs(SHFunction* function, float phi, float theta)
{
	Vector4 p = polarToCartesian(phi, theta);
	return p * Scalar(function->evaluate(phi, theta, p));
}

// Evaluate point using SH.
Vector4 Psh(SHEngine* engine, const SHCoeffs& coefficients, float phi, float theta)
{
	Vector4 p = polarToCartesian(phi, theta);
	return p * Scalar(engine->evaluate(phi, theta, coefficients));
}

uint32_t buildColor(float r, float g, float b)
{
	r = std::max(0.0f, r);
	g = std::max(0.0f, g);
	b = std::max(0.0f, b);
	r = std::min(1.0f, r);
	g = std::min(1.0f, g);
	b = std::min(1.0f, b);
	return (uint32_t(r * 255.0f) << 16) | (uint32_t(g * 255.0f) << 8) | (uint32_t(b * 255.0f));
}

SHMatrix concate(const SHMatrix& m1, const SHMatrix& m2)
{
	int dim = m1.getRows();

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
	IRenderSystem* renderSystem = m_editor->getStoreObject< IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_container = new ui::Container();
	m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));

	Ref< ui::QuadSplitter > splitter = new ui::QuadSplitter();
	splitter->create(m_container, ui::Point(50, 50), true);

	for (uint32_t i = 0; i < 4; ++i)
	{
		m_renderControls[i] = new RenderControl(3);
		if (!m_renderControls[i]->create(splitter, renderSystem, database))
			return false;
	}

	m_engine = new SHEngine(4);
	m_engine->generateSamplePoints(20000);

	//// @hack Generate SH coefficients from IBL probe.
	//{
	//	SHCoeffs coeffs[3];

	//	Ref< drawing::Image > ibl = drawing::Image::load("C:/temp/bilder/sky probes/Palenque_Angular_Map.jpg");
	//	T_ASSERT (ibl);

	//	IBLProbe functionR(ibl, Vector4(1.0f, 0.0f, 0.0f, 0.0f));
	//	m_engine->generateCoefficients(&functionR, coeffs[0]);
	//
	//	IBLProbe functionG(ibl, Vector4(0.0f, 1.0f, 0.0f, 0.0f));
	//	m_engine->generateCoefficients(&functionG, coeffs[1]);

	//	IBLProbe functionB(ibl, Vector4(0.0f, 0.0f, 1.0f, 0.0f));
	//	m_engine->generateCoefficients(&functionB, coeffs[2]);

	//	for (uint32_t i = 0; i < 3; ++i)
	//	{
	//		log::info << "<item>" << Endl;
	//		log::info << "\t<coefficients>" << Endl;

	//		for (uint32_t j = 0; j < 16; ++j)
	//		{
	//			log::info << "\t\t<item>" << coeffs[i][j] << "</item>" << Endl;
	//		}

	//		log::info << "\t</coefficients>" << Endl;
	//		log::info << "</item>" << Endl;
	//	}
	//}

	m_lightFunction = new DirectionalLight(Vector4(0.0f, 1.0f, 0.0f).normalized(), PI / 3.0f);
	m_engine->generateCoefficients(m_lightFunction, m_lightCoefficients);

	m_phaseFunction = new RayLeighPhaseFunction();
	m_engine->generateCoefficients(m_phaseFunction, m_phaseCoefficients);

	m_phaseTransfer = m_engine->generateTransferMatrix(m_phaseFunction);

	m_renderControls[0]->addEventHandler< RenderControlEvent >(this, &SHEditorPage::eventRender1);
	m_renderControls[1]->addEventHandler< RenderControlEvent >(this, &SHEditorPage::eventRender2);
	m_renderControls[2]->addEventHandler< RenderControlEvent >(this, &SHEditorPage::eventRender3);
	m_renderControls[3]->addEventHandler< RenderControlEvent >(this, &SHEditorPage::eventRender4);

	return true;
}

void SHEditorPage::destroy()
{
	if (m_container)
	{
		m_container->destroy();
		m_container = 0;
	}
}

bool SHEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool SHEditorPage::handleCommand(const ui::Command& command)
{
	return false;
}

void SHEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void SHEditorPage::eventRender1(RenderControlEvent* event)
{
	PrimitiveRenderer* primitiveRenderer = event->getPrimitiveRenderer();

	primitiveRenderer->pushWorld(event->getMatrices()[0]);

	SHMatrix shRotate1 = generateRotationSHMatrix(event->getMatrices()[1], 4);
	SHMatrix shRotate2 = generateRotationSHMatrix(event->getMatrices()[2], 4);

	SHCoeffs coeffs = m_lightCoefficients.transform(shRotate1).transform(concate(m_phaseTransfer, shRotate2));

	const int azimuthSteps = 40;
	const int zenithSteps = 40;

	for (int i = 0; i < azimuthSteps; ++i)
	{
		for (int j = 0; j < zenithSteps; ++j)
		{
			Vector4 v[] =
			{
				Psh(
					m_engine,
					coeffs,
					PI * i / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * i / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				)
			};
			//Vector4 v[] =
			//{
			//	Pfs(
			//		m_lightFunction,
			//		PI * i / float(azimuthSteps),
			//		2.0f * PI * j / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * i / float(azimuthSteps),
			//		2.0f * PI * (j + 1) / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * (i + 1) / float(azimuthSteps),
			//		2.0f * PI * (j + 1) / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * (i + 1) / float(azimuthSteps),
			//		2.0f * PI * j / float(zenithSteps)
			//	)
			//};

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
	//RenderEvent* renderEvent = checked_type_cast< RenderEvent* >(event);
	//PrimitiveRenderer* primitiveRenderer = renderEvent->getPrimitiveRenderer();
}

void SHEditorPage::eventRender3(RenderControlEvent* event)
{
	PrimitiveRenderer* primitiveRenderer = event->getPrimitiveRenderer();

	primitiveRenderer->pushWorld(event->getMatrices()[0]);

	SHMatrix shRotate = generateRotationSHMatrix(event->getMatrices()[1], 4);
	SHCoeffs coeffs = m_lightCoefficients.transform(shRotate);

	const int azimuthSteps = 40;
	const int zenithSteps = 40;

	for (int i = 0; i < azimuthSteps; ++i)
	{
		for (int j = 0; j < zenithSteps; ++j)
		{
			Vector4 v[] =
			{
				Psh(
					m_engine,
					coeffs,
					PI * i / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * i / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				)
			};
			//Vector4 v[] =
			//{
			//	Pfs(
			//		m_lightFunction,
			//		PI * i / float(azimuthSteps),
			//		2.0f * PI * j / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * i / float(azimuthSteps),
			//		2.0f * PI * (j + 1) / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * (i + 1) / float(azimuthSteps),
			//		2.0f * PI * (j + 1) / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * (i + 1) / float(azimuthSteps),
			//		2.0f * PI * j / float(zenithSteps)
			//	)
			//};

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

void SHEditorPage::eventRender4(RenderControlEvent* event)
{
	PrimitiveRenderer* primitiveRenderer = event->getPrimitiveRenderer();

	primitiveRenderer->pushWorld(event->getMatrices()[0]);

	SHMatrix shRotate = generateRotationSHMatrix(event->getMatrices()[1], 4);
	SHCoeffs coeffs = m_phaseCoefficients.transform(shRotate);

	const int azimuthSteps = 40;
	const int zenithSteps = 40;

	for (int i = 0; i < azimuthSteps; ++i)
	{
		for (int j = 0; j < zenithSteps; ++j)
		{
			Vector4 v[] =
			{
				Psh(
					m_engine,
					coeffs,
					PI * i / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * i / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * (j + 1) / float(zenithSteps)
				),
				Psh(
					m_engine,
					coeffs,
					PI * (i + 1) / float(azimuthSteps),
					2.0f * PI * j / float(zenithSteps)
				)
			};
			//Vector4 v[] =
			//{
			//	Pfs(
			//		m_lightFunction,
			//		PI * i / float(azimuthSteps),
			//		2.0f * PI * j / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * i / float(azimuthSteps),
			//		2.0f * PI * (j + 1) / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * (i + 1) / float(azimuthSteps),
			//		2.0f * PI * (j + 1) / float(zenithSteps)
			//	),
			//	Pfs(
			//		m_lightFunction,
			//		PI * (i + 1) / float(azimuthSteps),
			//		2.0f * PI * j / float(zenithSteps)
			//	)
			//};

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

	}
}
