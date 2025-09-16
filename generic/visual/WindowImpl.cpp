//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Window" TJS Class implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <algorithm>
#include "MsgIntf.h"
#include "WindowIntf.h"
#include "LayerIntf.h"
#include "WindowForm.h"
#include "SysInitIntf.h"
#include "tjsHashSearch.h"
#include "StorageIntf.h"
#include "VideoOvlIntf.h"
#include "DebugIntf.h"
#include "PluginImpl.h"
#include "LayerManager.h"
#include "EventIntf.h"

#include "Application.h"
#include "tjsDictionary.h"


//---------------------------------------------------------------------------
// Mouse Cursor management
//---------------------------------------------------------------------------
tjs_int TVPGetCursor(const ttstr & name)
{
	return 0;	// not supported yet. always default.
}

//---------------------------------------------------------------------------
// tTJSNI_Window
//---------------------------------------------------------------------------
tTJSNI_Window::tTJSNI_Window()
 : Form(nullptr)
 , LayerWidth(0)
 , LayerHeight(0)
 , UpdateDestRect(true)
 , SetWindowHandleToDrawDevice(false)
{
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
tTJSNI_Window::Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj)
{
	tjs_error hr = tTJSNI_BaseWindow::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;

	Form = Application->CreateWindowForm(this);

	InitDrawContext(tjs_obj);

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::Invalidate()
{
	tTJSNI_BaseWindow::Invalidate();
	if(Form)
	{
		Form->InvalidateClose();
		Form = NULL;
	}

	// remove all events
	TVPCancelSourceEvents(Owner);
	TVPCancelInputEvents(this);

	// Set Owner null
	Owner = NULL;
}

//---------------------------------------------------------------------------
bool tTJSNI_Window::CanDeliverEvents() const
{
	if(!Form) return false;
	return GetVisible() && Form->GetFormEnabled();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::NotifyWindowClose()
{
	ClearVideo();
	Form = NULL;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::TickBeat()
{
	if(Form) Form->TickBeat();
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetWindowActive()
{
	if(Form) return Form->GetWindowActive();
	return false;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::ResetDrawDevice()
{
	// DrawDevice 初期化実行
	if (DrawDevice) {
		DrawDevice->SetTargetWindow(0, IsMainWindow());
		UpdateDestRect = true;
		SetWindowHandleToDrawDevice = true;
	}
}

//---------------------------------------------------------------------------
void tTJSNI_Window::UpdateContent()
{
	if (Form) {
		if (SetWindowHandleToDrawDevice && DrawDevice) {
			DrawDevice->SetTargetWindow((HWND)Form->NativeWindowHandle(), IsMainWindow());
			SetWindowHandleToDrawDevice = false;
		}
		if (UpdateDestRect) {
			tTVPRect destRect = Form->CalcDestRect(LayerWidth, LayerHeight);
			SetDestRectangle(destRect);
			UpdateDestRect = false;
		}
		if ( DrawDevice ) {
			// is called from event dispatcher
			DrawDevice->Update();
			DrawDevice->Show();
		}
		CheckVideoOverlay();
	}
	EndUpdate();
}

//---------------------------------------------------------------------------
void tTJSNI_Window::UpdateVideo(tjs_int w, tjs_int h, std::function<void(char *dest, int pitch)> updator)
{
	if (DrawDevice) {
		DrawDevice->UpdateVideo(w, h, updator);
	}
}

void tTJSNI_Window::ClearVideo()
{
	if (DrawDevice) {
		DrawDevice->ClearVideo();
	}
}

void 
tTJSNI_Window::AddVideoOverlay( tTJSNI_VideoOverlay *overlay ) 
{
	VideoOverlays.push_back( overlay );
	CheckVideoOverlay();
}

void 
tTJSNI_Window::DelVideoOverlay( tTJSNI_VideoOverlay *overlay ) 
{
	auto i = std::remove( VideoOverlays.begin(), VideoOverlays.end(), overlay );
	VideoOverlays.erase( i, VideoOverlays.end() );
	CheckVideoOverlay();
}

void 
tTJSNI_Window::CheckVideoOverlay()
{
    int mixer_count = 0;
	for (auto it=VideoOverlays.begin(); it!= VideoOverlays.end(); it++) {
		(*it)->CheckUpdate();
		if ((*it)->IsMixerPlaying()) {
            mixer_count++;
        }
	}
    // mixer 再生中のものが居なくなったら破棄
    if (mixer_count == 0) {
        ClearVideo();
    }
}

void 
tTJSNI_Window::UpdateVideoOverlay()
{
	for (auto it=VideoOverlays.begin(); it!= VideoOverlays.end(); it++) {
		(*it)->Update();
	}
	CheckVideoOverlay();
}

//---------------------------------------------------------------------------
void tTJSNI_Window::PostInputEvent(const ttstr &name, iTJSDispatch2 * params)
{
	// posts input event
	if(!Form) return;

	static ttstr key_name(TJS_W("key"));
	static ttstr shift_name(TJS_W("shift"));

	// check input event name
	enum tEventType
	{
		etUnknown, etOnKeyDown, etOnKeyUp, etOnKeyPress
	} type;

	if(name == TJS_W("onKeyDown"))
		type = etOnKeyDown;
	else if(name == TJS_W("onKeyUp"))
		type = etOnKeyUp;
	else if(name == TJS_W("onKeyPress"))
		type = etOnKeyPress;
	else
		type = etUnknown;

	if(type == etUnknown)
		TVPThrowExceptionMessage(TVPSpecifiedEventNameIsUnknown, name);


	if(type == etOnKeyDown || type == etOnKeyUp)
	{
		// this needs params, "key" and "shift"
		if(params == NULL)
			TVPThrowExceptionMessage(
				TVPSpecifiedEventNeedsParameter, name);


		tjs_uint key;
		tjs_uint32 shift = 0;

		tTJSVariant val;
		if(TJS_SUCCEEDED(params->PropGet(0, key_name.c_str(), key_name.GetHint(),
			&val, params)))
			key = (tjs_int)val;
		else
			TVPThrowExceptionMessage(TVPSpecifiedEventNeedsParameter2,
				name, TJS_W("key"));

		if(TJS_SUCCEEDED(params->PropGet(0, shift_name.c_str(), shift_name.GetHint(),
			&val, params)))
			shift = (tjs_int)val;
		else
			TVPThrowExceptionMessage(TVPSpecifiedEventNeedsParameter2,
				name, TJS_W("shift"));

		tjs_uint16 vcl_key = key;
		if(type == etOnKeyDown)
			Form->InternalKeyDown(key, shift);
		else if(type == etOnKeyUp)
			//Form->OnKeyUp(Form, vcl_key, TVP_TShiftState_From_uint32(shift));
			Form->OnKeyUp( vcl_key, TVP_TShiftState_From_uint32(shift) );
	}
	else if(type == etOnKeyPress)
	{
		// this needs param, "key"
		if(params == NULL)
			TVPThrowExceptionMessage(
				TVPSpecifiedEventNeedsParameter, name);


		tjs_uint key;

		tTJSVariant val;
		if(TJS_SUCCEEDED(params->PropGet(0, key_name.c_str(), key_name.GetHint(),
			&val, params)))
			key = (tjs_int)val;
		else
			TVPThrowExceptionMessage(TVPSpecifiedEventNeedsParameter2,
				name, TJS_W("key"));

		char vcl_key = key;
		//Form->OnKeyPress(Form, vcl_key);
		Form->OnKeyPress(vcl_key,0,false,false);
	}
}

//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::NotifySrcResize()
{
	tTJSNI_BaseWindow::NotifySrcResize();

	// is called from primary layer
	// ( or from TWindowForm to reset paint box's size )
	if (DrawDevice) {
		DrawDevice->GetSrcSize(LayerWidth, LayerHeight);
		UpdateDestRect = true;
	}
}

//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetDefaultMouseCursor()
{
	// set window mouse cursor to default
	if(Form) Form->SetDefaultMouseCursor();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetMouseCursor(tjs_int handle)
{
	// set window mouse cursor
	if(Form) Form->SetMouseCursor(handle);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::GetCursorPos(tjs_int &x, tjs_int &y)
{
	// get cursor pos in primary layer's coordinates
	if(Form) Form->GetCursorPos(x, y);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetCursorPos(tjs_int x, tjs_int y)
{
	// set cursor pos in primar layer's coordinates
	if(Form) {
		Form->SetCursorPos(x, y);
		Form->UpdateCursorPos(x, y);
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::WindowReleaseCapture()
{
//	::ReleaseCapture(); // Windows API
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetHintText(iTJSDispatch2* sender, const ttstr & text)
{
	// set hint text to window
	if(Form) Form->SetHintText(sender,text);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetAttentionPoint(tTJSNI_BaseLayer *layer,
	tjs_int l, tjs_int t)
{
	// set attention point to window
	if(Form)
	{
		//class TFont * font = NULL;
		const tTVPFont * font = NULL;
		if(layer)
		{
			tTVPBaseBitmap *bmp = layer->GetMainImage();
			if(bmp) {
				//font = bmp->GetFontCanvas()->GetFont(); =
				// font = bmp->GetFontCanvas();
				const tTVPFont & finfo = bmp->GetFont();
				font = &finfo;
			}
		}

		Form->SetAttentionPoint(l, t, font);
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::DisableAttentionPoint()
{
	// disable attention point
	if(Form) Form->DisableAttentionPoint();
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetImeMode(tTVPImeMode mode)
{
	// set ime mode
	if(Form) Form->SetImeMode(mode);
}
//---------------------------------------------------------------------------
/*void tTJSNI_Window::SetDefaultImeMode(tTVPImeMode mode)
{
	// set default ime mode
	if(Form)
	{
//		Form->SetDefaultImeMode(mode, LayerManager->GetFocusedLayer() == NULL);
	}
}
//---------------------------------------------------------------------------
tTVPImeMode tTJSNI_Window::GetDefaultImeMode() const
{
	if(Form) return Form->GetDefaultImeMode();
	return ::imDisable;
}*/
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::ResetImeMode()
{
	// set default ime mode ( default mode is imDisable; IME is disabled )
	if(Form) Form->ResetImeMode();
}
//---------------------------------------------------------------------------
/*void tTJSNI_Window::RegisterWindowMessageReceiver(tTVPWMRRegMode mode,
		void * proc, const void *userdata)
{
	if(!Form) return;
	Form->RegisterWindowMessageReceiver(mode, proc, userdata);
}*/
//---------------------------------------------------------------------------
void tTJSNI_Window::Close()
{
	if(Form) Form->Close();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::OnCloseQueryCalled(bool b)
{
	if(Form) Form->OnCloseQueryCalled(b);
}
//---------------------------------------------------------------------------
#ifdef USE_OBSOLETE_FUNCTIONS
void tTJSNI_Window::BeginMove()
{
	if(Form) Form->BeginMove();
}
#endif
//---------------------------------------------------------------------------
void tTJSNI_Window::BringToFront()
{
	if(Form) Form->BringToFront();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::Update(tTVPUpdateType type)
{
	NotifyWindowExposureToLayer(tTVPRect(0,0,LayerWidth,LayerHeight));
	TVPDeliverWindowUpdateEvents();
}

//---------------------------------------------------------------------------
void tTJSNI_Window::ShowModal()
{
	if(Form)
	{
		TVPClearAllWindowInputEvents();
			// cancel all input events that can cause delayed operation
		Form->ShowWindowAsModal();
	}
}
//---------------------------------------------------------------------------
void tTJSNI_Window::HideMouseCursor()
{
	if(Form) Form->HideMouseCursor();
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetVisible() const
{
	if(!Form) return false;
	return Form->GetVisible();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetVisible(bool s)
{
	if( Form ) Form->SetVisibleFromScript(s);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::GetCaption(ttstr & v) const
{
	v = mCaption;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetCaption(const ttstr & v)
{
	mCaption = v;
	if(Form) Form->SetCaption( v.AsStdString() );
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetWidth(tjs_int w)
{
	if( Form ) Form->SetWidth( w );
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetWidth() const
{
	if(!Form) return 0;
	return Form->GetWidth();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetHeight(tjs_int h)
{
	if( Form ) Form->SetHeight( h );
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetHeight() const
{
	if(!Form) return 0;
	return Form->GetHeight();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetLeft(tjs_int l)
{
	if(Form) Form->SetLeft( l );
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetLeft() const
{
	if(!Form) return 0;
	return Form->GetLeft();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetTop(tjs_int t)
{
	if(Form) Form->SetTop( t );
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetTop() const
{
	if(!Form) return 0;
	return Form->GetTop();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetSize(tjs_int w, tjs_int h)
{
	if(Form) Form->SetSize( w, h );
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMinWidth(int v)
{
	if(Form) Form->SetMinWidth( v );
}
//---------------------------------------------------------------------------
int  tTJSNI_Window::GetMinWidth() const
{
	if(Form) return Form->GetMinWidth(); else return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMinHeight(int v)
{
	if(Form) Form->SetMinHeight( v );
}
//---------------------------------------------------------------------------
int  tTJSNI_Window::GetMinHeight() const
{
	if(Form) return Form->GetMinHeight(); else return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMinSize(int w, int h)
{
	if(Form) Form->SetMinSize( w, h );
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMaxWidth(int v)
{
	if(Form) Form->SetMaxWidth( v );
}
//---------------------------------------------------------------------------
int  tTJSNI_Window::GetMaxWidth() const
{
	if(Form) return Form->GetMaxWidth(); else return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMaxHeight(int v)
{
	if(Form) Form->SetMaxHeight( v );
}
//---------------------------------------------------------------------------
int  tTJSNI_Window::GetMaxHeight() const
{
	if(Form) return Form->GetMaxHeight(); else return 0;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMaxSize(int w, int h)
{
	if(Form) Form->SetMaxSize( w, h );
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetPosition(tjs_int l, tjs_int t)
{
	if(Form) Form->SetPosition( l, t );
}
//---------------------------------------------------------------------------
#ifdef USE_OBSOLETE_FUNCTIONS
void tTJSNI_Window::SetLayerLeft(tjs_int l)
{
	if(Form) Form->SetLayerLeft(l);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetLayerLeft() const
{
	if(!Form) return 0;
	return Form->GetLayerLeft();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetLayerTop(tjs_int t)
{
	if(Form) Form->SetLayerTop(t);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetLayerTop() const
{
	if(!Form) return 0;
	return Form->GetLayerTop();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetLayerPosition(tjs_int l, tjs_int t)
{
	if(Form) Form->SetLayerPosition(l, t);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetInnerSunken(bool b)
{
	if(Form) Form->SetInnerSunken(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetInnerSunken() const
{
	if(!Form) return true;
	return Form->GetInnerSunken();
}
#endif
//---------------------------------------------------------------------------
void tTJSNI_Window::SetInnerWidth(tjs_int w)
{
	if(Form) Form->SetInnerWidth(w);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetInnerWidth() const
{
	if(!Form) return 0;
	return Form->GetInnerWidth();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetInnerHeight(tjs_int h)
{
	if(Form) Form->SetInnerHeight(h);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetInnerHeight() const
{
	if(!Form) return 0;
	return Form->GetInnerHeight();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetInnerSize(tjs_int w, tjs_int h)
{
	if(Form) Form->SetInnerSize(w, h);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetBorderStyle(tTVPBorderStyle st)
{
	if(Form) Form->SetBorderStyle(st);
}
//---------------------------------------------------------------------------
tTVPBorderStyle tTJSNI_Window::GetBorderStyle() const
{
	if(!Form) return (tTVPBorderStyle)0;
	return Form->GetBorderStyle();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetStayOnTop(bool b)
{
	if(!Form) return;
	Form->SetStayOnTop(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetStayOnTop() const
{
	if(!Form) return false;
	return Form->GetStayOnTop();
}
//---------------------------------------------------------------------------
#ifdef USE_OBSOLETE_FUNCTIONS
void tTJSNI_Window::SetShowScrollBars(bool b)
{
	if(Form) Form->SetShowScrollBars(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetShowScrollBars() const
{
	if(!Form) return true;
	return Form->GetShowScrollBars();
}
#endif
//---------------------------------------------------------------------------
void tTJSNI_Window::SetFullScreen(bool b)
{
	if(!Form) return;
	Form->SetFullScreenMode(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetFullScreen() const
{
	if(!Form) return false;
	return Form->GetFullScreenMode();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetUseMouseKey(bool b)
{
	if(!Form) return;
	Form->SetUseMouseKey(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetUseMouseKey() const
{
	if(!Form) return false;
	return Form->GetUseMouseKey();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetTrapKey(bool b)
{
	if(!Form) return;
	Form->SetTrapKey(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetTrapKey() const
{
	if(!Form) return false;
	return Form->GetTrapKey();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetMouseCursorState(tTVPMouseCursorState mcs)
{
	if(!Form) return;
	Form->SetMouseCursorState(mcs);
}
//---------------------------------------------------------------------------
tTVPMouseCursorState tTJSNI_Window::GetMouseCursorState() const
{
	if(!Form) return mcsVisible;
	return Form->GetMouseCursorState();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetFocusable(bool b)
{
	if(!Form) return;
	Form->SetFocusable(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetFocusable()
{
	if(!Form) return true;
	return Form->GetFocusable();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetZoom(tjs_int numer, tjs_int denom)
{
	if(!Form) return;
	Form->SetZoom(numer, denom);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetZoomNumer(tjs_int n)
{
	if(!Form) return;
	Form->SetZoomNumer(n);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetZoomNumer() const
{
	if(!Form) return 1;
	return Form->GetZoomNumer();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetZoomDenom(tjs_int n)
{
	if(!Form) return;
	Form->SetZoomDenom(n);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetZoomDenom() const
{
	if(!Form) return 1;
	return Form->GetZoomDenom();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetTouchScaleThreshold( tjs_real threshold ) {
	if(!Form) return;
	Form->SetTouchScaleThreshold(threshold);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_Window::GetTouchScaleThreshold() const {
	if(!Form) return 0;
	return Form->GetTouchScaleThreshold();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetTouchRotateThreshold( tjs_real threshold ) {
	if(!Form) return;
	Form->SetTouchRotateThreshold(threshold);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_Window::GetTouchRotateThreshold() const {
	if(!Form) return 0;
	return Form->GetTouchRotateThreshold();
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_Window::GetTouchPointStartX( tjs_int index ) {
	if(!Form) return 0;
	return Form->GetTouchPointStartX(index);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_Window::GetTouchPointStartY( tjs_int index ) {
	if(!Form) return 0;
	return Form->GetTouchPointStartY(index);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_Window::GetTouchPointX( tjs_int index ) {
	if(!Form) return 0;
	return Form->GetTouchPointX(index);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_Window::GetTouchPointY( tjs_int index ) {
	if(!Form) return 0;
	return Form->GetTouchPointY(index);
}
//---------------------------------------------------------------------------
tjs_real tTJSNI_Window::GetTouchPointID( tjs_int index ) {
	if(!Form) return 0;
	return Form->GetTouchPointID(index);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetTouchPointCount() {
	if(!Form) return 0;
	return Form->GetTouchPointCount();
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetTouchVelocity( tjs_int id, float& x, float& y, float& speed ) const {
	if(!Form) return false;
	return Form->GetTouchVelocity( id, x, y, speed );
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetMouseVelocity( float& x, float& y, float& speed ) const {
	if(!Form) return false;
	return Form->GetMouseVelocity( x, y, speed );
}
//---------------------------------------------------------------------------
void tTJSNI_Window::ResetMouseVelocity() {
	if(!Form) return;
	return Form->ResetMouseVelocity();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetHintDelay( tjs_int delay )
{
	if(!Form) return;
	Form->SetHintDelay(delay);
}
//---------------------------------------------------------------------------
tjs_int tTJSNI_Window::GetHintDelay() const
{
	if(!Form) return 0;
	return Form->GetHintDelay();
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetEnableTouch( bool b )
{
	if(!Form) return;
	Form->SetEnableTouch(b);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::SetEnableTouchMouse( bool b )
{
	if(!Form) return;
	Form->SetEnableTouchMouse(b);
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetEnableTouchMouse() const
{
	if(!Form) return 0;
	return Form->GetEnableTouchMouse();
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::GetEnableTouch() const
{
	if(!Form) return 0;
	return Form->GetEnableTouch();
}
//---------------------------------------------------------------------------
int tTJSNI_Window::GetDisplayOrientation()
{
	if(!Form) return TTVPWindowForm::orientUnknown;
	return Form->GetDisplayOrientation();
}
//---------------------------------------------------------------------------
int tTJSNI_Window::GetDisplayRotate()
{
	if(!Form) return -1;
	return Form->GetDisplayRotate();
}
//---------------------------------------------------------------------------
bool tTJSNI_Window::WaitForVBlank( tjs_int* in_vblank, tjs_int* delayed )
{
	if( DrawDevice ) return DrawDevice->WaitForVBlank( in_vblank, delayed );
	return false;
}
//---------------------------------------------------------------------------
void tTJSNI_Window::UpdateWaitVSync()
{
	if (DrawDevice) {
		DrawDevice->SetWaitVSync(WaitVSync);
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::StartBitmapCompletion(iTVPLayerManager * manager)
{
	if( DrawDevice ) DrawDevice->StartBitmapCompletion(manager);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::NotifyBitmapCompleted(class iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const class BitmapInfomation * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	if( DrawDevice ) {
		DrawDevice->NotifyBitmapCompleted(manager,x,y,bits,bitmapinfo, cliprect, type, opacity );
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::EndBitmapCompletion(iTVPLayerManager * manager)
{
	if( DrawDevice ) DrawDevice->EndBitmapCompletion(manager);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetMouseCursor(class iTVPLayerManager* manager, tjs_int cursor)
{
	if( DrawDevice ) {
		if(cursor == 0)
			DrawDevice->SetDefaultMouseCursor(manager);
		else
			DrawDevice->SetMouseCursor(manager, cursor);
	}
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::GetCursorPos(class iTVPLayerManager* manager, tjs_int &x, tjs_int &y)
{
	if( DrawDevice ) DrawDevice->GetCursorPos(manager, x, y);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetCursorPos(class iTVPLayerManager* manager, tjs_int x, tjs_int y)
{
	if( DrawDevice ) DrawDevice->SetCursorPos(manager, x, y);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::ReleaseMouseCapture(class iTVPLayerManager* manager)
{
	if( DrawDevice ) DrawDevice->WindowReleaseCapture(manager);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetHint(class iTVPLayerManager* manager, iTJSDispatch2* sender, const ttstr &hint)
{
	if( DrawDevice ) DrawDevice->SetHintText(manager, sender, hint);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::NotifyLayerResize(class iTVPLayerManager* manager)
{
	if( DrawDevice ) DrawDevice->NotifyLayerResize(manager);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::NotifyLayerImageChange(class iTVPLayerManager* manager)
{
	if( DrawDevice ) DrawDevice->NotifyLayerImageChange(manager);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetAttentionPoint(class iTVPLayerManager* manager, tTJSNI_BaseLayer *layer, tjs_int x, tjs_int y)
{
	if( DrawDevice ) DrawDevice->SetAttentionPoint(manager, layer, x, y);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::DisableAttentionPoint(class iTVPLayerManager* manager)
{
	if( DrawDevice ) DrawDevice->DisableAttentionPoint(manager);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::SetImeMode( class iTVPLayerManager* manager, tjs_int mode ) // mode == tTVPImeMode
{
	if( DrawDevice ) DrawDevice->SetImeMode(manager, (tTVPImeMode)mode);
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_Window::ResetImeMode( class iTVPLayerManager* manager )
{
	if( DrawDevice ) DrawDevice->ResetImeMode(manager);
}
//---------------------------------------------------------------------------
void tTJSNI_Window::OnTouchUp( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id )
{
	tTJSNI_BaseWindow::OnTouchUp( x, y, cx, cy, id );
	if( Form )
	{
		Form->ResetTouchVelocity( id );
	}
}


//---------------------------------------------------------------------------
// tTJSNC_Window::CreateNativeInstance : returns proper instance object
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_Window::CreateNativeInstance()
{
	return new tTJSNI_Window();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPCreateNativeClass_Window
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Window()
{
	tTJSNativeClass *cls = new tTJSNC_Window();
	static tjs_uint32 TJS_NCM_CLASSID;
	TJS_NCM_CLASSID = tTJSNC_Window::ClassID;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(getTouchPoint)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	tjs_int index = (tjs_int)*param[0];
	if( index < _this->GetTouchPointCount() ) {
		if( result ) {
			iTJSDispatch2 * object = TJSCreateDictionaryObject();

			static ttstr startX_name(TJS_W("startX"));
			static ttstr startY_name(TJS_W("startY"));
			static ttstr X_name(TJS_W("x"));
			static ttstr Y_name(TJS_W("y"));
			static ttstr ID_name(TJS_W("ID"));
			{
				tTJSVariant val(_this->GetTouchPointStartX(index));
				if(TJS_FAILED(object->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, startX_name.c_str(), startX_name.GetHint(), &val, object)))
						TVPThrowInternalError;
			}
			{
				tTJSVariant val(_this->GetTouchPointStartY(index));
				if(TJS_FAILED(object->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, startY_name.c_str(), startY_name.GetHint(), &val, object)))
						TVPThrowInternalError;
			}
			{
				tTJSVariant val(_this->GetTouchPointX(index));
				if(TJS_FAILED(object->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, X_name.c_str(), X_name.GetHint(), &val, object)))
						TVPThrowInternalError;
			}
			{
				tTJSVariant val(_this->GetTouchPointY(index));
				if(TJS_FAILED(object->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, Y_name.c_str(), Y_name.GetHint(), &val, object)))
						TVPThrowInternalError;
			}
			{
				tTJSVariant val(_this->GetTouchPointID(index));
				if(TJS_FAILED(object->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, ID_name.c_str(), ID_name.GetHint(), &val, object)))
						TVPThrowInternalError;
			}
			tTJSVariant objval(object, object);
			object->Release();
			*result = objval;
		}
	} else {
		return TJS_E_INVALIDPARAM;
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(cls, getTouchPoint)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(getTouchVelocity)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
	if(numparams < 4) return TJS_E_BADPARAMCOUNT;

	tjs_int id = (tjs_int)*param[0];
	float x, y, speed;
	bool ret = _this->GetTouchVelocity( id, x, y, speed );
	if( result ) {
		*result = ret ? (tjs_int)1 : (tjs_int)0;
	}
	if( ret ) {
		(*param[1]) = (tjs_real)x;
		(*param[2]) = (tjs_real)y;
		(*param[3]) = (tjs_real)speed;
	} else {
		(*param[1]) = (tjs_real)0.0;
		(*param[2]) = (tjs_real)0.0;
		(*param[3]) = (tjs_real)0.0;
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(cls, getTouchVelocity)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(getMouseVelocity)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
	if(numparams < 3) return TJS_E_BADPARAMCOUNT;

	float x, y, speed;
	bool ret = _this->GetMouseVelocity( x, y, speed );
	if( result ) {
		*result = ret ? (tjs_int)1 : (tjs_int)0;
	}
	if( ret ) {
		(*param[0]) = (tjs_real)x;
		(*param[1]) = (tjs_real)y;
		(*param[2]) = (tjs_real)speed;
	} else {
		(*param[0]) = (tjs_real)0.0;
		(*param[1]) = (tjs_real)0.0;
		(*param[2]) = (tjs_real)0.0;
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(cls, getMouseVelocity)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(resetMouseVelocity)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
	_this->ResetMouseVelocity();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL_OUTER(cls, resetMouseVelocity)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(drawDevice)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetDrawDeviceObject();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		_this->SetDrawDeviceObject(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, drawDevice)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(touchScaleThreshold)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetTouchScaleThreshold();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		_this->SetTouchScaleThreshold(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, touchScaleThreshold)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(touchRotateThreshold)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetTouchRotateThreshold();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		_this->SetTouchRotateThreshold(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, touchRotateThreshold)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(touchPointCount)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetTouchPointCount();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, touchPointCount)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(hintDelay)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetHintDelay();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		_this->SetHintDelay(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, hintDelay)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(enableTouch)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetEnableTouch()?1:0;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		_this->SetEnableTouch( ((tjs_int)*param) ? true : false );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, enableTouch)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(enableTouchMouse)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetEnableTouchMouse()?1:0;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		_this->SetEnableTouchMouse( ((tjs_int)*param) ? true : false );
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, enableTouchMouse)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(displayOrientation)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetDisplayOrientation();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, displayOrientation)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(displayRotate)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_Window);
		*result = _this->GetDisplayRotate();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL_OUTER(cls, displayRotate)
//---------------------------------------------------------------------------

	//TVPGetDisplayColorFormat(); // this will be ran only once here

	return cls;
}
//---------------------------------------------------------------------------
