/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Editor/Assets.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.editor.Assets", 1, Assets, ISerializable)

void Assets::serialize(ISerializer& s)
{
	if (s.getVersion< Assets >() >= 1)
		s >> MemberStlVector< Dependency, MemberComposite< Dependency > >(L"dependencies", m_dependencies);
	else
	{
		std::vector< Guid > dependencies;
		s >> MemberStlVector< Guid >(L"dependencies", dependencies);
		
		m_dependencies.resize(dependencies.size());
		for (size_t i = 0; i < dependencies.size(); ++i)
		{
			m_dependencies[i].id = dependencies[i];
			m_dependencies[i].editorDeployOnly = false;
		}
	}
}

Assets::Dependency::Dependency()
:	editorDeployOnly(false)
{
}

void Assets::Dependency::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"id", id);
	s >> Member< bool >(L"editorDeployOnly", editorDeployOnly);
}

	}
}
