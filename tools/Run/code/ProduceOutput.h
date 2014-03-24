#ifndef ProduceOutput_H
#define ProduceOutput_H

#include <Core/Io/StringOutputStream.h>
#include "IOutput.h"

class ProduceOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	virtual void print(const std::wstring& s);

	virtual void printLn(const std::wstring& s);

	void printSection(int32_t id);

	int32_t addSection(const std::wstring& section);

	std::wstring getProduct() const;

private:
	std::vector< std::wstring > m_sections;
	traktor::StringOutputStream m_ss;
};

#endif	// ProduceOutput_H
