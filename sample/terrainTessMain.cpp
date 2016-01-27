
//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <glf/glf.hpp>
#include <gli/gli.hpp>
#include <gli/gtx/loader.hpp>
#include <gli/gtx/gl_texture2d.hpp>

namespace MySemantic
{
	namespace attr
	{
		//each vertices is defined by:
		enum type
		{
			IN_POSITION   = 0,//position
			IN_TEXCOORD   = 1,//texture coordinate
		};
	}
}
namespace
{
	struct VERTEX_ELEMENT
	{
		glm::vec2 positionMS;//position Model Space
		glm::vec2 textureCoor;//texture coordinate
	};
	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,

			MAX
		};
	}//namespace buffer

	extern bool AddNoise;
	std::string const SAMPLE_NAME = "OpenGL Terrain Tessellation";	
	std::string const SAMPLE_VERTEX_SHADER(amd::DATA_DIRECTORY + "tess.vert");
	std::string const SAMPLE_CONTROL_SHADER(amd::DATA_DIRECTORY + "tess.cont");
	std::string const SAMPLE_EVALUATION_SHADER(amd::DATA_DIRECTORY + "tess.eval");
	std::string const SAMPLE_FRAGMENT_SHADER(amd::DATA_DIRECTORY + "tess.frag");
	int const SAMPLE_SIZE_WIDTH(800);
	int const SAMPLE_SIZE_HEIGHT(600);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	amd::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const size = 161; 
	glm::ivec2 scalarSize(5, 160);

	GLsizei const IndexCount((size-1)*(size-1)*4);
	GLuint indexBuffer[IndexCount];

	GLuint BufferName[buffer::MAX] = {0};

	GLuint ProgramName(0);
	GLuint VertexArrayName(0);

	GLint UniformScalar(0);
	GLint UniformSize(0);
	GLint UniformIsNoise(0);
	GLint UniformPhongTess(0);
	GLint UniformMVP(0);
	GLint UniformView(0);
	GLint UniformProj(0);
	GLuint oglDisplacementMap(0);
	GLuint oglDiffuseMap(0);
	GLuint oglNoiseMap(0);
	GLuint oglDetailMap(0);

	// used for pre-processing of the displacement map to generate Normal from heightmap
	// output will be RGB(Normal)A(displacement)
	GLuint ProgramComputeNormal(0);
	std::string const COMPUTE_NORMAL_VERTEX_SHADER(amd::DATA_DIRECTORY + "computenormal.vert");
	std::string const COMPUTE_NORMAL_FRAGMENT_SHADER(amd::DATA_DIRECTORY + "computenormal.frag");
	int fboWidth(0);
	int fboHeight(0);
	GLuint fbo(0);
	GLuint texFbo(0);
	bool fboGood(false);
	// screenquad rendering
	GLuint quadVA(0);
	GLuint quadVBV(0);
	GLuint quadVBT(0);

	//shader for blurring
	GLuint ProgramBlur(0);
	GLuint texFbo2(0);
	std::string const BLUR_VERTEX_SHADER(amd::DATA_DIRECTORY + "blur.vert");
	std::string const BLUR_FRAGMENT_SHADER(amd::DATA_DIRECTORY + "blur.frag");
	GLint f_numBlurPixelsPerSide;
	GLint vec2_blurMultiplyVec;

	//camera movement
	glm::vec3 vPos(0);
	float xrot = 0, yrot = 0;
	int lastx = 0, lasty = 0;
	float deltaxrot = 0;
	float deltayrot = 0;
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;

	int currentTime = 0;
	int lastTime = 0;
	int elapsedTime = 0;
	bool waitForProcessMovement = false;
	bool mouseLook = true;

	glm::vec4 vLeft(0);
	glm::vec4 vForward(0);

	gli::texture2D TextureHeightMap; //keep it local to get height CPU side for camera constraint.

	//lighting
	glm::vec3 lightDir(0,-1,0);
	GLint UniformLightDir(0);

}//namespace

float GetHeightAt(float x, float z)
{
	int xx = int(-x*(float)fboWidth/(float)(scalarSize.x*(size-1))+0.5);
	int zz = fboHeight - int(-z*(float)fboHeight/(float)(scalarSize.x*(size-1))+0.5);
	if (xx < 0)
		xx = 0;
	if (zz < 0)
		zz = 0;
	if (xx > fboWidth)
		xx = fboWidth;
	if (zz > fboHeight)
		zz = fboHeight;

	int val = *(TextureHeightMap[0].data()+xx +zz*fboHeight);
	float yy = (float)val * (float)scalarSize.y / 256.0f;
//	fprintf(stdout,"X:%d Y:%d Z:%d\n", xx, val, zz);

	return yy;
}

bool initTexture()
{
	bool Validated(true);

	oglDisplacementMap = gli::createTexture2D(amd::DATA_DIRECTORY + "terrain_heightmap.dds" );
	TextureHeightMap = gli::load(amd::DATA_DIRECTORY + "terrain_heightmap.dds");
	
	oglDiffuseMap = gli::createTexture2D(amd::DATA_DIRECTORY + "terrain_texture8k_unlit.dds" );
	glBindTexture(GL_TEXTURE_2D, oglDiffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	oglNoiseMap = gli::createTexture2D(amd::DATA_DIRECTORY + "noise.dds" );
	glBindTexture(GL_TEXTURE_2D, oglNoiseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	oglDetailMap = gli::createTexture2D(amd::DATA_DIRECTORY + "detail.dds" );
	glBindTexture(GL_TEXTURE_2D, oglDetailMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	// texture for color fbo attachment
	glGenTextures(1, &texFbo);
	glBindTexture(GL_TEXTURE_2D, texFbo);
	fboWidth = TextureHeightMap[0].dimensions().x;
	fboHeight = TextureHeightMap[0].dimensions().y;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);


	// texture for color fbo attachment to blur
	glGenTextures(1, &texFbo2);
	glBindTexture(GL_TEXTURE_2D, texFbo2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	vPos = glm::vec3(-202, -171, -205);
	xrot = 24.0f;
	yrot = -198.0f;
	return Validated;
}

bool initProgram()
{
	bool Validated = true;

	if(Validated)
	{
		ProgramName = glCreateProgram();
		GLuint VertexShader = amd::createShader(GL_VERTEX_SHADER, SAMPLE_VERTEX_SHADER);
		GLuint ControlShader = amd::createShader(GL_TESS_CONTROL_SHADER, SAMPLE_CONTROL_SHADER);
		GLuint EvaluationShader = amd::createShader(GL_TESS_EVALUATION_SHADER, SAMPLE_EVALUATION_SHADER);
		GLuint FragmentShader = amd::createShader(GL_FRAGMENT_SHADER, SAMPLE_FRAGMENT_SHADER);

		glAttachShader(ProgramName, VertexShader);
		glAttachShader(ProgramName, ControlShader);
		glAttachShader(ProgramName, EvaluationShader);
		glAttachShader(ProgramName, FragmentShader);
		glDeleteShader(VertexShader);
		glDeleteShader(ControlShader);
		glDeleteShader(EvaluationShader);
		glDeleteShader(FragmentShader);
		glLinkProgram(ProgramName);
		Validated = amd::checkProgram(ProgramName);

		if(Validated)
		{
			UniformScalar = glGetUniformLocation(ProgramName, "scalar");
			UniformSize = glGetUniformLocation(ProgramName, "size");
			UniformIsNoise = glGetUniformLocation(ProgramName, "isNoise");
			UniformPhongTess = glGetUniformLocation(ProgramName, "applyPhongTess");
			UniformMVP = glGetUniformLocation(ProgramName, "matMVP");
			UniformView = glGetUniformLocation(ProgramName, "matView");
			UniformProj = glGetUniformLocation(ProgramName, "matProj");
			UniformLightDir = glGetUniformLocation(ProgramName, "vLight");
		}
		glUseProgram(ProgramName);
		glUniform2iv(UniformScalar, 1, &scalarSize[0]);
		glUniform1i(UniformSize, size);
	}

	if(Validated)
	{
		ProgramComputeNormal = glCreateProgram();
		GLuint VertexShader = amd::createShader(GL_VERTEX_SHADER, COMPUTE_NORMAL_VERTEX_SHADER);
		GLuint FragmentShader = amd::createShader(GL_FRAGMENT_SHADER, COMPUTE_NORMAL_FRAGMENT_SHADER);
		glAttachShader(ProgramComputeNormal, VertexShader);
		glAttachShader(ProgramComputeNormal, FragmentShader);
		glDeleteShader(VertexShader);
		glDeleteShader(FragmentShader);
		glLinkProgram(ProgramComputeNormal);
		Validated = amd::checkProgram(ProgramComputeNormal);
		glUseProgram(ProgramComputeNormal);
		GLint scalar = glGetUniformLocation(ProgramComputeNormal, "scalar");
		glUniform2iv(scalar, 1, &scalarSize[0]);
	}

	if(Validated)
	{
		ProgramBlur = glCreateProgram();
		GLuint VertexShader = amd::createShader(GL_VERTEX_SHADER, BLUR_VERTEX_SHADER);
		GLuint FragmentShader = amd::createShader(GL_FRAGMENT_SHADER, BLUR_FRAGMENT_SHADER);
		glAttachShader(ProgramBlur, VertexShader);
		glAttachShader(ProgramBlur, FragmentShader);
		glDeleteShader(VertexShader);
		glDeleteShader(FragmentShader);
		glLinkProgram(ProgramBlur);
		Validated = amd::checkProgram(ProgramBlur);
		if(Validated)
		{
			f_numBlurPixelsPerSide = glGetUniformLocation(ProgramBlur, "numBlurPixelsPerSide");
			vec2_blurMultiplyVec   = glGetUniformLocation(ProgramBlur, "blurMultiplyVec");
			GLint blurSize = glGetUniformLocation(ProgramComputeNormal, "blurSize");
			glUniform1f(blurSize, 1.0f/(float)fboWidth);
		}
	}


	unsigned int patchCount=0;
	for (unsigned int i=0; i<size-1; i++)
	{
		for (unsigned int j=0; j<size-1; j++)
		{
			indexBuffer[4*patchCount] = i*size+j;
			indexBuffer[4*patchCount+1] = i*size+j+1;
			indexBuffer[4*patchCount+2] = (i+1)*size+j+1;
			indexBuffer[4*patchCount+3] = (i+1)*size+j;
			patchCount++;
		}
	}

	return Validated && amd::checkError("initProgram");
}

bool initBuffer()
{
	glGenBuffers(buffer::MAX, BufferName);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexBuffer) , indexBuffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Setup our FBO with texture texFbo for color attachment
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texFbo, 0);
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	fboGood = (status == GL_FRAMEBUFFER_COMPLETE_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	//setup our vertex array to render a fullscreen quad
	glGenVertexArrays(1, &quadVA);
	glBindVertexArray(quadVA);

	glGenBuffers(1, &quadVBV);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBV);
	float* vert = new float[9];	// vertex array
	vert[0] = -1.0f; vert[1] = -1.0f;
	vert[2] =  1.0;  vert[3] = -1.0f;
	vert[4] = -1.0f; vert[5] =  1.0f;
	vert[6] =  1.0f; vert[7] =  1.0f;
	glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat), vert, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0); 

	// resuse vert for texcoord
	glGenBuffers(1, &quadVBT);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBT);
	vert[0] = 0.0f; vert[1] = 0.0f;
	vert[2] =  1.0;  vert[3] = 0.0f;
	vert[4] = 0.0f; vert[5] =  1.0f;
	vert[6] =  1.0f; vert[7] =  1.0f;
	glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat), vert, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0); 

	glBindVertexArray(0);
	delete vert;

	return amd::checkError("initBuffer");
}

bool initDebugOutput()
{
	bool Validated(true);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&amd::debugOutput, NULL);

	return Validated;
}

bool computeNormal()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glViewport(0, 0, fboWidth, fboHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, oglDisplacementMap);	// as input

	glUseProgram(ProgramComputeNormal);


	// render screen quad
	glBindVertexArray(quadVA);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glUseProgram(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glFlush();	// sync to make sure tetxure result is available for next step

	return amd::checkError("computeNormal");
}

bool blurNormal()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glViewport(0, 0, fboWidth, fboHeight);

	//output is texFbo2 for horizontal pass
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texFbo2, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texFbo);	// as input

	glUseProgram(ProgramBlur);

	GLfloat f = 3.0f;
	glUniform1fv(f_numBlurPixelsPerSide, 1, &f);
	GLfloat fh[2] = {1.0f, 0.0f};
	GLfloat fv[2] = {0.0f, 1.0f};
	glUniform2fv(vec2_blurMultiplyVec, 1, &fh[0]);

	// render screen quad horizontal pass
	glBindVertexArray(quadVA);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	// vertical pass: switch texFbo and texFbo2
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texFbo, 0);	// output texture
	glBindTexture(GL_TEXTURE_2D, texFbo2);	// input texture
	glUniform2fv(vec2_blurMultiplyVec, 1, &fv[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glFlush();	// sync to make sure tetxure result is available for next step

	return amd::checkError("blurNormal");
}

bool begin()
{
	bool Validated = amd::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated && amd::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initTexture();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();

	if(Validated)
		Validated = computeNormal();
	if(Validated)
		Validated = blurNormal();

	// Set initial rendering states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	return Validated && amd::checkError("begin");
}

bool end()
{
	glutSetCursor(GLUT_CURSOR_INHERIT); 

	glDeleteVertexArrays(1, &VertexArrayName);
	glDeleteProgram(ProgramName);
	glDeleteTextures(1, &oglDisplacementMap);
	glDeleteTextures(1, &oglDiffuseMap);
	glDeleteTextures(1, &oglNoiseMap);

	glDeleteTextures(1, &texFbo);
	glDeleteTextures(1, &texFbo2);
	glDeleteFramebuffersEXT(1, &fbo);

	glDeleteBuffers(1, &quadVBV);
	glDeleteBuffers(1, &quadVBT);
	glDeleteVertexArrays(1, &quadVA);

	return amd::checkError("end");
}

void pressKey(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT : moveLeft = true; break;
	case GLUT_KEY_RIGHT : moveRight = true; break;
	case GLUT_KEY_UP : moveUp = true; break;
	case GLUT_KEY_DOWN : moveDown = true; break;
	}
}

void releaseKey(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT : moveLeft = false; break;
	case GLUT_KEY_RIGHT : moveRight = false; break;
	case GLUT_KEY_UP : moveUp = false; break;
	case GLUT_KEY_DOWN : moveDown = false; break;
	}

}

void processMovement(int elapsedTime)
{
    //printf("%s elapsedTime=%d\n", __FUNCTION__, elapsedTime);

	glm::vec4 deltaPos(0);

	float mouseSpeedFactor = (float)elapsedTime*0.15f;
	float keySpeedFactor = (float)elapsedTime*0.1f;

	// process mouse rotation
	xrot += deltaxrot*mouseSpeedFactor;
	yrot += deltayrot*mouseSpeedFactor;
	deltaxrot = 0;
	deltayrot = 0;

	//process key movement
	if (moveUp)
		deltaPos += vForward;

	if (moveDown)
		deltaPos -= vForward;

	if (moveRight)
		deltaPos -= vLeft;

	if (moveLeft)
		deltaPos += vLeft;

	vPos += glm::vec3(deltaPos)*keySpeedFactor;
	float y = GetHeightAt(vPos.x, vPos.z)+3.0f;
	if (-vPos.y < y)
		vPos.y = -y;
	waitForProcessMovement = false;
}


void fps_keyboard (unsigned char key, int x, int y) 
{
    printf("%s key=%d\n", __FUNCTION__, (int)key);

	switch(key) 
	{
	case ' ':
		mouseLook = !mouseLook;
		if (mouseLook)
		{
			int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2; 
			int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2; 
			glutSetCursor(GLUT_CURSOR_NONE); 
			glutWarpPointer(centerX, centerY); 
			waitForProcessMovement = true;
		}
		else
		{
			glutSetCursor(GLUT_CURSOR_INHERIT); 
		}
		break;
	}

	amd::keyboard(key, x, y);
}

#define MOUSEMOVEMAX	10
void fps_mouseMovement(int x, int y)
{
    printf("%s mouseLook=%d waitForProcessMovement=%d\n", __FUNCTION__, mouseLook, waitForProcessMovement);

	if (!mouseLook || waitForProcessMovement)
		return;

	static int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2; 
	static int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2; 

	deltaxrot = (float)(y - centerY); 
	deltayrot = (float)(x - centerX); 

	if(deltaxrot != 0.0f || deltayrot != 0.0f)
	{ 
		waitForProcessMovement = true;
		glutWarpPointer(centerX, centerY); 
	} 

	if (deltaxrot > MOUSEMOVEMAX)
		deltaxrot = MOUSEMOVEMAX;
	else if (deltaxrot < -MOUSEMOVEMAX)
		deltaxrot = -MOUSEMOVEMAX;

	if (deltayrot > MOUSEMOVEMAX)
		deltayrot = MOUSEMOVEMAX;
	else if (deltayrot < -MOUSEMOVEMAX)
		deltayrot = -MOUSEMOVEMAX;
}

void setup()
{
	glutPassiveMotionFunc(fps_mouseMovement); //check for mouse movement
	glutSpecialFunc(pressKey);
	glutIgnoreKeyRepeat(1);
	glutSpecialUpFunc(releaseKey);
	glutKeyboardFunc(fps_keyboard);
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	lastTime = currentTime;

	int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2; 
	int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2; 
	glutWarpPointer(centerX, centerY); 
	glutSetCursor(GLUT_CURSOR_NONE); 
}

void display()
{
	currentTime = glutGet(GLUT_ELAPSED_TIME); 
	elapsedTime = currentTime - lastTime;
	lastTime = currentTime;

	processMovement(1);//elapsedTime);

	glViewport(0, 0, Window.Size.x, Window.Size.y);
	//clear back buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (Window.WireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texFbo);	// use texture with normal and displacement as alpha
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, oglDiffuseMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, oglNoiseMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, oglDetailMap);
	glActiveTexture(GL_TEXTURE0);

	// Update the transformation matrix
	float yScale = 1.0f / tanf( (3.14f / 4.0f)   /2.0f);
	float xScale = yScale / (Window.Size.x / Window.Size.y);
	float zNear = 0.01f;
	float zFar = 1000.0f;

	glm::mat4 Projection;
	Projection[0] = glm::vec4(xScale,      0.0f,       0.0f,                     0.0f);
	Projection[1] = glm::vec4(0.0f,        yScale,     0.0f,                     0.0f);
	Projection[2] = glm::vec4(0.0f,        0.0f,       zFar/(zNear-zFar),        -1.0f);
	Projection[3] = glm::vec4(0.0f,        0.0f,       -zNear*zFar/(zFar-zNear), 0.0f);

	glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), xrot, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 ViewRotateY = glm::rotate(ViewRotateX, yrot, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 View = glm::translate(ViewRotateY, vPos);
	glm::mat4 Model = glm::mat4(1.0f); // object space and world space are the same here

	glm::mat4 matMVP = Projection * View * Model; 
	glm::mat4 matView = View;
	glm::mat4 matProj = Projection;

	vLeft = glm::vec4(1.f, 0.f, 0.f, 0.f) *  matView;
	vForward = glm::vec4(0.f, 0.f, 1.f, 0.f) * matView;

	glUseProgram(ProgramName);
	glUniform1i(UniformIsNoise, 0);
	glUniform1i(UniformPhongTess, 0);	
	if (Window.AddNoise) 
		glUniform1i(UniformIsNoise, 1);
	if (Window.PhongTess) 
		glUniform1i(UniformPhongTess, 1);

	glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &matMVP[0][0]);
	glUniformMatrix4fv(UniformView, 1, GL_FALSE, &matView[0][0]);
	glUniformMatrix4fv(UniformProj, 1, GL_FALSE, &matProj[0][0]);

	lightDir = glm::normalize(glm::vec3(-1, 1,0.2));
	glUniform3fv(UniformLightDir, 1, &lightDir[0]);

    static bool flag = false;
    if (!flag)
    {
        flag = true;
        static GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glDrawElements(GL_PATCHES, sizeof(indexBuffer) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

	amd::checkError("display");
	amd::swapBuffers();
}

int main(int argc, char* argv[])
{
	return amd::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 8, 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 
		::SAMPLE_MAJOR_VERSION, ::SAMPLE_MINOR_VERSION);
}
