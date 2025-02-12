
#include "tjsCommHead.h"
#include <assert.h>
#include "DebugIntf.h"
#include "MsgIntf.h"
#include "OpenGLHeader.h"

bool _CheckGLErrorAndLog(const tjs_char *file, const int lineno, const tjs_char* funcname) 
{
	GLenum error_code = glGetError();
	if( error_code == GL_NO_ERROR ) return true;
	if( funcname != nullptr ) {
		tjs_char buff[256+1];
		TJS_snprintf( buff, 256, TJS_W("%s:%d %s error:%08x"), file, lineno, funcname, error_code);
		TVPAddLog( buff );
	}
	switch( error_code ) {
	case GL_INVALID_ENUM: TVPAddLog( TJS_W( "GL error : GL_INVALID_ENUM." ) ); break;
	case GL_INVALID_VALUE: TVPAddLog( TJS_W( "GL error : GL_INVALID_VALUE." ) ); break;
	case GL_INVALID_OPERATION: TVPAddLog( TJS_W( "GL error : GL_INVALID_OPERATION." ) ); break;
	case GL_OUT_OF_MEMORY: TVPAddLog( TJS_W( "GL error : GL_OUT_OF_MEMORY." ) ); break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: TVPAddLog( TJS_W( "GL error : GL_INVALID_FRAMEBUFFER_OPERATION." ) ); break;
	default: TVPAddLog( (tjs_string(TJS_W( "GL error code : " )) + to_tjs_string( (unsigned long)error_code )).c_str() ); break;
	}
	return false;
}

static GLADapiproc gladload(const char *name)
{
	return (GLADapiproc)TVPGLGetProcAddress(name);
}

static bool glesInited = false;

int TVPOpenGLESVersion = 200;

int TVPGetOpenGLESVersion() { return TVPOpenGLESVersion; }

void InitGLES()
{
	if (!glesInited) {
		// glad の GLES を初期化
		int gles_version = gladLoadGLES2(gladload);
		if (!gles_version) {
			TVPThrowExceptionMessage(TJS_W("Unable to load glad GLES.\n"));
		}
		TVPOpenGLESVersion = GLAD_VERSION_MAJOR(gles_version) * 100 + GLAD_VERSION_MINOR(gles_version);
		//fprintf(stderr, "Loaded GLES %d.%d.\n",
		//	GLAD_VERSION_MAJOR(gles_version), GLAD_VERSION_MINOR(gles_version));
		glesInited = true;
	}
}
