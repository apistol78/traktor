/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundAssetEditorFactory_H
#define traktor_sound_SoundAssetEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundAssetEditorFactory : public editor::IObjectEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const T_OVERRIDE T_FINAL;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const T_OVERRIDE T_FINAL;

	virtual Ref< editor::IObjectEditor > createObjectEditor(editor::IEditor* editor) const T_OVERRIDE T_FINAL;

	virtual void getCommands(std::list< ui::Command >& outCommands) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_sound_SoundAssetEditorFactory_H
