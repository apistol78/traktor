/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/McpToolSupport.h"

#include "Core/Guid.h"
#include "Core/Thread/Signal.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "MCP/Editor/Json.h"
#include "Ui/Application.h"

#include <memory>

namespace traktor::mcp
{
namespace
{

// Heap-held work item, kept alive by both the deferred lambda and this call, so a
// timed-out wait never leaves the deferred function referencing freed state.
struct UiWork
{
	std::function< void() > fn;
	Signal done;
};

}

bool runOnUiThread(const std::function< void() >& fn, int32_t timeoutMs)
{
	ui::Application* application = ui::Application::getInstance();
	if (!application)
		return false;

	std::shared_ptr< UiWork > work = std::make_shared< UiWork >();
	work->fn = fn;

	application->defer([work]() {
		if (work->fn)
			work->fn();
		work->done.set();
	});

	return work->done.wait(timeoutMs);
}

Ref< db::Instance > resolveInstance(db::Database* database, const Json* arguments, std::wstring& outError)
{
	if (!arguments)
	{
		outError = L"Missing arguments; expected \"guid\" or \"path\".";
		return nullptr;
	}

	const std::wstring guidText = arguments->getMember(L"guid") ? arguments->getMember(L"guid")->getString() : L"";
	if (!guidText.empty())
	{
		const Guid guid(guidText);
		if (!guid.isValid())
		{
			outError = L"Invalid guid: " + guidText;
			return nullptr;
		}
		return database->getInstance(guid);
	}

	if (arguments->getMember(L"path"))
		return database->getInstance(arguments->getMember(L"path")->getString());

	outError = L"Missing \"guid\" or \"path\".";
	return nullptr;
}

Ref< Json > coerceStructuredArguments(const Json* arguments)
{
	if (!arguments || !arguments->isObject())
		return const_cast< Json* >(arguments);

	// Detect members delivered as a JSON-encoded string of an array/object.
	// (A bare string that merely looks like a number/bool is left alone so
	// genuine string parameters keep their literal value.)
	bool needsCoerce = false;
	for (uint32_t i = 0; i < arguments->getMemberCount() && !needsCoerce; ++i)
	{
		const Json* value = arguments->getMemberValue(i);
		if (value && value->isString())
		{
			Ref< Json > parsed = Json::parse(value->getString());
			if (parsed && (parsed->isArray() || parsed->isObject()))
				needsCoerce = true;
		}
	}

	// Common case: nothing to recover - return the original arguments unchanged.
	// NOTE: never return a wrapper that *shares* the original child nodes; that
	// creates double ownership and crashes on teardown.
	if (!needsCoerce)
		return const_cast< Json* >(arguments);

	// Build a fully INDEPENDENT arguments object (no child shared with the
	// original): deep-copy each member via serialization, replacing any
	// stringified array/object member with its parsed form.
	Ref< Json > result = Json::createObject();
	for (uint32_t i = 0; i < arguments->getMemberCount(); ++i)
	{
		const std::wstring name = arguments->getMemberName(i);
		const Json* value = arguments->getMemberValue(i);
		Ref< Json > copy;
		if (value && value->isString())
		{
			Ref< Json > parsed = Json::parse(value->getString());
			if (parsed && (parsed->isArray() || parsed->isObject()))
				copy = parsed;
		}
		if (!copy && value)
			copy = Json::parse(value->toString());
		result->set(name, copy);
	}
	return result;
}

}
