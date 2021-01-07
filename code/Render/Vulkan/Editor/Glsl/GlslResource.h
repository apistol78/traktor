#pragma once

#include <string>
#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class GlslResource : public Object
{
	T_RTTI_CLASS;

public:
	enum BindStages
	{
		BsVertex = 1,
		BsFragment = 2,
		BsCompute = 4
	};

	int32_t getBinding() const { return m_binding; }

	void addStage(uint8_t stage) { m_stages |= stage; }

	uint8_t getStages() const { return m_stages; }

	const std::wstring& getName() const { return m_name; }

	virtual int32_t getOrdinal() const = 0;

protected:
	GlslResource(const std::wstring& name, uint8_t stages);

private:
	friend class GlslLayout;

	int32_t m_binding;
	uint8_t m_stages;
	std::wstring m_name;
};

	}
}
