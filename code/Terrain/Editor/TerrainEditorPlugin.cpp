#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldEditorPlugin.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/Terrain.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditorPlugin", TerrainEditorPlugin, scene::ISceneEditorPlugin)

TerrainEditorPlugin::TerrainEditorPlugin(scene::SceneEditorContext* context)
:	m_context(context)
,	m_selectedTool(0)
{
}

bool TerrainEditorPlugin::create(ui::Widget* parent, ui::custom::ToolBar* toolBar)
{
	uint32_t base = toolBar->addImage(ui::Bitmap::load(c_ResourceTerrain, sizeof(c_ResourceTerrain), L"png"), 6);
	toolBar->addItem(new ui::custom::ToolBarSeparator());

	m_toolRaiseTool = new ui::custom::ToolBarButton(L"Raise/lower tool", ui::Command(L"Terrain.RaiseLowerTool"), base, ui::custom::ToolBarButton::BsDefaultToggled);
	m_toolFlattenTool = new ui::custom::ToolBarButton(L"Flatten tool", ui::Command(L"Terrain.FlattenTool"), base + 1, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolSmoothTool = new ui::custom::ToolBarButton(L"Smooth tool", ui::Command(L"Terrain.SmoothTool"), base + 2, ui::custom::ToolBarButton::BsDefaultToggle);

	toolBar->addItem(m_toolRaiseTool);
	toolBar->addItem(m_toolFlattenTool);
	toolBar->addItem(m_toolSmoothTool);
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(new ui::custom::ToolBarButton(L"Align sel. with terrain", ui::Command(L"Terrain.AlignSelected"), base + 3, ui::custom::ToolBarButton::BsDefault));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Orient sel. with terrain", ui::Command(L"Terrain.OrientSelected"), base + 4, ui::custom::ToolBarButton::BsDefault));

	return true;
}

bool TerrainEditorPlugin::handleCommand(const ui::Command& command)
{
	if (command == L"Terrain.RaiseLowerTool")
	{
		m_toolRaiseTool->setToggled(true);
		m_toolFlattenTool->setToggled(false);
		m_toolSmoothTool->setToggled(false);
		m_selectedTool = 0;
	}
	else if (command == L"Terrain.FlattenTool")
	{
		m_toolRaiseTool->setToggled(false);
		m_toolFlattenTool->setToggled(true);
		m_toolSmoothTool->setToggled(false);
		m_selectedTool = 1;
	}
	else if (command == L"Terrain.SmoothTool")
	{
		m_toolRaiseTool->setToggled(false);
		m_toolFlattenTool->setToggled(false);
		m_toolSmoothTool->setToggled(true);
		m_selectedTool = 2;
	}
	else if (command == L"Terrain.AlignSelected")
	{
		scene::EntityAdapter* terrainEntityAdapter = m_context->findAdapterFromType(type_of< TerrainEntityData >());
		if (!terrainEntityAdapter)
			return false;

		Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(terrainEntityAdapter->getEntity());

		hf::HeightfieldEditorPlugin* heightfieldPlugin = m_context->getEditorPluginOf< hf::HeightfieldEditorPlugin >();
		if (!heightfieldPlugin)
			return false;

		hf::HeightfieldCompositor* compositor = heightfieldPlugin->getCompositor(terrainEntity->getHeightfield().getGuid());
		if (!compositor)
			return false;

		RefArray< scene::EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);

		for (RefArray< scene::EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
		{
			Transform T = (*i)->getTransform();

			float Ty = compositor->getBilinearHeight(T.translation().x(), T.translation().z());
			float Ey = (*i)->getBoundingBox().mn.y();

			(*i)->setTransform(Transform(
				T.translation() * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4(0.0f, Ty - Ey, 0.0f, 0.0f),
				T.rotation()
			));
		}
	}
	else if (command == L"Terrain.OrientSelected")
	{
		scene::EntityAdapter* terrainEntityAdapter = m_context->findAdapterFromType(type_of< TerrainEntityData >());
		if (!terrainEntityAdapter)
			return false;

		Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(terrainEntityAdapter->getEntity());

		hf::HeightfieldEditorPlugin* heightfieldPlugin = m_context->getEditorPluginOf< hf::HeightfieldEditorPlugin >();
		if (!heightfieldPlugin)
			return false;

		hf::HeightfieldCompositor* compositor = heightfieldPlugin->getCompositor(terrainEntity->getHeightfield().getGuid());
		if (!compositor)
			return false;

		RefArray< scene::EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, scene::SceneEditorContext::GfDescendants | scene::SceneEditorContext::GfSelectedOnly);

		const float dxz = 0.01f;

		for (RefArray< scene::EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
		{
			Transform T = (*i)->getTransform();

			float Txz[][2] =
			{
				{ T.translation().x() - dxz, T.translation().z() },
				{ T.translation().x() + dxz, T.translation().z() },
				{ T.translation().x(), T.translation().z() - dxz },
				{ T.translation().x(), T.translation().z() + dxz }
			};

			float Ty[] =
			{
				compositor->getBilinearHeight(Txz[0][0], Txz[0][1]),
				compositor->getBilinearHeight(Txz[1][0], Txz[1][1]),
				compositor->getBilinearHeight(Txz[2][0], Txz[2][1]),
				compositor->getBilinearHeight(Txz[3][0], Txz[3][1])
			};

			Vector4 Tp[] =
			{
				Vector4(Txz[0][0], Ty[0], Txz[0][1], 1.0f),
				Vector4(Txz[1][0], Ty[1], Txz[1][1], 1.0f),
				Vector4(Txz[2][0], Ty[2], Txz[2][1], 1.0f),
				Vector4(Txz[3][0], Ty[3], Txz[3][1], 1.0f)
			};

			Vector4 N = cross(Tp[2] - Tp[3], Tp[0] - Tp[1]).normalized();

			(*i)->setTransform(Transform(
				T.translation(),
				Quaternion(
					Vector4(0.0f, 1.0f, 0.0f, 0.0f),
					N
				)
			));
		}
	}
	else
		return false;

	return true;
}

	}
}
