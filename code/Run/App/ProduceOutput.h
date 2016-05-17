#ifndef traktor_run_ProduceOutput_H
#define traktor_run_ProduceOutput_H

#include "Core/Io/StringOutputStream.h"
#include "Run/App/IOutput.h"

namespace traktor
{
	namespace run
	{

class ProduceOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	virtual void print(const std::wstring& s) T_OVERRIDE T_FINAL;

	virtual void printLn(const std::wstring& s) T_OVERRIDE T_FINAL;

	void printSection(int32_t id);

	int32_t addSection(const std::wstring& section);

	std::wstring getProduct() const;

private:
	std::vector< std::wstring > m_sections;
	StringOutputStream m_ss;
};

	}
}

#endif	// traktor_run_ProduceOutput_H
