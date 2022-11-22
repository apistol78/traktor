/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class ISceneControllerEditor;

/*!
 * \ingroup Scene
 */
class T_DLLCLASS ISceneControllerEditorFactory : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get supported set of controller data types.
	 *
	 * \return Set of controller data types.
	 */
	virtual const TypeInfoSet getControllerDataTypes() const = 0;

	/*! Create controller editor instance.
	 *
	 * \param controllerDataType Controller data type.
	 */
	virtual Ref< ISceneControllerEditor > createControllerEditor(const TypeInfo& controllerDataType) const = 0;
};

}
