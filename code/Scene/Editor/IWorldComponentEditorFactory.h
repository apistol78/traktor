/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

class IWorldComponentEditor;

/*!
 * \ingroup Scene
 */
class T_DLLCLASS IWorldComponentEditorFactory : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get supported set of component data types.
	 *
	 * \return Set of component data types.
	 */
	virtual const TypeInfoSet getComponentDataTypes() const = 0;

	/*! Create component editor instance.
	 *
	 * \param controllerDataType Component data type.
	 */
	virtual Ref< IWorldComponentEditor > createComponentEditor(const TypeInfo& componentDataType) const = 0;
};

}
