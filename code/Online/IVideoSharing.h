#ifndef traktor_online_IVideoSharing_H
#define traktor_online_IVideoSharing_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class T_DLLCLASS IVideoSharing : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool beginCapture(int32_t duration) = 0;

	virtual void endCapture() = 0;

	virtual bool isCapturing() const = 0;

	virtual bool showShareUI() = 0;
};

	}
}

#endif	// traktor_online_IVideoSharing_H
