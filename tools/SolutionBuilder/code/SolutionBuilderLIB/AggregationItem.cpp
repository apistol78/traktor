#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include "SolutionBuilderLIB/AggregationItem.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"AggregationItem", 0, AggregationItem, ISerializable)

void AggregationItem::setSourceFile(const std::wstring& sourceFile)
{
	m_sourceFile = sourceFile;
}

const std::wstring& AggregationItem::getSourceFile() const
{
	return m_sourceFile;
}

void AggregationItem::setTargetPath(const std::wstring& targetPath)
{
	m_targetPath = targetPath;
}

const std::wstring& AggregationItem::getTargetPath() const
{
	return m_targetPath;
}

void AggregationItem::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"sourceFile", m_sourceFile);
	s >> Member< std::wstring >(L"targetPath", m_targetPath);
}
