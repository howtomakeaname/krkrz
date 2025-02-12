//---------------------------------------------------------------------------
/*
	Kirikiri Z
	See details of license at "LICENSE"
*/
//---------------------------------------------------------------------------
// Video Overlay support implementation
//---------------------------------------------------------------------------


#include "tjsCommHead.h"
#include "CharacterSet.h"

#include <algorithm>
#include "VideoOvlImpl.h"
#include "WindowForm.h"
#include "Application.h"
#include "StorageIntf.h"
#include "LayerIntf.h"
#include "LayerBitmapIntf.h"
#include "MsgImpl.h"

//---------------------------------------------------------------------------
// tTJSNI_VideoOverlay
//---------------------------------------------------------------------------
tTJSNI_VideoOverlay::tTJSNI_VideoOverlay() 
: mPlayer(nullptr) 
, mAudioStream(nullptr)
, Layer1(nullptr)
, Layer2(nullptr)
, Bitmap(nullptr)
{
	Mode = vomOverlay;
	Visible = false;

}

tTJSNI_VideoOverlay::~tTJSNI_VideoOverlay()
{
	Close();
}

//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTJSNI_VideoOverlay::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	tjs_error hr = inherited::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;
	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_VideoOverlay::Invalidate()
{
	Close();
	inherited::Invalidate();
}

bool tTJSNI_VideoOverlay::IsMixerPlaying() const 
{ 
	return Mode == vomMixer && mPlayer && mPlayer->IsPlaying(); 
}

bool
tTJSNI_VideoOverlay::Update()
{
	if (mPlayer && mPlayer->IsPlaying()) {

		SetStatusAsync( tTVPVideoOverlayStatus::Play );

		if (Mode == vomMixer) {
			long width = mPlayer->Width();
			long height = mPlayer->Height();

			Window->GetForm()->UpdateVideo(width,height, [this,width,height](char *destp, int dest_pitch) {
				this->mPlayer->GetVideoFrame((uint8_t*)destp, width, height, dest_pitch);
			});
		}

		if (Mode == vomLayer && Bitmap) {

			// get video image size
			long width = mPlayer->Width();
			long height = mPlayer->Height();

			tTJSNI_BaseLayer	*l1 = Layer1;
			tTJSNI_BaseLayer	*l2 = Layer2;

			// Check layer image size
			if( l1 != NULL )
			{
				if( (long)l1->GetImageWidth() != width || (long)l1->GetImageHeight() != height )
					l1->SetImageSize( width, height );
				if( (long)l1->GetWidth() != width || (long)l1->GetHeight() != height )
					l1->SetSize( width, height );
			}
			if( l2 != NULL )
			{
				if( (long)l2->GetImageWidth() != width || (long)l2->GetImageHeight() != height )
					l2->SetImageSize( width, height );
				if( (long)l2->GetWidth() != width || (long)l2->GetHeight() != height )
					l2->SetSize( width, height );
			}

			// bitmap に描画
			{
				tTVPBitmap *bitmap = Bitmap->GetBitmap();
				tjs_uint width  = bitmap->GetWidth();
				tjs_uint height = bitmap->GetHeight();

				// XXX 上下反転してるので注意
				tjs_int pitch = bitmap->GetPitch(); 
				uint8_t *dest = static_cast<uint8_t*>(bitmap->GetScanLine(0));

				mPlayer->GetVideoFrame(dest, width, height, pitch);
			}
			// レイヤに割当
			if( l1 ) l1->AssignMainImage( Bitmap );
			if( l2 ) l2->AssignMainImage( Bitmap );
			if( l1 ) l1->Update();
			if( l2 ) l2->Update();
			FireFrameUpdateEvent(0);
		}
		return true;
	} else {
		SetStatusAsync( tTVPVideoOverlayStatus::Stop );
    }
	return false;
}


//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Open(const ttstr &name) 
{
	Close();

	ttstr path;
	ttstr newpath = TVPGetPlacedPath(name);
	if( newpath.IsEmpty() ) {
		path = TVPNormalizeStorageName(name);
	} else {
		path = newpath;
	}

    TVPGetLocalName(path);

	mPlayer = TVPCreateMoviePlayer(path.c_str());
	if (mPlayer) {
		if (Mode == vomLayer) {
			long width = mPlayer->Width();
			long height = mPlayer->Height();
			if( Bitmap != NULL ) {
				delete Bitmap;
			}
			// XXほんとはプレイヤー側にダブルバッファ登録するのが妥当かもしれず
			Bitmap = new tTVPBaseBitmap( width, height, 32 );
		}

		// オーディオ再生を吉里吉里側で行う
		if (false && Application->MovieAudioEnable() && mPlayer->IsAudioAvailable()) {

			iTVPMoviePlayer::AudioFormat format;
			mPlayer->GetAudioFormat(&format);

			tTVPAudioStreamParam param;
			param.Channels      = format.Channels;		// チャンネル数
			param.SampleRate    = format.SampleRate;	// サンプリングレート
			param.BitsPerSample = format.BitsPerSample;	// サンプル当たりのビット数
			param.SampleType    = format.SampleType;	// サンプルの形式
			param.FramesPerBuffer = format.SampleRate / 2; // バッファサイズ

			mAudioStream = TVPCreateAudioStream(param);

			if (mAudioStream)  {
				mPlayer->SetOnAudioDecoded([](void *userPtr, const uint8_t *data, size_t sizeBytes) {
					if (userPtr) {
						((iTVPAudioStream*)userPtr)->Enqueue((void*)data, sizeBytes, false);
					}
				}, this);	
			}
		}

	} else {
		SetStatus( tTVPVideoOverlayStatus::LoadError );
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Close() 
{
	if (mPlayer) {
		Window->GetForm()->DelVideoOverlay(this);
		delete mPlayer;
		mPlayer = nullptr;
	}

	if (mAudioStream) {
		delete mAudioStream;
		mAudioStream = nullptr;
	}

	if( Bitmap ) {
		delete Bitmap;
		Bitmap = NULL;
	}

	SetStatus(tTVPVideoOverlayStatus::Unload);
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Shutdown() 
{
	Close();
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Disconnect() 
{
	Shutdown();
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Play() {
	if (mPlayer) {
		mPlayer->Play();
		Window->GetForm()->AddVideoOverlay(this);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Stop() {
	if (mPlayer) {
		mPlayer->Stop();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Pause() {
	if (mPlayer) {
		mPlayer->Pause();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Rewind() {
	if (mPlayer) {
		mPlayer->Seek( 0 );
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::Prepare() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetSegmentLoop( int comeFrame, int goFrame ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetPeriodEvent( int eventFrame ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetRectangleToVideoOverlay() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetPosition(tjs_int left, tjs_int top) {
	if( Mode == vomLayer )
	{
		if( Layer1 != NULL ) Layer1->SetPosition( left, top );
		if( Layer2 != NULL ) Layer2->SetPosition( left, top );
	}
	else
	{
		// XXX
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetSize(tjs_int width, tjs_int height) {
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetBounds(const tTVPRect & rect) {
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetLeft(tjs_int l) {
	if( Mode == vomLayer )
	{
		if( Layer1 != NULL ) Layer1->SetLeft( l );
		if( Layer2 != NULL ) Layer2->SetLeft( l );
	}
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetTop(tjs_int t) 
{
	if( Mode == vomLayer )
	{
		if( Layer1 != NULL ) Layer1->SetTop( t );
		if( Layer2 != NULL ) Layer2->SetTop( t );
	}

}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetWidth(tjs_int w) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetHeight(tjs_int h) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetVisible(bool b) {
	Visible = b;
	if( Mode == vomLayer )
	{
		if( Layer1 != NULL ) Layer1->SetVisible( Visible );
		if( Layer2 != NULL ) Layer2->SetVisible( Visible );
	} else {
		// XXX mPlayer の表示状態制御
	}
}
//---------------------------------------------------------------------------
bool tTJSNI_VideoOverlay::GetVisible() const {
	return Visible; 
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::ResetOverlayParams() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::DetachVideoOverlay() {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetRectOffset(tjs_int ofsx, tjs_int ofsy) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetTimePosition( tjs_uint64 p ) {
	if (mPlayer) {
		mPlayer->Seek( p * 1000 );
	}
}
//---------------------------------------------------------------------------
tjs_uint64 tTJSNI_VideoOverlay::GetTimePosition() {
	if (mPlayer) {
		return mPlayer->Position() / 1000;
	}
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetFrame( tjs_int f ) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetFrame() {
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetStopFrame( tjs_int f ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetDefaultStopFrame() {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetStopFrame() {
	return 0;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetFPS() {
	return 0.0;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetNumberOfFrame() {
	return 0;
}
//---------------------------------------------------------------------------
tjs_int64 tTJSNI_VideoOverlay::GetTotalTime() {
	if( mPlayer ) {
		return mPlayer->Duration() / 1000;
	}
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetLoop( bool b ) {}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetLayer1( tTJSNI_BaseLayer *l ) { Layer1 = l; }
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetLayer2( tTJSNI_BaseLayer *l ) { Layer2 = l; }
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetMode( tTVPVideoOverlayMode m ) {
	// ビデオオープン後のモード変更は禁止
	if( !mPlayer )
	{
		// 強制で vomMixer扱い
		if (m != vomLayer) m = vomMixer;
		Mode = m;
	}
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetPlayRate()
{
	return 0.0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetPlayRate(tjs_real r) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetAudioBalance()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetAudioBalance(tjs_int b) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetAudioVolume() {
	if (mPlayer) {
		float volume = 1.0; // mPlayer->GetMovieVolume();
		return (tjs_int)(volume * 100000);
	}
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetAudioVolume(tjs_int b) {
	if (mPlayer) {
		if( b < 0 ) b = 0;
		if( b > 100000 ) b = 100000;
		float volume = (float)b / 100000.0f;
		//mPlayer->SetMovieVolume ( volume );
	}
}
//---------------------------------------------------------------------------
tjs_uint tTJSNI_VideoOverlay::GetNumberOfAudioStream()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SelectAudioStream(tjs_uint n) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetEnabledAudioStream()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::DisableAudioStream() {}
//---------------------------------------------------------------------------
tjs_uint tTJSNI_VideoOverlay::GetNumberOfVideoStream()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SelectVideoStream(tjs_uint n) {}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetEnabledVideoStream()
{
	return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetMixingLayer( tTJSNI_BaseLayer *l )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::ResetMixingBitmap()
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetMixingMovieAlpha( tjs_real a )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetMixingMovieAlpha()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetMixingMovieBGColor( tjs_uint col )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_uint tTJSNI_VideoOverlay::GetMixingMovieBGColor()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrastRangeMin()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrastRangeMax()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrastDefaultValue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrastStepSize()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetContrast()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetContrast( tjs_real v )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightnessRangeMin()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightnessRangeMax()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightnessDefaultValue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightnessStepSize()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetBrightness()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetBrightness( tjs_real v )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHueRangeMin()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHueRangeMax()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHueDefaultValue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHueStepSize()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetHue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetHue( tjs_real v )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturationRangeMin()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturationRangeMax()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturationDefaultValue()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturationStepSize()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_VideoOverlay::GetSaturation()
{
	TJS_eTJSError(TJSNotImplemented);
	return 0.0f;
}
//---------------------------------------------------------------------------
void tTJSNI_VideoOverlay::SetSaturation( tjs_real v )
{
	TJS_eTJSError(TJSNotImplemented);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetOriginalWidth()
{
	return 0;
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_VideoOverlay::GetOriginalHeight()
{
	return 0;
}

//---------------------------------------------------------------------------
// tTJSNC_VideoOverlay::CreateNativeInstance : returns proper instance object
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_VideoOverlay::CreateNativeInstance()
{
	return new tTJSNI_VideoOverlay();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPCreateNativeClass_VideoOverlay
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_VideoOverlay()
{
	return new tTJSNC_VideoOverlay();
}
//---------------------------------------------------------------------------

