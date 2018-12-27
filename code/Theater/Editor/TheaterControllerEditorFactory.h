/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_theater_TheaterControllerEditorFactory_H
#define traktor_theater_TheaterControllerEditorFactory_H

#include "Scene/Editor/ISceneControllerEditorFactory.h"

namespace traktor
{
	namespace theater
	{

/*! \brief
 */
class TheaterControllerEditorFactory : public scene::ISceneControllerEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getControllerDataTypes() const override final;

	virtual Ref< scene::ISceneControllerEditor > createControllerEditor(const TypeInfo& controllerDataType) const override final;
};

	}
}

#endif	// traktor_theater_TheaterControllerEditorFactory_H
