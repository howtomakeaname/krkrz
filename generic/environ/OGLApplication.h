#ifndef __OGLApplication_h
#define __OGLApplication_h

#include "tjsCommHead.h"
#include "WindowForm.h"
#include "GLTexture.h"

/**
 * @brief OGLむけ WindowForm
 * 未実装部分を環境別に実装する
 */
class OGLWindowForm : public TTVPWindowForm 
{

public:
	virtual void SetWaitVSync(bool waitVSync);
	virtual void InitNativeWindow();
	virtual void DoneNativeWindow();

	// クローズ制御
	virtual void OnClose();
	virtual void OnResize();

	virtual bool UpdateContent();

	// VideoOverlay 対応
	virtual void UpdateVideo(int w, int h, std::function<void(char *dest, int pitch)> updator);
	virtual void AddVideoOverlay( tTJSNI_VideoOverlay *overlay );
	virtual void DelVideoOverlay( tTJSNI_VideoOverlay *overlay );
	virtual void UpdateVideoOverlay();
	virtual void GLDrawTexture(class GLTexture *tex, int scr_w, int scr_h, float position[], int tex_w=0, int tex_h=0);

protected:

	OGLWindowForm(class tTJSNI_Window* win);
	virtual ~OGLWindowForm();
	
	// ----------------------------------------------------
	// VideoOverlay 関連処理
	// ----------------------------------------------------

	class iTVPGLContext *glContext;

	// 動画用テクスチャ
	GLTexture *_video_texture;
	GLfloat _video_position[8];

	void ClearVideo();
	void UpdateVideoPosition(int w, int h);

	std::mutex videooverlay_mutex_;
	std::vector<tTJSNI_VideoOverlay *> VideoOverlays;
	void CheckVideoOverlay();

	GLTextureDrawer mTextureDrawer;
};

class OGLApplication : public tTVPApplication
{
public:
	OGLApplication();

	// アプリ処理用の DrawDevice実装を返す
	virtual tTJSNativeClass* CreateDrawDevice();

	virtual void Terminate() {};
	virtual void Exit(int code) {};
};


#endif
