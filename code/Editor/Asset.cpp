#include "Editor/Asset.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Asset", Asset, Serializable)

Asset::Asset()
{
}

Asset::Asset(const Path& fileName)
:	m_fileName(fileName)
{
}

void Asset::setFileName(const Path& fileName)
{
	m_fileName = fileName;
}

const Path& Asset::getFileName() const
{
	return m_fileName;
}

bool Asset::serialize(Serializer& s)
{
	return s >> Member< Path >(L"fileName", m_fileName);
}

	}
}
