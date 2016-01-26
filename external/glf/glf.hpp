#ifndef GLF_WINDOW_INCLUDED
#define GLF_WINDOW_INCLUDED

//#pragma warning(disable : once)

// OpenGL
#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <GL/glew.h>
#	include <GL/wglew.h>
#	include <cl/cl.h>
#	include <cl/cl_ext.h>
#	include <cl/cl_gl.h>
#	include <cl/cl_gl_ext.h>
//#	include <GL/glext.h>
#elif defined(linux) || defined(__linux)
#	include <GL/glew.h>
#	define GL_GLEXT_PROTOTYPES 1
#	include <GL/gl.h>
#	include <GL/glext.h>
#elif defined(__APPLE__)
#	include <OpenGL/gl.h>
#else
#	error "Unsupported platform"
#endif

#include "common.hpp"
#include "vertex.hpp"
#include "compiler.hpp"
#include "sementics.hpp"
#include "error.hpp"

namespace glf
{
	enum mouse_button
	{
		MOUSE_BUTTON_NONE = 0,
		MOUSE_BUTTON_LEFT = (1 << 0),
		MOUSE_BUTTON_RIGHT = (1 << 1),
		MOUSE_BUTTON_MIDDLE = (1 << 2)
	};

	struct window
	{
		window(glm::ivec2 const & Size) :
			Size(Size),
			MouseOrigin(Size >> 1),
			MouseCurrent(Size >> 1),
			TranlationOrigin(0, 4),
			TranlationCurrent(0, 4),
			RotationOrigin(0), 
			RotationCurrent(0),
			MouseButtonFlags(0)
		{
			memset(KeyPressed, 0, sizeof(KeyPressed));	
		}

		glm::ivec2 Size;
		glm::vec2 MouseOrigin;
		glm::vec2 MouseCurrent;
		glm::vec2 TranlationOrigin;
		glm::vec2 TranlationCurrent;
		glm::vec2 RotationOrigin;
		glm::vec2 RotationCurrent;
		int MouseButtonFlags;
		std::size_t KeyPressed[256];
		bool WireFrame;
		bool DrawInputMesh;
		bool AddNoise;
		bool PhongTess;
	};

	int version(int Major, int Minor);
	int run(int argc, char* argv[], glm::ivec2 const & Size, int Profile, int Major, int Minor);

	struct vertexattrib
	{
		vertexattrib() :
			Enabled(GL_FALSE),
			//Binding(0),
			Size(4),
			Stride(0),
			Type(GL_FLOAT),
			Normalized(GL_FALSE),
			Integer(GL_FALSE),
			Long(GL_FALSE),
			Divisor(0),
			Pointer(NULL)
		{}

		vertexattrib
		(
			GLint Enabled,
			//GLint Binding,
			GLint Size,
			GLint Stride,
			GLint Type,
			GLint Normalized,
			GLint Integer,
			GLint Long,
			GLint Divisor,
			GLvoid* Pointer
		) :
			Enabled(Enabled),
			//Binding(Binding),
			Size(Size),
			Stride(Stride),
			Type(Type),
			Normalized(Normalized),
			Integer(Integer),
			Long(Long),
			Divisor(Divisor),
			Pointer(Pointer)
		{}

		GLint Enabled;
		//GLint Binding;
		GLint Size;
		GLint Stride;
		GLint Type;
		GLint Normalized;
		GLint Integer;
		GLint Long;
		GLint Divisor;
		GLvoid* Pointer;
	};

	inline bool operator== (vertexattrib const & A, vertexattrib const & B)
	{
		return A.Enabled == B.Enabled && 
			A.Size == B.Size && 
			A.Stride == B.Stride && 
			A.Type == B.Type && 
			A.Normalized == B.Normalized && 
			A.Integer == B.Integer && 
			A.Long == B.Long;
	}

	inline bool operator!= (vertexattrib const & A, vertexattrib const & B)
	{
		return !(A == B);
	}
}//namespace glf

namespace 
{
	extern glf::window Window;
}//namespace 

namespace amd = glf;

#define GLF_BUFFER_OFFSET(i) ((char *)NULL + (i))
#ifndef GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD
#define GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD 0x9160
#endif

#ifndef WGL_CONTEXT_CORE_PROFILE_BIT_ARB
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#endif

#ifndef WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif

#ifndef WGL_CONTEXT_ES2_PROFILE_BIT_EXT
#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
#endif

#ifndef GLX_CONTEXT_CORE_PROFILE_BIT_ARB
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#endif

#ifndef GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
#define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif

#ifndef WGLX_CONTEXT_ES2_PROFILE_BIT_EXT
#define WGLX_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
#endif

#include "glf.inl"

#endif//GLF_WINDOW_INCLUDED
