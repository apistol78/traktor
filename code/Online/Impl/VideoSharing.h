#ifndef traktor_online_VideoSharing_H
#define traktor_online_VideoSharing_H

#include "Online/IVideoSharing.h"

namespace traktor
{
	namespace online
	{

class IVideoSharingProvider;

class VideoSharing : public IVideoSharing
{
	T_RTTI_CLASS;

public:
	virtual bool beginCapture(int32_t duration);

	virtual void endCapture(const PropertyGroup* metaData);

	virtual bool isCapturing() const;

	virtual bool showShareUI();

private:
	friend class SessionManager;

	Ref< IVideoSharingProvider > m_provider;
	bool m_capturing;

	VideoSharing(IVideoSharingProvider* provider);
};

	}
}

#endif	// traktor_online_VideoSharing_H
