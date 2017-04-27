/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/Lua/ScriptBlobLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptBlobLua", 0, ScriptBlobLua, IScriptBlob)

void ScriptBlobLua::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"fileName", m_fileName);
	s >> Member< std::string >(L"script", m_script);
}

	}
}
