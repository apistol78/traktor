#ifndef traktor_online_LocalVideoSharing_H
#define traktor_online_LocalVideoSharing_H

#include "Online/Provider/IVideoSharingProvider.h"

namespace traktor
{
	namespace online
	{

class LocalVideoSharing : public IVideoSharingProvider
{
	T_RTTI_CLASS;

public:
	virtual bool beginCapture(int32_t duration);

	virtual void endCapture();

	virtual bool showShareUI();
};

	}
}

#endif	// traktor_online_LocalVideoSharing_H
