#ifndef traktor_editor_ThumbnailGenerator_H
#define traktor_editor_ThumbnailGenerator_H

#include "Editor/IThumbnailGenerator.h"

namespace traktor
{
	namespace editor
	{

class ThumbnailGenerator : public IThumbnailGenerator
{
	T_RTTI_CLASS;

public:
	ThumbnailGenerator(const Path& thumbsPath);

	virtual Ref< drawing::Image > get(const Path& fileName, int32_t width, int32_t height, bool visibleAlpha) T_OVERRIDE T_FINAL;

private:
	Path m_thumbsPath;
};

	}
}

#endif	// traktor_editor_ThumbnailGenerator_H
