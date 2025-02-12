#include "tjsCommHead.h"
#include <assert.h>
#include "MsgIntf.h"
#include "DebugIntf.h"
#include "EglContext.h"


#ifndef EGL_ANGLE_software_display
#define EGL_ANGLE_software_display 1
#define EGL_SOFTWARE_DISPLAY_ANGLE ((EGLNativeDisplayType)-1)
#endif /* EGL_ANGLE_software_display */

#ifndef EGL_ANGLE_direct3d_display
#define EGL_ANGLE_direct3d_display 1
#define EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE ((EGLNativeDisplayType)-2)
#define EGL_D3D11_ONLY_DISPLAY_ANGLE ((EGLNativeDisplayType)-3)
#endif /* EGL_ANGLE_direct3d_display */

#ifndef EGL_ANGLE_surface_d3d_texture_2d_share_handle
#define EGL_ANGLE_surface_d3d_texture_2d_share_handle 1
#endif /* EGL_ANGLE_surface_d3d_texture_2d_share_handle */

#ifndef EGL_ANGLE_surface_d3d_render_to_back_buffer
#define EGL_ANGLE_surface_d3d_render_to_back_buffer 1
#define EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER 0x320B
#define EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER 0x320C
#endif /* EGL_ANGLE_surface_d3d_render_to_back_buffer */

#ifndef EGL_ANGLE_direct_composition
#define EGL_ANGLE_direct_composition 1
#define EGL_DIRECT_COMPOSITION_ANGLE 0x33A5
#endif /* EGL_ANGLE_direct_composition */

#ifndef EGL_ANGLE_platform_angle
#define EGL_ANGLE_platform_angle 1
#define EGL_PLATFORM_ANGLE_ANGLE          0x3202
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE     0x3203
#define EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE 0x3204
#define EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE 0x3205
#define EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE 0x3206
#endif /* EGL_ANGLE_platform_angle */

#ifndef EGL_ANGLE_platform_angle_d3d
#define EGL_ANGLE_platform_angle_d3d 1
#define EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE 0x3207
#define EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE 0x3208
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE 0x3209
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE 0x320A
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE 0x320B
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE 0x320C
#define EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE 0x320F
#endif /* EGL_ANGLE_platform_angle_d3d */

#ifndef EGL_ANGLE_platform_angle_opengl
#define EGL_ANGLE_platform_angle_opengl 1
#define EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE 0x320D
#define EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE 0x320E
#endif /* EGL_ANGLE_platform_angle_opengl */

#ifndef EGL_ANGLE_platform_angle_null
#define EGL_ANGLE_platform_angle_null 1
#define EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE 0x33AE
#endif /* EGL_ANGLE_platform_angle_null */

#ifndef EGL_ANGLE_window_fixed_size
#define EGL_ANGLE_window_fixed_size 1
#define EGL_FIXED_SIZE_ANGLE              0x3201
#endif /* EGL_ANGLE_window_fixed_size */

#ifndef EGL_ANGLE_x11_visual
#define EGL_ANGLE_x11_visual
#define EGL_X11_VISUAL_ID_ANGLE 0x33A3
#endif /* EGL_ANGLE_x11_visual */

#ifndef EGL_ANGLE_flexible_surface_compatibility
#define EGL_ANGLE_flexible_surface_compatibility 1
#define EGL_FLEXIBLE_SURFACE_COMPATIBILITY_SUPPORTED_ANGLE 0x33A6
#endif /* EGL_ANGLE_flexible_surface_compatibility */

#ifndef EGL_ANGLE_surface_orientation
#define EGL_ANGLE_surface_orientation
#define EGL_OPTIMAL_SURFACE_ORIENTATION_ANGLE 0x33A7
#define EGL_SURFACE_ORIENTATION_ANGLE 0x33A8
#define EGL_SURFACE_ORIENTATION_INVERT_X_ANGLE 0x0001
#define EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE 0x0002
#endif /* EGL_ANGLE_surface_orientation */

#ifndef EGL_ANGLE_experimental_present_path
#define EGL_ANGLE_experimental_present_path
#define EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE 0x33A4
#define EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE 0x33A9
#define EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE 0x33AA
#endif /* EGL_ANGLE_experimental_present_path */


#ifndef  NDEBUG

bool _CheckEGLErrorAndLog() 
{
	GLenum error_code = eglGetError();
	if( error_code == EGL_SUCCESS ) return true;
	switch( error_code ) {
	case EGL_BAD_DISPLAY: TVPAddLog( TJS_W( "Display is not an EGL display connection." ) ); break;
	case EGL_BAD_ATTRIBUTE: TVPAddLog( TJS_W( "Attribute_list contains an invalid frame buffer configuration attribute or an attribute value that is unrecognized or out of range." ) ); break;
	case EGL_NOT_INITIALIZED: TVPAddLog( TJS_W( "Display has not been initialized." ) ); break;
	case EGL_BAD_PARAMETER: TVPAddLog( TJS_W( "Num_config is NULL." ) ); break;
	case EGL_SUCCESS: TVPAddLog( TJS_W( "The last function succeeded without error." ) ); break;
break;
	case EGL_BAD_ACCESS: TVPAddLog( TJS_W( "EGL cannot access a requested resource( for example a context is bound in another thread )." ) ); break;
	case EGL_BAD_ALLOC: TVPAddLog( TJS_W( "EGL failed to allocate resources for the requested operation." ) ); break;
	case EGL_BAD_CONTEXT: TVPAddLog( TJS_W( "An EGLContext argument does not name a valid EGL rendering context." ) ); break;
	case EGL_BAD_CONFIG: TVPAddLog( TJS_W( "An EGLConfig argument does not name a valid EGL frame buffer configuration." ) ); break;
	case EGL_BAD_CURRENT_SURFACE: TVPAddLog( TJS_W( "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid." ) ); break;
	case EGL_BAD_SURFACE: TVPAddLog( TJS_W( "An EGLSurface argument does not name a valid surface( window, pixel buffer or pixmap ) configured for GL rendering." ) ); break;
	case EGL_BAD_MATCH: TVPAddLog( TJS_W( "Arguments are inconsistent( for example, a valid context requires buffers not supplied by a valid surface )." ) ); break;
	case EGL_BAD_NATIVE_PIXMAP: TVPAddLog( TJS_W( "A NativePixmapType argument does not refer to a valid native pixmap." ) ); break;
	case EGL_BAD_NATIVE_WINDOW: TVPAddLog( TJS_W( "A NativeWindowType argument does not refer to a valid native window." ) ); break;
	case EGL_CONTEXT_LOST: TVPAddLog( TJS_W( "A power management event has occurred.The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering." ) ); break;
	default: TVPAddLog( (tjs_string(TJS_W( "ANGLE Error : " )) + to_tjs_string( (unsigned long)error_code )).c_str() ); break;
	}
	return false;
}

#define CheckEGLErrorAndLog() _CheckEGLErrorAndLog()
#else
#define CheckEGLErrorAndLog() (1)
#endif

extern int TVPOpenGLESVersion;

extern void *TVPGLGetNativeDisplay(void *nativeWindow);
extern void TVPGLReleaseNativeDisplay(void *nativeWindow, void *nativeDisplay);

static GLADapiproc gladload(const char *name)
{
	return (GLADapiproc)TVPGLGetProcAddress(name);
}

std::vector<tTVPEGLContext*> contexts;

void
tTVPEGLContext::Remove(tTVPEGLContext *context)
{
	auto it = std::find( contexts.begin(), contexts.end(), context );
	if( it != contexts.end() ) {
		contexts.erase( it );
	}
}

iTVPGLContext *
tTVPEGLContext::GetContext(void *nativeWindow)
{
	for( auto context : contexts ) {
		if( context->NativeWindow() == nativeWindow ) {
			context->AddRef();
			return context;
		}
	}
	tTVPEGLContext *context = new tTVPEGLContext( (EGLNativeWindowType)nativeWindow );
	if (context->Initialize() ) {
		contexts.push_back( context );
		return context;
	}
	delete context;
	return nullptr;
}


tTVPEGLContext::tTVPEGLContext(EGLNativeWindowType nativeWindow) 
: mNativeWindow(nativeWindow),
mNativeDisplay(nullptr),
mDisplay(EGL_NO_DISPLAY),
mSurface(EGL_NO_SURFACE),
mContext(EGL_NO_CONTEXT),
mSwapInterval(0),
mRedBits(-1),
mGreenBits(-1),
mBlueBits(-1),
mAlphaBits(-1),
mDepthBits(-1),
mStencilBits(-1),
mMultisample(false),
mMinSwapInterval(1),
mMaxSwapInterval(1),
mRefCount(1)
{
	mNativeDisplay = ::TVPGLGetNativeDisplay(mNativeWindow);
}

tTVPEGLContext::~tTVPEGLContext()
{
	Destroy();
	if( mNativeDisplay ) {
		::TVPGLReleaseNativeDisplay( mNativeWindow, mNativeDisplay );
		mNativeDisplay = 0;
	}
	Remove(this);
}

int
tTVPEGLContext::AddRef()
{
	return ++mRefCount;
}

int
tTVPEGLContext::Release()
{
	if( --mRefCount == 0 ) {
		delete this;
		return 0;
	}
	return mRefCount;
}

// 最小構成での初期化を試す
// OpenGL ES2.0 環境を試す、これで動かないとたぶんもう無理
bool tTVPEGLContext::TryMinimumLevelInitialize() 
{
	EGLint displayAttributes[] = { EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE, EGL_NONE };
	mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, mNativeDisplay, displayAttributes );
	EGLint majorVersion, minorVersion;
	if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
		TVPAddLog( TJS_W( "Failed to call eglInitialize." ) );
		CheckEGLErrorAndLog();
		Destroy();
		EGLint displayAttributes2[] = { EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE, EGL_NONE };
		mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, mNativeDisplay, displayAttributes );
		if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
			Destroy();
			return false;
		}
	}
	const EGLint configAttributes[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_RED_SIZE,       EGL_DONT_CARE,
		EGL_GREEN_SIZE,     EGL_DONT_CARE,
		EGL_BLUE_SIZE,      EGL_DONT_CARE,
		EGL_ALPHA_SIZE,     EGL_DONT_CARE,
		EGL_NONE
	};
	EGLint configCount;
	EGLBoolean result = eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount );
	if( result == EGL_FALSE || ( configCount == 0 ) ) {
		TVPAddLog( TJS_W( "Failed to call eglChooseConfig." ) );
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}
	EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes );
	if( !CheckEGLErrorAndLog() || mContext == EGL_NO_CONTEXT ) {
		TVPAddLog( TJS_W( "Failed to call eglCreateContext." ) );
		Destroy();
		return false;
	}
	TVPOpenGLESVersion = 200;
	GetConfigAttribute( mConfig );
	return true;
}

static bool eglInited = false;

bool tTVPEGLContext::Initialize() 
{
	if( !mNativeDisplay ) {
		return false;
	}

	// ANGLE_ENABLE_D3D11 を有効に
	// ANGLE_ENABLE_D3D9 を無効に
	// OpenGL ES 3.0 を使用する
	EGLint displayAttributes[] = {
		EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
		EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE,
		EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE,
		EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
		EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
		EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
		EGL_NONE
	};

	mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void*>(mNativeDisplay), displayAttributes);
	if( !CheckEGLErrorAndLog() ) {
		TVPAddLog( TJS_W( "Failed to call eglGetPlatformDisplayEXT." ) );
		Destroy();
		return false;
	}
	EGLint esClientVersion = 3;
	EGLint majorVersion, minorVersion;
	if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
		TVPAddLog( TJS_W( "Failed to call eglInitialize." ) );
		CheckEGLErrorAndLog();
		// 要求を下げて再試行
		Destroy();
		EGLint displayAttributes2[] = {
			EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
			EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE,
			EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE,
			EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
			EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
			EGL_NONE
		};

		mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, mNativeDisplay, displayAttributes2 );
		if( !CheckEGLErrorAndLog() ) {
			TVPAddLog( TJS_W( "Failed to call eglGetPlatformDisplayEXT." ) );
			if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
				Destroy();
				return false;
			}
		}
		if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
			TVPAddLog( TJS_W( "Failed to call eglInitialize." ) );
			CheckEGLErrorAndLog();
			// 要求を下げて再試行
			Destroy();
			EGLint displayAttributes3[] = {
				EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE,
				EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE,
				EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE,
				EGL_NONE
			};

			mDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, mNativeDisplay, displayAttributes3 );
			if( !CheckEGLErrorAndLog() ) {
				TVPAddLog( TJS_W( "Failed to call eglGetPlatformDisplayEXT." ) );
				if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
					Destroy();
					return false;
				}
			}
			if( eglInitialize( mDisplay, &majorVersion, &minorVersion ) == EGL_FALSE ) {
				TVPAddLog( TJS_W( "Failed to call eglInitialize." ) );
				CheckEGLErrorAndLog();
				Destroy();
				return false;
			}
		}
		esClientVersion = 2;	// DirectX 9 の時は、ES 2.0 でないと動かない
		TVPOpenGLESVersion = 200;
	}
	TVPAddLog( ttstr( TJS_W( "(info) Support EGL") ) + to_tjs_string( majorVersion ) + ttstr( TJS_W( "." ) ) + to_tjs_string( minorVersion ) );

	if (!eglInited) {
		int egl_version = gladLoadEGL(mDisplay, gladload);
		if (!egl_version) {
			TVPAddLog( TJS_W("Unable to reload glad EGL") );
			Destroy();
			return false;
		}
	    //fprintf(stderr, "Loaded EGL %d.%d after reload.\n",
    	//       GLAD_VERSION_MAJOR(egl_version), GLAD_VERSION_MINOR(egl_version));
		eglInited = true;
	}

	eglBindAPI( EGL_OPENGL_ES_API );

#if 1
	// D3D11 でも、ES3 が動かせない環境があるようなので、EGL_RENDERABLE_TYPE は指定せずに列挙する
	const EGLint configAttributesAny[] = { EGL_RED_SIZE, EGL_DONT_CARE, EGL_GREEN_SIZE, EGL_DONT_CARE, EGL_BLUE_SIZE, EGL_DONT_CARE, EGL_ALPHA_SIZE, EGL_DONT_CARE, EGL_NONE };
	EGLint configSize;
	mConfig = nullptr;
	if( eglChooseConfig( mDisplay, configAttributesAny, NULL, 0, &configSize ) != EGL_FALSE && configSize > 0 ) {
		std::vector<EGLConfig> configs( configSize );
		EGLint configCount;
		EGLBoolean ret = eglChooseConfig( mDisplay, configAttributesAny, &( configs[0] ), configSize, &configCount );
		if( ret != EGL_FALSE ) {
			if( configCount == 1 ) {
				// 1個しか見付からないのなら、それでもう確定
				mConfig = configs[0];
			}
			EGLint r, g, b, a;
			if( mConfig == nullptr ) {
				// 最初にRGBA全て8のものを探す
				for( EGLint i = 0; i < configCount; i++ ) {
					EGLConfig config = configs[i];
					eglGetConfigAttrib( mDisplay, config, EGL_RED_SIZE, &r );
					eglGetConfigAttrib( mDisplay, config, EGL_GREEN_SIZE, &g );
					eglGetConfigAttrib( mDisplay, config, EGL_BLUE_SIZE, &b );
					eglGetConfigAttrib( mDisplay, config, EGL_ALPHA_SIZE, &a );
					if( r == 8 && g == 8 && b == 8 && a == 8 ) {
						mConfig = config;
						break;
					}
				}
			}
			if( mConfig == nullptr ) {
				// 見付からなかったようなので、Alphaは気にしないことにして探す
				for( EGLint i = 0; i < configCount; i++ ) {
					EGLConfig config = configs[i];
					eglGetConfigAttrib( mDisplay, config, EGL_RED_SIZE, &r );
					eglGetConfigAttrib( mDisplay, config, EGL_GREEN_SIZE, &g );
					eglGetConfigAttrib( mDisplay, config, EGL_BLUE_SIZE, &b );
					eglGetConfigAttrib( mDisplay, config, EGL_ALPHA_SIZE, &a );
					if( r == 8 && g == 8 && b == 8 ) {
						mConfig = config;
						break;
					}
				}
			}
			if( mConfig == nullptr ) {
				// RGB888 がないのなら、もう気にせず最初のやつに決定してしまう
				mConfig = configs[0];
			}
			// D3D11 でも ES3.0 が使用出来ない環境があるので、バージョンを取得してから試す
			EGLint eglver = 0;
			eglGetConfigAttrib( mDisplay, mConfig, EGL_RENDERABLE_TYPE, &eglver );
			if( esClientVersion > 2 ){
				if( eglver & EGL_OPENGL_ES3_BIT ) {
					esClientVersion = 3;
					TVPOpenGLESVersion = 300;
				} else {
					esClientVersion = 2;
					TVPOpenGLESVersion = 200;
				}
			}
			bool successCreateContext = true;
			EGLint contextAttributes[] = {
				EGL_CONTEXT_CLIENT_VERSION, esClientVersion,
				EGL_CONTEXT_MINOR_VERSION, 0,
				EGL_NONE };
			mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes );
			if( mContext == EGL_NO_CONTEXT ) { TVPAddLog( TJS_W( "eglCreateContext returned EGL_NO_CONTEXT." ) ); }
			if( !CheckEGLErrorAndLog() ) {
				// 選択候補で失敗。全てのパターンで初期化を試す
				TVPAddLog( TJS_W( "Failed to call eglCreateContext. Try all patterns." ) );
				successCreateContext = false;
				if( mContext != EGL_NO_CONTEXT ) {
					eglDestroyContext( mDisplay, mContext );
					mContext = EGL_NO_CONTEXT;
				}
				for( EGLint i = 0; i < configCount; i++ ) {
					mConfig = configs[i];
					eglGetConfigAttrib( mDisplay, mConfig, EGL_RENDERABLE_TYPE, &eglver );
					if( esClientVersion > 2 ) {
						if( eglver & EGL_OPENGL_ES3_BIT ) {
							esClientVersion = 3;
							TVPOpenGLESVersion = 300;
						} else {
							esClientVersion = 2;
							TVPOpenGLESVersion = 200;
						}
					}
					EGLint contextAttributes2[] = { EGL_CONTEXT_CLIENT_VERSION, esClientVersion, EGL_NONE };
					mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes2 );
					if( mContext == EGL_NO_CONTEXT ) { TVPAddLog( TJS_W( "eglCreateContext returned EGL_NO_CONTEXT." ) ); }
					if( !CheckEGLErrorAndLog() ) {
						if( mContext != EGL_NO_CONTEXT ) {
							eglDestroyContext( mDisplay, mContext );
							mContext = EGL_NO_CONTEXT;
						}
					} else {
						successCreateContext = true;
						break;
					}
				}
			}
			if( successCreateContext ) {
				GetConfigAttribute( mConfig );
			} else {
				// 全パターン失敗。
				TVPAddLog( TJS_W( "Failed to call eglCreateContext." ) );
				Destroy();
				esClientVersion = 2;
				// 最小構成を試してみる
				if( !TryMinimumLevelInitialize() ) {
					TVPAddLog( TJS_W( "Failed to initialize ANGLE. Cannot run on this device." ) );
					Destroy();
					return false;
				}
			}
		} else {
			// 失敗。起動できない
			TVPAddLog( TJS_W( "Failed to call eglChooseConfig." ) );
			CheckEGLErrorAndLog();
			Destroy();
			return false;
		}
	} else {
		// 1個も見付からない時は、失敗。起動できない
		TVPAddLog( TJS_W( "Failed to call eglChooseConfig." ) );
		CheckEGLErrorAndLog();
		Destroy();
		return false;
	}
#else
	const EGLint configAttributes[] = {
		EGL_RED_SIZE,       8,
		EGL_GREEN_SIZE,     8,
		EGL_BLUE_SIZE,      8,
		EGL_ALPHA_SIZE,     8,
		EGL_DEPTH_SIZE,     EGL_DONT_CARE,
		EGL_STENCIL_SIZE,   EGL_DONT_CARE,
		EGL_SAMPLE_BUFFERS, mMultisample ? 1 : 0,
		EGL_NONE
	};
	EGLint configCount;
	EGLBoolean result = eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount );
	if( result == EGL_FALSE || (configCount == 0) ) {
		TVPAddLog( TJS_W( "Failed to call eglChooseConfig. try EGL_DONT_CARE." ) );
		const EGLint configAttributes2[] = {
			EGL_RED_SIZE,       EGL_DONT_CARE,
			EGL_GREEN_SIZE,     EGL_DONT_CARE,
			EGL_BLUE_SIZE,      EGL_DONT_CARE,
			EGL_ALPHA_SIZE,     EGL_DONT_CARE,
			EGL_DEPTH_SIZE,     EGL_DONT_CARE,
			EGL_STENCIL_SIZE,   EGL_DONT_CARE,
			EGL_NONE
		};
		EGLBoolean result = eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount );
		if( result == EGL_FALSE || ( configCount == 0 ) ) {
			TVPAddLog( TJS_W( "Failed to call eglChooseConfig." ) );
			CheckEGLErrorAndLog();
			Destroy();
			return false;
		}
	}
	GetConfigAttribute( mConfig );
	EGLint eglver = 0;
	eglGetConfigAttrib( mDisplay, mConfig, EGL_RENDERABLE_TYPE, &eglver );

	if( esClientVersion > 2 ) {
		if( eglver & EGL_OPENGL_ES3_BIT ) {
			esClientVersion = 3;
			TVPOpenGLESVersion = 300;
		} else {
			esClientVersion = 2;
			TVPOpenGLESVersion = 200;
		}
	}
	EGLint contextAttributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, esClientVersion,
		EGL_CONTEXT_MINOR_VERSION, 0,
		EGL_NONE };
	mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes );
	if( mContext == EGL_NO_CONTEXT ) { TVPAddLog( TJS_W( "eglCreateContext returned EGL_NO_CONTEXT." ) ); }
	if( !CheckEGLErrorAndLog() ) {
		TVPAddLog( TJS_W( "Failed to call eglCreateContext. Try eglChooseConfig again." ) );
		if( mContext != EGL_NO_CONTEXT ) {
			eglDestroyContext( mDisplay, mContext );
			mContext = EGL_NO_CONTEXT;
		}

		const EGLint configAttributes2[] = {
			EGL_RED_SIZE,       EGL_DONT_CARE,
			EGL_GREEN_SIZE,     EGL_DONT_CARE,
			EGL_BLUE_SIZE,      EGL_DONT_CARE,
			EGL_ALPHA_SIZE,     EGL_DONT_CARE,
			EGL_DEPTH_SIZE,     EGL_DONT_CARE,
			EGL_STENCIL_SIZE,   EGL_DONT_CARE,
			EGL_NONE
		};
		EGLBoolean result = eglChooseConfig( mDisplay, configAttributes, &mConfig, 1, &configCount );
		if( result == EGL_FALSE || ( configCount == 0 ) ) {
			TVPAddLog( TJS_W( "Failed to call eglChooseConfig." ) );
			CheckEGLErrorAndLog();
			Destroy();
			return false;
		}
		mContext = eglCreateContext( mDisplay, mConfig, nullptr, contextAttributes );
		if( mContext == EGL_NO_CONTEXT ) { TVPAddLog( TJS_W( "eglCreateContext returned EGL_NO_CONTEXT." ) ); }
		if( !CheckEGLErrorAndLog() ) {
			TVPAddLog( TJS_W( "Failed to call eglCreateContext." ) );
			Destroy();
			esClientVersion = 2;
			if( !TryMinimumLevelInitialize() ) {
				TVPAddLog( TJS_W( "Failed to initialize ANGLE. Cannot run on this device." ) );
				Destroy();
				return false;
			}
		}
		GetConfigAttribute( mConfig );
	}
#endif
	TVPAddLog( ttstr( TJS_W( "(info) Run on OpenGL ES" ) ) + to_tjs_string( esClientVersion ) + ttstr( TJS_W( ".0 (ANGLE)" ) ) );

	EGLint surfaceAttributes[] = { EGL_NONE };

	mSurface = eglCreateWindowSurface( mDisplay, mConfig, mNativeWindow, surfaceAttributes );
	if( mSurface == EGL_NO_SURFACE ) { TVPAddLog( TJS_W( "eglCreateWindowSurface returned EGL_NO_SURFACE." ) ); }
	if( !CheckEGLErrorAndLog() ) {
		TVPAddLog( TJS_W( "Failed to call eglCreateWindowSurface." ) );
		Destroy();
		return false;
	}
	assert( mSurface != EGL_NO_SURFACE );

	eglMakeCurrent( mDisplay, mSurface, mSurface, mContext );
	if( !CheckEGLErrorAndLog() ) {
		TVPAddLog( TJS_W( "Failed to call eglMakeCurrent." ) );
		Destroy();
		return false;
	}
	mSwapInterval = mMinSwapInterval;
	eglSwapInterval( mDisplay, mSwapInterval );	// V-sync wait?

	return true;
}

void tTVPEGLContext::GetConfigAttribute( EGLConfig config ) 
{
	eglGetConfigAttrib( mDisplay, config, EGL_RED_SIZE, &mRedBits );
	eglGetConfigAttrib( mDisplay, config, EGL_GREEN_SIZE, &mGreenBits );
	eglGetConfigAttrib( mDisplay, config, EGL_BLUE_SIZE, &mBlueBits );
	eglGetConfigAttrib( mDisplay, config, EGL_ALPHA_SIZE, &mAlphaBits );
	eglGetConfigAttrib( mDisplay, config, EGL_DEPTH_SIZE, &mDepthBits );
	eglGetConfigAttrib( mDisplay, config, EGL_STENCIL_SIZE, &mStencilBits );
	eglGetConfigAttrib( mDisplay, config, EGL_MIN_SWAP_INTERVAL, &mMinSwapInterval );
	eglGetConfigAttrib( mDisplay, config, EGL_MAX_SWAP_INTERVAL, &mMaxSwapInterval );
	ttstr displog = ttstr( TJS_W( "(info) EGL config :" ) );
	displog += ttstr( TJS_W( " R:" ) ) + to_tjs_string( mRedBits );
	displog += ttstr( TJS_W( " G:" ) ) + to_tjs_string( mGreenBits );
	displog += ttstr( TJS_W( " B:" ) ) + to_tjs_string( mBlueBits );
	displog += ttstr( TJS_W( " A:" ) ) + to_tjs_string( mAlphaBits );
	displog += ttstr( TJS_W( " Depth:" ) ) + to_tjs_string( mDepthBits );
	displog += ttstr( TJS_W( " Stencil:" ) ) + to_tjs_string( mStencilBits );
	TVPAddLog( displog );
}

void tTVPEGLContext::Destroy() 
{
	if( mSurface != EGL_NO_SURFACE ) {
		assert( mDisplay != EGL_NO_DISPLAY );
		eglDestroySurface( mDisplay, mSurface );
		mSurface = EGL_NO_SURFACE;
	}
	if( mContext != EGL_NO_CONTEXT ) {
		assert( mDisplay != EGL_NO_DISPLAY );
		eglDestroyContext( mDisplay, mContext );
		mContext = EGL_NO_CONTEXT;
	}
	if( mDisplay != EGL_NO_DISPLAY ) {
		eglMakeCurrent( mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
		eglTerminate( mDisplay );
		mDisplay = EGL_NO_DISPLAY;
	}
}

bool tTVPEGLContext::IsInitialized() const {
	return mSurface != EGL_NO_SURFACE && mContext != EGL_NO_CONTEXT && mDisplay != EGL_NO_DISPLAY;
}

void tTVPEGLContext::GetSurfaceSize(int *width, int *height)
{
	if (width) *width = SurfaceWidth();
	if (height) *height = SurfaceHeight();
}

EGLint tTVPEGLContext::SurfaceWidth() const
{
	EGLint result = 0;
	EGLBoolean ret = eglQuerySurface( mDisplay, mSurface, EGL_WIDTH, &result );
	if( ret == EGL_FALSE ) {
		CheckEGLErrorAndLog();
	}
	return result;
}

EGLint tTVPEGLContext::SurfaceHeight() const
{
	EGLint result = 0;
	EGLBoolean ret = eglQuerySurface( mDisplay, mSurface, EGL_HEIGHT, &result );
	if( ret == EGL_FALSE ) {
		CheckEGLErrorAndLog();
	}
	return result;
}

void tTVPEGLContext::MakeCurrent()
{
	if( mSurface != EGL_NO_SURFACE && mContext != EGL_NO_CONTEXT ) {
		eglMakeCurrent( mDisplay, mSurface, mSurface, mContext );
	}
	}

void tTVPEGLContext::Swap() {
	if( mSurface != EGL_NO_SURFACE ) {
		eglSwapBuffers( mDisplay, mSurface );
	}
}

void tTVPEGLContext::SetWaitVSync( bool b ) {
	if( b ) {
		mSwapInterval = 1;
		if (mDisplay != EGL_NO_DISPLAY) {
			eglSwapInterval( mDisplay, mSwapInterval );
		}
	} else {
		mSwapInterval = mMinSwapInterval;
		if (mDisplay != EGL_NO_DISPLAY) {
			eglSwapInterval( mDisplay, mSwapInterval );
		}
	}
}

void tTVPEGLContext::InitEGL()
{
	static int egl_version = 0;
	if (!egl_version) {
		egl_version = gladLoadEGL(nullptr, gladload);
		if (!egl_version) {
			TVPThrowExceptionMessage( TJS_W("Unable to initialize glad EGL.") );
		}
		//fprintf(stderr, "Loaded EGL %d.%d on first load.\n",
		//       GLAD_VERSION_MAJOR(egl_version), GLAD_VERSION_MINOR(egl_version));
	}
}

