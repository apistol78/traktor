#include "ProduceOutput.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"ProduceOutput", ProduceOutput, IOutput)

void ProduceOutput::print(const std::wstring& s)
{
	m_ss << s;
}

void ProduceOutput::printLn(const std::wstring& s)
{
	m_ss << s << Endl;
}

void ProduceOutput::printSection(int32_t id)
{
	T_ASSERT (id >= 0 && id < int32_t(m_sections.size()));
	m_ss << m_sections[id];
}

int32_t ProduceOutput::addSection(const std::wstring& section)
{
	m_sections.push_back(section);
	return int32_t(m_sections.size()) - 1;
}

std::wstring ProduceOutput::getProduct() const
{
	return m_ss.str();
}
