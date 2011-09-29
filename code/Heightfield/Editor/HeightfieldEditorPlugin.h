#ifndef traktor_hf_HeightfieldEditorPlugin_H
#define traktor_hf_HeightfieldEditorPlugin_H

#include <map>
#include "Core/Guid.h"
#include "Scene/Editor/ISceneEditorPlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class HeightfieldCompositor;

class T_DLLCLASS HeightfieldEditorPlugin : public scene::ISceneEditorPlugin
{
	T_RTTI_CLASS;

public:
	HeightfieldEditorPlugin(scene::SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::custom::ToolBar* toolBar);

	virtual bool handleCommand(const ui::Command& command);

	HeightfieldCompositor* getCompositor(const Guid& assetGuid);

	bool saveCompositors();

private:
	scene::SceneEditorContext* m_context;
	std::map< Guid, Ref< HeightfieldCompositor > > m_compositors;
};

	}
}

#endif	// traktor_hf_HeightfieldEditorPlugin_H
