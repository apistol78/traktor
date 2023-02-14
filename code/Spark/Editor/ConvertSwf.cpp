#include "Spark/Editor/ConvertSwf.h"
#include "Spark/Swf/SwfMovieFactory.h"
#include "Spark/Swf/SwfReader.h"

namespace traktor::spark
{

Ref< Movie > convertSwf(const db::Instance* sourceInstance, IStream* sourceStream)
{
	Ref< SwfReader > swf = new SwfReader(sourceStream);
	return SwfMovieFactory().createMovie(swf);
}

}
