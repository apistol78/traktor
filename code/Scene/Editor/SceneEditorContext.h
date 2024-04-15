/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Frustum.h"
#include "Render/Types.h"
#include "Ui/EventSubject.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Database;
class Instance;

}

namespace traktor::editor
{

class IDocument;
class IEditor;

}

namespace traktor::physics
{

class PhysicsManager;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::script
{

class IScriptContext;

}

namespace traktor::world
{

class Entity;
class EntityData;

}

namespace traktor::scene
{

class Camera;
class EntityAdapter;
class IComponentEditorFactory;
class IEntityEditorFactory;
class IModifier;
class ISceneControllerEditor;
class ISceneEditorPlugin;
class ISceneEditorProfile;
class ISceneRenderControl;
class Scene;
class SceneAsset;

/*! Scene editor context.
 *
 * Scene entities, editor profiles and entity editors are kept
 * by the scene editor context.
 */
class T_DLLCLASS SceneEditorContext : public ui::EventSubject
{
	T_RTTI_CLASS;

public:
	enum GetEntitiesFlags
	{
		GfNoDescendants = 0,
		GfDescendants = 1 << 0,
		GfSelectedOnly = 1 << 1,
		GfNoSelected = 1 << 2,
		GfExternalOnly = 1 << 5,
		GfNoExternal = 1 << 6,
		GfExternalChildOnly = 1 << 7,
		GfNoExternalChild = 1 << 8,
		GfDefault = (GfDescendants)
	};

	enum SnapMode
	{
		SmNone,
		SmGrid,
		SmNeighbour
	};

	explicit SceneEditorContext(
		editor::IEditor* editor,
		editor::IDocument* document,
		db::Database* resourceDb,
		db::Database* sourceDb,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		physics::PhysicsManager* physicsManager,
		script::IScriptContext* scriptContext
	);

	virtual ~SceneEditorContext();

	void destroy();

	void addEditorProfile(ISceneEditorProfile* editorProfile);

	void addEditorPlugin(ISceneEditorPlugin* editorPlugin);

	void createFactories();

	void setControllerEditor(ISceneControllerEditor* controllerEditor);

	/*! \name State management. */
	//@{

	void setModifier(IModifier* modifier);

	IModifier* getModifier() const;

	void setGuideSize(float guideSize);

	float getGuideSize() const;

	void setPickEnable(bool pickEnable);

	bool getPickEnable() const;

	void setSnapMode(SnapMode snapMode);

	SnapMode getSnapMode() const;

	void setSnapSpacing(float snapSpacing);

	float getSnapSpacing() const;

	void setPhysicsEnable(bool physicsEnable);

	bool getPhysicsEnable() const;

	void resetPhysics();

	//@}

	/*! \name Camera control. */
	//@{

	/*! Get camera by index.
	 *
	 * \param index Camera index, 0 to 3.
	 * \return Camera.
	 */
	Camera* getCamera(int index) const;

	void moveToEntityAdapter(EntityAdapter* entityAdapter);

	//@}

	/*! \name Time control. */
	//@{

	void setPlaying(bool playing);

	bool isPlaying() const;

	void setTimeScale(float timeScale);

	float getTimeScale() const;

	void setTime(double time);

	double getTime() const;

	void enqueueRedraw(ISceneRenderControl* renderControl);

	void processAutoRedraw();

	//@}

	/*! \name Entity editors. */
	//@{

	void setDrawGuide(const std::wstring& guideId, bool shouldDraw);

	bool shouldDrawGuide(const std::wstring& guideId) const;

	//@}

	void setSceneAsset(SceneAsset* sceneAsset);

	const IEntityEditorFactory* findEntityEditorFactory(const TypeInfo& entityDataType) const;

	const IComponentEditorFactory* findComponentEditorFactory(const TypeInfo& componentDataType) const;

	void buildEntities();

	void buildController();

	void selectEntity(EntityAdapter* entityAdapter, bool select = true);

	void selectAllEntities(bool select = true);

	/*! Get entities. */
	RefArray< EntityAdapter > getEntities(uint32_t flags = GfDefault) const;

	uint32_t findAdaptersOfType(const TypeInfo& entityType, RefArray< EntityAdapter >& outEntityAdapters, uint32_t flags = GfDefault) const;

	EntityAdapter* findAdapterFromEntity(const world::Entity* entity) const;

	EntityAdapter* queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, bool onlyPickable, bool throughSelected) const;

	RefArray< EntityAdapter > queryFrustum(const Frustum& worldFrustum, bool onlyPickable) const;

	void cloneSelected();

	/*! \name Plugin access. */
	//@{

	ISceneEditorPlugin* getEditorPluginOf(const TypeInfo& pluginType) const;

	template < typename PluginType >
	PluginType* getEditorPluginOf() const
	{
		return dynamic_type_cast< PluginType* >(getEditorPluginOf(type_of< PluginType >()));
	}

	//@}

	/*! \name Accessors. */
	//@{

	editor::IEditor* getEditor() { return m_editor; }

	editor::IDocument* getDocument() { return m_document; }

	db::Database* getResourceDatabase() { return m_resourceDb; }

	db::Database* getSourceDatabase() { return m_sourceDb; }

	resource::IResourceManager* getResourceManager() const { return m_resourceManager; }

	render::IRenderSystem* getRenderSystem() const { return m_renderSystem; }

	physics::PhysicsManager* getPhysicsManager() const { return m_physicsManager; }

	script::IScriptContext* getScriptContext() const { return m_scriptContext; }

	RefArray< ISceneEditorProfile >& getEditorProfiles() { return m_editorProfiles; }

	RefArray< ISceneEditorPlugin >& getEditorPlugins() { return m_editorPlugins; }

	Ref< ISceneControllerEditor >& getControllerEditor() { return m_controllerEditor; }

	SceneAsset* getSceneAsset() { return m_sceneAsset; }

	Scene* getScene() { return m_scene; }

	const RefArray< EntityAdapter > getLayerEntityAdapters() { return m_layerEntityAdapters; }

	uint32_t getEntityCount() const { return m_entityCount; }

	//@}

	/*! \name Events. */
	//@{

	void raisePreModify();

	void raisePostModify();

	void raisePostFrame();

	void raisePostBuild();

	void raiseSelect();

	void raiseCameraMoved();

	void raiseModifierChanged();

	void raiseRedraw(ISceneRenderControl* renderControl);

	void raiseMeasurement(int32_t pass, int32_t level, const std::wstring& name, double start, double duration);

	//@}

private:
	editor::IEditor* m_editor;
	editor::IDocument* m_document;
	Ref< db::Database > m_resourceDb;
	Ref< db::Database > m_sourceDb;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< physics::PhysicsManager > m_physicsManager;
	Ref< script::IScriptContext > m_scriptContext;
	RefArray< ISceneEditorProfile > m_editorProfiles;
	RefArray< ISceneEditorPlugin > m_editorPlugins;
	RefArray< const IEntityEditorFactory > m_entityEditorFactories;
	RefArray< const IComponentEditorFactory > m_componentEditorFactories;
	Ref< ISceneControllerEditor > m_controllerEditor;
	Ref< IModifier > m_modifier;
	SmallMap< std::wstring, bool > m_drawGuide;
	float m_guideSize;
	bool m_pickEnable;
	SnapMode m_snapMode;
	float m_snapSpacing;
	bool m_physicsEnable;
	bool m_playing;
	float m_timeScale;
	double m_time;
	int32_t m_redrawUntilStop;
	uint32_t m_buildCount;
	uint32_t m_entityCount;
	Ref< Camera > m_cameras[4];
	Ref< SceneAsset > m_sceneAsset;
	Ref< Scene > m_scene;
	RefArray< EntityAdapter > m_layerEntityAdapters;
	SmallMap< const world::Entity*, EntityAdapter* > m_entityAdapterMap;
};

}
