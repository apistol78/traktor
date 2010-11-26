
namespace traktor
{

alias "cpp" Vector4
{
	"traktor::Vector4",
	"const traktor::Vector4&",
	"traktor::Member< Vector4 >"
}

alias "cpp" Vector4[]
{
	"traktor::AlignedVector< traktor::Vector4 >",
	"const traktor::AlignedVector< traktor::Vector4 >&",
	"traktor::MemberAlignedVector< Vector4 >"
}

alias "cpp" Ref< A >
{
	"traktor::Ref< %A.type >",
	"const traktor::Ref< %A.type >&",
	"traktor::MemberRef< %A.type >"
}

alias "cpp" Ref< A >[]
{
	"traktor::RefArray< %A.type >",
	"const traktor::RefArray< %A.type >&",
	"traktor::MemberRefArray< %A.type >"
}

}
