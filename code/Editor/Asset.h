#pragma once

#include "Core/Io/Path.h"
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

/*! Base class for assets.
 * \ingroup Editor
 *
 * Derive from this class for assets which reference
 * source assets such as image files etc.
 */
class T_DLLCLASS Asset : public ISerializable
{
	T_RTTI_CLASS;

public:
	Asset() = default;

	explicit Asset(const Path& fileName);

	void setFileName(const Path& fileName);

	const Path& getFileName() const;

	virtual void serialize(ISerializer& s) override;

private:
	Path m_fileName;
};

}
