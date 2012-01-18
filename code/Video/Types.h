#ifndef traktor_video_Types_H
#define traktor_video_Types_H

namespace traktor
{
	namespace video
	{

/*! \brief Video formats.
 * \ingroup Video
 */
enum VideoFormat
{
	VfUnknown = 0,
	VfHalfXYChroma = 1,
	VfHalfXChroma = 2,
	VfFullChroma = 3
};

	}
}

#endif	// traktor_video_Types_H
