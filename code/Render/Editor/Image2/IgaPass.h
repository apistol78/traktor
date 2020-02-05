#pragma once

#include <list>
#include <string>
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IgaTarget;

class T_DLLCLASS IgaPass : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Input
	{
		std::wstring name;
		Ref< const IgaTarget > source;

		void serialize(ISerializer& s);
	};

	struct Output
	{
		Ref< const IgaTarget > target;

		void serialize(ISerializer& s);
	};

	IgaPass();

	explicit IgaPass(const std::wstring& name);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setPosition(int32_t x, int32_t y);

	const int32_t* getPosition() const;

	const std::list< Input >& getInputs() const;

	bool attachInput(const std::wstring& name, const IgaTarget* source);

	const Output& getOutput() const;

	bool attachOutput(const IgaTarget* target);

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	int32_t m_position[2];
	std::list< Input > m_inputs;
	Output m_output;
};

	}
}