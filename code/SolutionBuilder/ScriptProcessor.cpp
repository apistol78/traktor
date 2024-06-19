/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/BoxedClassFactory.h"
#include "Core/Class/CoreClassFactory1.h"
#include "Core/Class/CoreClassFactory2.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptProfiler.h"
#include "Script/Lua/ScriptCompilerLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "SolutionBuilder/ClassFactory.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/Output.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Solution.h"

namespace traktor::sb
{
	namespace
	{

class ScriptProfilerListener : public script::IScriptProfiler::IListener
{
public:
	SmallMap< std::wstring, double > m_durations;

	virtual void callEnter(const Guid& scriptId, const std::wstring& function) override final
	{
	}

	virtual void callLeave(const Guid& scriptId, const std::wstring& function) override final
	{
	}

	virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) override final
	{
		m_durations[function] += exclusiveDuration;
	}
};


	}

T_IMPLEMENT_RTTI_CLASS(L"ScriptProcessor", ScriptProcessor, Object)

bool ScriptProcessor::create(const CommandLine& cmdLine)
{
	// Create script manager and register our classes.
	m_scriptCompiler = new script::ScriptCompilerLua();
	m_scriptManager = new script::ScriptManagerLua();
	m_profile = cmdLine.hasOption(L"profile");

	BoxedClassFactory().createClasses(m_scriptManager);
	CoreClassFactory1().createClasses(m_scriptManager);
	CoreClassFactory2().createClasses(m_scriptManager);
	ClassFactory().createClasses(m_scriptManager);

	return true;
}

void ScriptProcessor::destroy()
{
	safeDestroy(m_scriptContext);
	safeDestroy(m_scriptManager);
}

bool ScriptProcessor::prepare(const std::wstring& fileName)
{
	// Read generator script into memory.
	Ref< IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return false;

	Utf8Encoding encoding;
	BufferedStream stream(file);
	StringReader reader(&stream, &encoding);
	StringOutputStream ss;

	std::wstring tmp;
	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	file->close();

	// Keep source from stream.
	const std::wstring source = ss.str();
	ss.reset();

	// Transform generator script into pure code script.
	size_t offset = 0;

	ss << L"function __main__()" << Endl;

	m_sections.resize(0);
	for (;;)
	{
		const size_t s = source.find(L"<?--", offset);
		if (s == source.npos)
			break;

		const size_t e = source.find(L"--?>", s);
		if (e == source.npos)
			return false;

		const std::wstring section = source.substr(offset, s - offset);
		m_sections.push_back(section);

		ss << L"\toutput:printSection(" << (int32_t)(m_sections.size() - 1) << L")" << Endl;
		ss << source.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	const std::wstring section = source.substr(offset);
	m_sections.push_back(section);

	ss << L"\toutput:printSection(" << (int32_t)(m_sections.size() - 1) << L")" << Endl;
	ss << L"end" << Endl;

	// Compile script into blob.
	Ref< script::IScriptBlob > scriptBlob = m_scriptCompiler->compile(fileName, ss.str(), 0);
	if (!scriptBlob)
		return false;

	// Create execution context.
	safeDestroy(m_scriptContext);
	m_scriptContext = m_scriptManager->createContext(true);
	if (!m_scriptContext)
		return false;

	if (!m_scriptContext->load(scriptBlob))
		return false;

	return true;
}

bool ScriptProcessor::generate(const Solution* solution, const Project* project, const std::wstring& configurationName, const std::wstring& projectPath, std::wstring& output) const
{
	const Path projectPathAbs = FileSystem::getInstance().getAbsolutePath(Path(projectPath));

	Ref< Output > o = new Output(m_sections);

	m_scriptContext->setGlobal("output", Any::fromObject(o));
	m_scriptContext->setGlobal("solution", Any::fromObject(const_cast< Solution* >(solution)));
	m_scriptContext->setGlobal("project", Any::fromObject(const_cast< Project* >(project)));
	m_scriptContext->setGlobal("configurationName", Any::fromString(configurationName));
	m_scriptContext->setGlobal("projectPath", Any::fromObject(new Path(projectPathAbs)));
	m_scriptContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));

	Ref< script::IScriptProfiler > profiler;
	ScriptProfilerListener pl;

	if (m_profile)
	{
		if ((profiler = m_scriptManager->createProfiler()) != nullptr)
			profiler->addListener(&pl);
	}

	m_scriptContext->executeFunction("__main__");

	if (profiler)
	{
		profiler->removeListener(&pl);
		for (auto it : pl.m_durations)
			log::info << it.first << L" : " << (int32_t)(it.second * 1000.0) << L" ms" << Endl;
		profiler = nullptr;
	}

	output = o->getProduct();
	return true;
}

}
