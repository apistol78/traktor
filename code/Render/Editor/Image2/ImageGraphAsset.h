#pragma once

#include "Core/RefArray.h"
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

class IgaPass;
class IgaTarget;

class T_DLLCLASS ImageGraphAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addTarget(IgaTarget* target);

	const RefArray< IgaTarget >& getTargets() const;

	void addPass(IgaPass* pass);

	const RefArray< IgaPass >& getPasses() const;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< IgaTarget > m_targets;
	RefArray< IgaPass > m_passes;
};

	}
}
