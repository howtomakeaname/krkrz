
# 共通の include ディレクトリ
set( KRKRZ_INC
common/environ
common/tjs2
common/base
common/extension
common/sound
common/msg
common/utils
common/visual
common/visual/gl
common/visual/opengl
common/glad/include 
external
)

set( KRKRZ_INC_WIN32_COMMON
win32/vcproj
win32/environ
common/visual/IA32
)

set( KRKRZ_INC_WIN32
win32/vcproj
win32/environ
win32/base
win32/sound
win32/msg
win32/utils
win32/visual
win32/movie
common/visual/IA32
)

set( KRKRZ_SRC 
common/tjs2/tjs.cpp
common/tjs2/tjs.tab.cpp
common/tjs2/tjsArray.cpp
common/tjs2/tjsBinarySerializer.cpp
common/tjs2/tjsByteCodeLoader.cpp
common/tjs2/tjsCompileControl.cpp
common/tjs2/tjsConfig.cpp
common/tjs2/tjsConstArrayData.cpp
common/tjs2/tjsDate.cpp
common/tjs2/tjsdate.tab.cpp
common/tjs2/tjsDateParser.cpp
common/tjs2/tjsDebug.cpp
common/tjs2/tjsDictionary.cpp
common/tjs2/tjsDisassemble.cpp
common/tjs2/tjsError.cpp
common/tjs2/tjsException.cpp
common/tjs2/tjsGlobalStringMap.cpp
common/tjs2/tjsInterCodeExec.cpp
common/tjs2/tjsInterCodeGen.cpp
common/tjs2/tjsInterface.cpp
common/tjs2/tjsLex.cpp
common/tjs2/tjsMath.cpp
common/tjs2/tjsMessage.cpp
common/tjs2/tjsMT19937ar-cok.cpp
common/tjs2/tjsNamespace.cpp
common/tjs2/tjsNative.cpp
common/tjs2/tjsObject.cpp
common/tjs2/tjsObjectExtendable.cpp
common/tjs2/tjsOctPack.cpp
common/tjs2/tjspp.tab.cpp
common/tjs2/tjsRandomGenerator.cpp
common/tjs2/tjsRegExp.cpp
common/tjs2/tjsScriptBlock.cpp
common/tjs2/tjsScriptCache.cpp
common/tjs2/tjsSnprintf.cpp
common/tjs2/tjsString.cpp
common/tjs2/tjsUtils.cpp
common/tjs2/tjsVariant.cpp
common/tjs2/tjsVariantString.cpp
common/base/BinaryStream.cpp
common/base/CharacterSet.cpp
common/base/EventIntf.cpp
common/base/PluginIntf.cpp
common/base/ScriptMgnIntf.cpp
common/base/StorageIntf.cpp
common/base/SysInitIntf.cpp
common/base/SystemIntf.cpp
common/base/TextStream.cpp
common/base/UtilStreams.cpp
common/base/XP3Archive.cpp
common/base/StorageCache.cpp
common/environ/TouchPoint.cpp
common/extension/Extension.cpp
common/msg/MsgIntf.cpp
common/sound/MathAlgorithms.cpp
common/sound/PhaseVocoderDSP.cpp
common/sound/PhaseVocoderFilter.cpp
common/sound/QueueSoundBufferImpl.cpp
common/sound/RealFFT.cpp
common/sound/SoundBufferBaseIntf.cpp
common/sound/SoundBufferBaseImpl.cpp
common/sound/SoundDecodeThread.cpp
common/sound/SoundEventThread.cpp
common/sound/SoundPlayer.cpp
common/sound/WaveFormatConverter.cpp
common/sound/WaveIntf.cpp
common/sound/WaveLoopManager.cpp
common/sound/WaveSegmentQueue.cpp
common/sound/OpusCodecDecoder.cpp
common/sound/VorbisCodecDecoder.cpp
common/utils/ClipboardIntf.cpp
common/utils/cp932_uni.cpp
common/utils/DebugIntf.cpp
common/utils/md5.c
common/utils/MiscUtility.cpp
common/utils/Random.cpp
common/utils/ThreadIntf.cpp
common/utils/TickCount.cpp
common/utils/TimerThread.cpp
common/utils/TimerIntf.cpp
common/utils/TVPTimer.cpp
common/utils/uni_cp932.cpp
common/utils/VelocityTracker.cpp
common/visual/BitmapIntf.cpp
common/visual/BitmapLayerTreeOwner.cpp
common/visual/CharacterData.cpp
common/visual/ComplexRect.cpp
common/visual/DrawDevice.cpp
common/visual/FontSystem.cpp
common/visual/FreeType.cpp
common/visual/FreeTypeFontRasterizer.cpp
common/visual/GraphicsLoaderIntf.cpp
common/visual/GraphicsLoadThread.cpp
common/visual/ImageFunction.cpp
common/visual/LayerBitmapImpl.cpp
common/visual/LayerBitmapIntf.cpp
common/visual/LayerIntf.cpp
common/visual/LayerManager.cpp
common/visual/LayerTreeOwnerImpl.cpp
common/visual/LoadJPEG.cpp
common/visual/LoadPNG.cpp
common/visual/LoadTLG.cpp
common/visual/NullDrawDevice.cpp
common/visual/BitmapDrawDevice.cpp
common/visual/PrerenderedFont.cpp
common/visual/RectItf.cpp
common/visual/SaveTLG5.cpp
common/visual/SaveTLG6.cpp
common/visual/TransIntf.cpp
common/visual/tvpgl.c
common/visual/VideoOvlIntf.cpp
common/visual/WindowIntf.cpp
common/visual/KeyRepeat.cpp
common/visual/gl/blend_function.cpp
common/visual/gl/ResampleImage.cpp
common/visual/gl/WeightFunctor.cpp
)

if (USEOPENGL)

set( KRKRZ_SRC_OPENGL
common/visual/opengl/OGLDrawDevice.cpp
common/visual/opengl/CanvasIntf.cpp
common/visual/opengl/GLTexture.cpp
common/visual/opengl/GLFrameBufferObject.cpp
common/visual/opengl/GLShaderUtil.cpp
common/visual/opengl/Matrix32Intf.cpp
common/visual/opengl/Matrix44Intf.cpp
common/visual/opengl/OffscreenIntf.cpp
common/visual/opengl/OpenGLError.cpp
common/visual/opengl/ShaderProgramIntf.cpp
common/visual/opengl/TextureIntf.cpp
common/visual/opengl/TextureLayerTreeOwner.cpp
common/visual/opengl/VertexBinderIntf.cpp
common/visual/opengl/VertexBufferIntf.cpp 
common/glad/src/gles2.c
)

list(APPEND KRKRZ_DEFINES
TVP_USE_OPENGL
)

set( KRKRZ_SRC_WIN32_OPENGL
common/glad/src/egl.c
common/visual/opengl/EGLContext.cpp
common/visual/win32/OpenGLPlatform.cpp
)

endif()

set( KRKRZ_SRC_WIN32 
win32/environ/CompatibleNativeFuncs.cpp
win32/environ/ConfigFormUnit.cpp
win32/environ/EmergencyExit.cpp
win32/environ/MouseCursor.cpp
win32/environ/SystemControl.cpp
win32/environ/TVPWindow.cpp
win32/environ/VersionFormUnit.cpp
win32/environ/WindowFormUnit.cpp
win32/environ/WindowsUtil.cpp
win32/base/EventImpl.cpp
win32/base/FileSelector.cpp
win32/base/FuncStubs.cpp
win32/base/NativeEventQueue.cpp
win32/base/PluginImpl.cpp
win32/base/SusieArchive.cpp
win32/base/ScriptMgnImpl.cpp
win32/base/StorageImpl.cpp
win32/base/SysInitImpl.cpp
win32/base/SystemImpl.cpp
win32/msg/MsgImpl.cpp
win32/msg/MsgLoad.cpp
win32/msg/ReadOptionDesc.cpp
win32/sound/tvpsnd.c
win32/sound/WaveImpl.cpp
win32/sound/XAudio2Device.cpp
win32/utils/ClipboardImpl.cpp
win32/utils/ThreadImpl.cpp
win32/utils/TickCountImpl.cpp
win32/visual/BasicDrawDevice.cpp
win32/visual/BitmapBitsAlloc.cpp
win32/visual/BitmapInfomationImpl.cpp
win32/visual/DInputMgn.cpp
win32/visual/DrawDeviceImpl.cpp
win32/visual/GDIFontRasterizer.cpp
win32/visual/GraphicsLoaderImpl.cpp
win32/visual/LayerImpl.cpp
win32/visual/NativeFreeTypeFace.cpp
win32/visual/TVPScreen.cpp
win32/visual/TVPSysFont.cpp
win32/visual/VideoOvlImpl.cpp
win32/visual/VSyncTimingThread.cpp
win32/visual/WindowImpl.cpp
win32/environ/Application.cpp
win32/movie/TVPVideoOverlay.cpp

win32/vcproj/tvpwin32.rc
win32/vcproj/dpi.manifest

common/visual/LoadJXR.cpp
common/sound/xmmlib.cpp 

)

set( KRKRZ_SRC_WIN32_SSE
win32/environ/DetectCPU.cpp
common/visual/gl/ResampleImageAVX2.cpp
common/visual/gl/ResampleImageSSE2.cpp
common/visual/gl/x86simdutil.cpp
common/visual/gl/x86simdutilAVX2.cpp
common/visual/gl/blend_function_sse2.cpp	
common/visual/gl/blend_function_avx2.cpp
common/visual/gl/adjust_color_sse2.cpp
common/visual/gl/blend_function_sse2.cpp
common/visual/gl/boxblur_sse2.cpp
common/visual/gl/colorfill_sse2.cpp
common/visual/gl/colormap_sse2.cpp
common/visual/gl/pixelformat_sse2.cpp
common/visual/gl/tlg_sse2.cpp
common/visual/gl/univtrans_sse2.cpp
common/visual/IA32/detect_cpu.cpp
common/visual/IA32/tvpgl_ia32_intf.c
common/sound/MathAlgorithms_SSE.cpp
common/sound/RealFFT_SSE.cpp
common/sound/WaveFormatConverter_SSE.cpp
)

set( KRKRZ_SRC_WIN32_NAS
common/visual/IA32/tlg6_chroma.nas
common/visual/IA32/tlg6_golomb.nas
)

if (USENEON)

    list(APPEND KRKRZ_SRC
    common/visual/gl/blend_function_neon.cpp
    )

endif()

# エラーになったので一時除外
if ( FALSE AND (ANDROID_ABI STREQUAL x86 OR ANDROID_ABI STREQUAL x86_64) )
	list(APPEND KRKRZ_SRC
	common/sound/MathAlgorithms_SSE.cpp
	common/sound/RealFFT_SSE.cpp
	common/sound/WaveFormatConverter_SSE.cpp
	common/sound/xmmlib.cpp 
	)
endif()

set( KRKRZ_INC_GENERIC
generic/base
generic/environ
generic/msg
generic/utils
generic/visual
)

set( KRKRZ_SRC_GENERIC
generic/base/DrawDeviceImpl.cpp
generic/base/EventImpl.cpp
generic/base/FuncStubs.cpp
generic/base/NativeEventQueue.cpp
generic/base/PluginImpl.cpp
generic/base/ScriptMgnImpl.cpp
generic/base/StorageImpl.cpp
generic/base/SysInitImpl.cpp
generic/base/SystemImpl.cpp
generic/environ/Application.cpp
generic/environ/FontSystemBase.cpp
generic/environ/WindowForm.cpp
generic/environ/JoyPad.cpp
generic/msg/MsgImpl.cpp
generic/msg/MsgLoad.cpp
generic/utils/ClipboardImpl.cpp
generic/utils/TickCountImpl.cpp
generic/visual/BitmapBitsAlloc.cpp
generic/visual/LayerImpl.cpp
generic/visual/VideoOvlImpl.cpp
generic/visual/WindowImpl.cpp
generic/visual/BitmapInfomationImpl.cpp
)

set(KRKRZ_PUBLIC_HEADER 
common/visual/tvpinputdefs.h
common/visual/MoviePlayer.h
common/sound/AudioDevice.h
generic/krkrz.h
generic/base/LocalFileSystem.h
generic/environ/WindowFormEvent.h
generic/environ/VirtualKey.h
)
list(TRANSFORM KRKRZ_PUBLIC_HEADER PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

if (WIN32)
	list(APPEND KRKRZ_INC_GENERIC ${KRKRZ_INC_WIN32_COMMON})
	list(APPEND KRKRZ_SRC_GENERIC ${KRKRZ_SRC_WIN32_SSE})
	if (NOT WIN64)
		list(APPEND KRKRZ_SRC_GENERIC ${KRKRZ_SRC_WIN32_NAS})
		list(APPEND KRKRZ_SRC_WIN32 ${KRKRZ_SRC_WIN32_NAS})
	endif()
endif()

