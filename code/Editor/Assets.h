#pragma once

#include <vector>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

/*! Root assets.
 * \ingroup Editor
 */
class T_DLLCLASS Assets : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override;

private:
	friend class AssetsPipeline;

	struct Dependency
	{
		Guid id;
		bool editorDeployOnly = false;	//!< Only built when deployed from editor.

		void serialize(ISerializer& s);
	};

	std::vector< Dependency > m_dependencies;
};

}
