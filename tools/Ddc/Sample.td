
import "Test/Standard";
import "Test/Traktor.Core";

namespace test
{

class Snorre
{
	std.wstring tag;
}

class Snorre2 : Snorre
{
	std.wstring tag2;
}

class Balle
{
	int32 first;
	int32[] second;
	std.vector< int32 > third;
	std.vector< std.vector< int32 > > fourth;
	traktor.Ref< Snorre2 > fifth;
}

}
