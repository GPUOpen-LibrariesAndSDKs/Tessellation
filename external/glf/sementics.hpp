#ifndef GLF_SEMENTICS_INCLUDED
#define GLF_SEMENTICS_INCLUDED

namespace glf{
namespace semantic
{
	namespace uniform
	{
		enum type
		{
			MATERIAL = 0,
			TRANSFORM0 = 1,
			TRANSFORM1 = 2
		};
	};

	namespace image
	{
		enum type
		{
			DIFFUSE = 0,
			PICKING = 1
		};
	}//namesapce image

	namespace attr
	{
		enum type
		{
			POSITION = 0,
			COLOR	 = 3,
			TEXCOORD = 4
		};
	}//namespace attr

	namespace vert
	{
		enum type
		{
			POSITION = 0,
			COLOR	 = 3,
			TEXCOORD = 4,
			INSTANCE = 7
		};
	}//namespace vert

	namespace frag
	{
		enum type
		{
			COLOR	= 0,
			RED		= 0,
			GREEN	= 1,
			BLUE	= 2,
			ALPHA	= 0
		};
	}//namespace frag

	namespace renderbuffer
	{
		enum type
		{
			DEPTH,
			COLOR0
		};
	}//namespace renderbuffer

}//namespace semantic
}//namespace glf

#endif//GLF_SEMENTICS_INCLUDED
