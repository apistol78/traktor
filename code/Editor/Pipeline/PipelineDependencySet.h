#pragma once

#include "Editor/IPipelineDependencySet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class T_DLLCLASS PipelineDependencySet : public IPipelineDependencySet
{
	T_RTTI_CLASS;

public:
	virtual uint32_t add(PipelineDependency* dependency) override final;

	virtual uint32_t add(const Guid& dependencyGuid, PipelineDependency* dependency) override final;

	virtual PipelineDependency* get(uint32_t index) const override final;

	virtual uint32_t get(const Guid& dependencyGuid) const override final;

	virtual uint32_t size() const override final;

	virtual void dump(OutputStream& os) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< PipelineDependency > m_dependencies;
	std::map< Guid, uint32_t > m_indices;
};

	}
}

