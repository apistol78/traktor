#ifndef traktor_scene_SceneEditorContext_H
#define traktor_scene_SceneEditorContext_H

#include "Core/Heap/Ref.h"
#include "Core/Math/Vector4.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class Editor;

	}

	namespace db
	{

class Database;

	}

	namespace render
	{

class RenderSystem;
class PrimitiveRenderer;

	}

	namespace physics
	{

class PhysicsManager;

	}

	namespace world
	{

class Entity;
class EntityData;

	}

	namespace scene
	{

class SceneEditorProfile;
class Camera;
class Modifier;
class EntityEditor;
class EntityAdapter;
class SceneAsset;

/*! \brief Scene editor context.
 *
 * Scene entities, editor profiles and entity editors are kept
 * by the scene editor context.
 */
class T_DLLCLASS SceneEditorContext : public ui::EventSubject
{
	T_RTTI_CLASS(SceneEditorContext)

public:
	enum GetEntitiesFlags
	{
		GfDescendants = 1 << 0,
		GfSelectedOnly = 1 << 1,
		GfDefault = (GfDescendants)
	};

	enum AxisEnable
	{
		AeX = 1,
		AeY = 2,
		AeZ = 4,
		AeXYZ = AeX | AeY | AeZ
	};

	enum EditSpace
	{
		EsWorld,
		EsObject
	};

	SceneEditorContext(
		editor::Editor* editor,
		db::Database* resourceDb,
		db::Database* sourceDb,
		render::RenderSystem* renderSystem,
		physics::PhysicsManager* physicsManager
	);

	void addEditorProfile(SceneEditorProfile* editorProfile);
	
	/*! \name State management. */
	//@{

	void setCamera(Camera* camera);

	Camera* getCamera() const;

	void setModifier(Modifier* modifier);

	Modifier* getModifier() const;

	void setPickEnable(bool pickEnable);

	bool getPickEnable() const;

	void setAxisEnable(uint32_t axisEnable);

	uint32_t getAxisEnable() const;

	void setEditSpace(EditSpace editSpace);

	EditSpace getEditSpace() const;

	void setGuideEnable(bool guideEnable);

	bool getGuideEnable() const;

	void setSnapEnable(bool snapEnable);

	bool getSnapEnable() const;

	void setDeltaScale(float deltaScale);

	float getDeltaScale() const;

	void setPhysicsEnable(bool physicsEnable);

	bool getPhysicsEnable() const;

	void setTimeScale(float timeScale);

	float getTimeScale() const;

	//@}

	/*! \name Entity editors. */
	//@{

	void setEntityEditors(const RefArray< EntityEditor >& entityEditors);

	void drawGuide(render::PrimitiveRenderer* primitiveRenderer, EntityAdapter* entityAdapter);

	//@}

	void setSceneAsset(SceneAsset* sceneAsset);

	/*! \brief Reset entities.
	 *
	 * Reset entity to entity data specification;
	 * useful when previewing physics simulation etc.
	 */
	void resetEntities();

	void updateModified();

	void buildEntities();

	void selectEntity(EntityAdapter* entityAdapter, bool select = true);

	void selectAllEntities(bool select = true);

	/*! \brief Get entities. */
	uint32_t getEntities(RefArray< EntityAdapter >& outEntityAdapters, uint32_t flags = GfDefault) const;

	EntityAdapter* findEntityFromData(const world::EntityData* entityData, int index) const;

	EntityAdapter* queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection) const;

	/*! \name Accessors. */
	//@{

	editor::Editor* getEditor() { return m_editor; }

	db::Database* getResourceDatabase() { return m_resourceDb; }

	db::Database* getSourceDatabase() { return m_sourceDb; }

	render::RenderSystem* getRenderSystem() { return m_renderSystem; }

	physics::PhysicsManager* getPhysicsManager() { return m_physicsManager; }

	RefArray< SceneEditorProfile >& getEditorProfiles() { return m_editorProfiles; }

	RefArray< EntityEditor >& getEntityEditors() { return m_entityEditors; }

	Ref< SceneAsset >& getSceneAsset() { return m_sceneAsset; }

	Ref< EntityAdapter >& getRootEntityAdapter() { return m_rootEntityAdapter; }

	//@}

	/*! \name Events. */
	//@{

	enum Events
	{
		EiPreModify = ui::EiUser + 1,
		EiPostModify = ui::EiUser + 2,
		EiPostFrame = ui::EiUser + 3,
		EiPostBuild = ui::EiUser + 4,
		EiSelect = ui::EiUser + 5
	};

	void raisePreModify();

	void raisePostModify();

	void raisePostFrame(ui::Event* event);

	void raisePostBuild();

	void raiseSelect();

	void addPreModifyEventHandler(ui::EventHandler* eventHandler);

	void addPostModifyEventHandler(ui::EventHandler* eventHandler);

	void addPostFrameEventHandler(ui::EventHandler* eventHandler);

	void addPostBuildEventHandler(ui::EventHandler* eventHandler);

	void addSelectEventHandler(ui::EventHandler* eventHandler);

	//@}

private:
	editor::Editor* m_editor;
	Ref< db::Database > m_resourceDb;
	Ref< db::Database > m_sourceDb;
	Ref< render::RenderSystem > m_renderSystem;
	Ref< physics::PhysicsManager > m_physicsManager;
	RefArray< SceneEditorProfile > m_editorProfiles;
	Ref< Camera > m_camera;
	Ref< Modifier > m_modifier;
	bool m_pickEnable;
	uint32_t m_axisEnable;
	EditSpace m_editSpace;
	bool m_guideEnable;
	bool m_snapEnable;
	float m_deltaScale;
	bool m_physicsEnable;
	float m_timeScale;
	RefArray< EntityEditor > m_entityEditors;
	Ref< SceneAsset > m_sceneAsset;
	Ref< EntityAdapter > m_rootEntityAdapter;
};

	}
}

#endif	// traktor_scene_SceneEditorContext_H
