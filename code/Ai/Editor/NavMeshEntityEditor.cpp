#include "Ai/Editor/NavMeshEntityEditor.h"
#include "Ai/NavMesh.h"
#include "Ai/NavMeshEntityData.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshEntityEditor", NavMeshEntityEditor, scene::DefaultEntityEditor)

NavMeshEntityEditor::NavMeshEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

void NavMeshEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const NavMeshEntityData* navMeshEntityData = checked_type_cast< const NavMeshEntityData* >(getEntityAdapter()->getEntityData());

	if (getContext()->shouldDrawGuide(L"Ai.NavMesh"))
	{
		resource::Proxy< NavMesh > navMesh;
		if (!getContext()->getResourceManager()->bind(navMeshEntityData->get(), navMesh))
			return;

		if (navMesh->m_navMeshPolygons.empty())
			return;

		primitiveRenderer->pushWorld(/*getEntityAdapter()->getTransform().toMatrix44()*/ Matrix44::identity());
		primitiveRenderer->pushDepthEnable(true);

		const uint16_t* nmp = navMesh->m_navMeshPolygons.c_ptr();
		T_ASSERT (nmp);

		for (uint32_t i = 0; i < navMesh->m_navMeshPolygons.size(); )
		{
			uint16_t npv = nmp[i++];

			for (uint16_t j = 0; j < npv - 2; ++j)
			{
				uint16_t i0 = nmp[i];
				uint16_t i1 = nmp[i + j + 1];
				uint16_t i2 = nmp[i + j + 2];

				primitiveRenderer->drawSolidTriangle(
					navMesh->m_navMeshVertices[i0],
					navMesh->m_navMeshVertices[i1],
					navMesh->m_navMeshVertices[i2],
					Color4ub(0, 255, 255, 64)
				);
			}

			i += npv;
		}

		primitiveRenderer->popDepthEnable();
		primitiveRenderer->pushDepthEnable(false);

		for (uint32_t i = 0; i < navMesh->m_navMeshPolygons.size(); )
		{
			uint16_t npv = nmp[i++];

			for (uint16_t j = 0; j < npv; ++j)
			{
				uint16_t i0 = nmp[i + j];
				uint16_t i1 = nmp[i + (j + 1) % npv];

				primitiveRenderer->drawLine(
					navMesh->m_navMeshVertices[i0],
					navMesh->m_navMeshVertices[i1],
					Color4ub(0, 255, 255, 255)
				);
			}

			i += npv;
		}

		primitiveRenderer->popDepthEnable();
		primitiveRenderer->popWorld();
	}

	scene::DefaultEntityEditor::drawGuide(primitiveRenderer);
}

	}
}
