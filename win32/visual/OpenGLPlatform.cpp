#include "tjsCommHead.h"
#include "DebugIntf.h"
#include "Application.h"
#include "FilePathUtil.h"
#include "MsgIntf.h"	// TVPThrowExceptionMessage
#include "SysInitIntf.h"
#include "EGLContext.h"

static HMODULE TVPhModuleLibEGL = nullptr;

typedef void *GetProcAddressFunc(const char * procname);
static GetProcAddressFunc * _eglGetProcAddress = nullptr;

//---------------------------------------------------------------------------
static void TVPUninitializeOpenGLPlatform() {
	if( TVPhModuleLibEGL ) {
		::FreeLibrary( TVPhModuleLibEGL );
		TVPhModuleLibEGL = nullptr;
	}
}
//---------------------------------------------------------------------------
static tTVPAtExit TVPUninitANGLEAtExit
	(TVP_ATEXIT_PRI_SHUTDOWN, TVPUninitializeOpenGLPlatform);
//---------------------------------------------------------------------------
static bool TVPInitializeOpenGLPlatform() {
	if ( !TVPhModuleLibEGL ) {

		HMODULE hModule = ::LoadLibrary( TJS_W("libEGL.dll") );
		if( !hModule ) {
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ::GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, NULL );
			TVPAddLog( (tjs_char*)lpMsgBuf );
			::LocalFree( lpMsgBuf );
			return false;
		}

		TVPhModuleLibEGL = hModule;

		if (!(_eglGetProcAddress = (GetProcAddressFunc*)::GetProcAddress(hModule, "eglGetProcAddress"))) {
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ::GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, NULL );
			TVPAddLog( (tjs_char*)lpMsgBuf );
			::LocalFree( lpMsgBuf );
			TVPUninitializeOpenGLPlatform();
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------------
void* TVPGLGetProcAddress(const char * procname) {
	return _eglGetProcAddress ? _eglGetProcAddress(procname) : nullptr;
}

void *TVPGLGetNativeDisplay(void *nativeWindow)
{
	return (void*)::GetDC((HWND)nativeWindow);
}

void TVPGLReleaseNativeDisplay(void *nativeWindow, void *nativeDisplay)
{
	::ReleaseDC((HWND)nativeWindow, (HDC)nativeDisplay);
}

//---------------------------------------------------------------------------

static bool oglInited = false;

iTVPGLContext *
iTVPGLContext::GetContext(void *nativeWindow)
{
	if (!oglInited) {
		if (!TVPInitializeOpenGLPlatform()) {
			TVPThrowExceptionMessage( TJS_W("Unable to initialize OpenGL.") );
		}
		tTVPEGLContext::InitEGL();
		oglInited = true;
	}
	auto ctx = tTVPEGLContext::GetContext(nativeWindow);
	if( !ctx ) TVPThrowExceptionMessage( TJS_W("Cannot create low level graphics system(maybe low memory).") );
	return ctx;
}
