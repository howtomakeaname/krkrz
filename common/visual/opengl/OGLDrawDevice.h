#ifndef OGL_DRAW_DEVICE_H
#define OGL_DRAW_DEVICE_H

#include "DrawDevice.h"
#include "OpenGLHeader.h"
#include "TextureUpdateRect.h"
#include "GLTexture.h"
#include <functional>

//---------------------------------------------------------------------------
//! @brief	OpelGL のテクスチャに描画する想定の DrawDevice
//---------------------------------------------------------------------------
class tTVPOGLDrawDevice : public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

	iTJSDispatch2 *Owner;
	iTJSDispatch2 *Self;
	tTJSVariant WindowObject;

	class iTVPGLContext *GLContext;
	GLTextureDrawer TextureDrawer;

	// primaryLayer 描画用テクスチャ
	tTJSNativeClass *TextureClass;
	tTJSVariant TextureObject;
	class tTJSNI_Texture *TextureInstance;
	TextureUpdateRect mTextureUpdateRect;

	// primaryLayer 描画用座標値
	tTJSVariant MatrixObject;
	class tTJSNI_Matrix32 *MatrixInstance;

	GLfloat _position[8];
	int SurfaceWidth;
	int SurfaceHeight;

	~tTVPOGLDrawDevice(); //!< デストラクタ

	void CreateTexture();
	void DestroyTexture();
	void UpdateTexture(int x, int y, int w, int h, std::function<void(char *dest, int pitch)> updator);

	void InitPosition();
	void InitMatrix();
	void InitUV();

	void InitContext(void *nativeWindow);
	void DoneContext();

	// -----------------------------------------------------
	// Canvas Interface
	// -----------------------------------------------------

	bool DoCreateCanvas;

	void CreateCanvas();
	void DestroyCanvas();
	
	tTJSVariant CanvasObject; //!< Current Canvas TJS2 Object
	class tTJSNI_Canvas* CanvasInstance;
	void SetCanvasObject(const tTJSVariant & val);

public:
	void RequestCreateCanvas();
	const tTJSVariant & GetCanvasObject() const { return CanvasObject; }
	const tTJSVariant & GetWindowObject() const { return WindowObject; }
	const tTJSVariant & GetTextureObject() const { return TextureObject; }
	const tTJSVariant & GetMatrixObject() const { return MatrixObject; }

public:
	tTVPOGLDrawDevice(iTJSDispatch2 *tjs_obj); //!< コンストラクタ

	//---- オブジェクト生存期間制御
	virtual void TJS_INTF_METHOD Destruct();

	//---- window interface 関連
	virtual void TJS_INTF_METHOD SetWindowInterface(iTVPWindow * window);

    //---- LayerManager の管理関連
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);

    //---- 描画位置・サイズ関連
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main);
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);

	//----- 表示処理
	virtual void TJS_INTF_METHOD Show();

    //---- LayerManager からの画像受け渡し関連
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const class BitmapInfomation * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTJSNI_OGLDrawDevice
//---------------------------------------------------------------------------
class tTJSNI_OGLDrawDevice :
	public tTJSNativeInstance
{
	typedef tTJSNativeInstance inherited;

	tTVPOGLDrawDevice * Device;

public:
	tTJSNI_OGLDrawDevice();
	~tTJSNI_OGLDrawDevice();
	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param,
			iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

public:
	tTVPOGLDrawDevice * GetDevice() const { return Device; }

};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTJSNC_OGLDrawDevice
//---------------------------------------------------------------------------
class tTJSNC_OGLDrawDevice : public tTJSNativeClass
{
public:
	tTJSNC_OGLDrawDevice();

	static tjs_uint32 ClassID;

private:
	iTJSNativeInstance *CreateNativeInstance();
};
//---------------------------------------------------------------------------


#endif // OGL_DRAW_DEVICE_H