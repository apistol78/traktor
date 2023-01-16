/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Class/Any.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

}

namespace traktor::script
{

class IScriptBlob;

/*! Script context.
 * \ingroup Script
 *
 * A script context is a thin, isolated, environment
 * in the script land.
 * They're created by the script manager and are
 * intended to be small environments which can
 * be embedded into entities etc.
 */
class T_DLLCLASS IScriptContext : public Object
{
	T_RTTI_CLASS;

public:
	/*! Explicit destroy context. */
	virtual void destroy() = 0;

	/*! Load script blob into context.
	 *
	 * \param scriptBlob Pre-compiled script blob.
	 * \return True if resource loaded successfully.
	 */
	virtual bool load(const IScriptBlob* scriptBlob) = 0;

	/*! Set global variable value.
	 *
	 * \param globalName Name of global variable.
	 * \param globalValue Value of global variable.
	 */
	virtual void setGlobal(const std::string& globalName, const Any& globalValue) = 0;

	/*! Get global variable value.
	 *
	 * \param globalName Name of global variable.
	 * \return Global value; void if not found.
	 */
	virtual Any getGlobal(const std::string& globalName) = 0;

	/*! Get class defined in script environment.
	 *
	 * \param className Name of class.
	 * \return Runtime class declaration.
	 */
	virtual Ref< const IRuntimeClass > findClass(const std::string& className) = 0;

	/*! Return true if context contains function (or method).
	 *
	 * \param functionName Name of function of interest.
	 * \return True if function exists.
	 */
	virtual bool haveFunction(const std::string& functionName) const = 0;

	/*! Execute function.
	 *
	 * \param functionName Name of function to execute.
	 * \param argc Number of arguments.
	 * \param argv Argument vector.
	 * \return Return value from function.
	 */
	virtual Any executeFunction(const std::string& functionName, uint32_t argc = 0, const Any* argv = 0) = 0;
};

}
