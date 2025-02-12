#include "tjsCommHead.h"
#include "CharacterSet.h"
#include "DebugIntf.h"

#include "OGLApplication.h"
#include "VideoOvlImpl.h"
#include "GLTexture.h"
#include "OpenGLContext.h"

OGLWindowForm::OGLWindowForm(class tTJSNI_Window* win)
 : TTVPWindowForm(win)
 , glContext(0)
 , _video_texture(0)
{
    // 描画位置指定・いったん全画面対象
    _video_position[0] = -1.0f; // left top
    _video_position[1] =  1.0f;
    _video_position[2] = -1.0f; // left bottom
    _video_position[3] = -1.0f;
    _video_position[4] =  1.0f; // right top
    _video_position[5] =  1.0f;
    _video_position[6] =  1.0f; // right bottom
    _video_position[7] = -1.0f;
}

void 
OGLWindowForm::SetWaitVSync(bool waitVSync) 
{
    if (glContext) {
        glContext->SetWaitVSync(waitVSync);
    }
}

void
OGLWindowForm::InitNativeWindow()
{
    if (!glContext) {
        glContext = iTVPGLContext::GetContext(NativeWindowHandle());
        if (glContext) {
            glContext->MakeCurrent();
            InitGLES();
            glContext->GetSurfaceSize(&mSurfaceWidth, &mSurfaceHeight);
            mTextureDrawer.Init();
        }
    }
    //Application->DPRINTF(TJS_W("init size:%d,%d"), mSurfaceWidth, mSurfaceHeight);
}

void
OGLWindowForm::DoneNativeWindow()
{
    if (glContext) {
        ClearVideo();
        mTextureDrawer.Done();
        glContext->Release();
        glContext = 0;
    }
}

OGLWindowForm::~OGLWindowForm()
{
}

void 
OGLWindowForm::OnClose()
{
    ClearVideo();
    TTVPWindowForm::OnClose();
}

void
OGLWindowForm::OnResize()
{
    if (glContext) {
        glContext->GetSurfaceSize(&mSurfaceWidth, &mSurfaceHeight);
    }
    TTVPWindowForm::OnResize();
}

bool
OGLWindowForm::UpdateContent()
{
    // ビデオ描画中
    if (glContext && _video_texture) {
        glContext->MakeCurrent();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GLDrawTexture(_video_texture, mSurfaceWidth, mSurfaceHeight, _video_position);
        glContext->Swap();
        return true;
    }
    return false;
}

void 
OGLWindowForm::UpdateVideoPosition(int w, int h)
{
    // 配置座標計算 (内接表示で補正)
    int sw =  mSurfaceWidth;
    int sh =  mSurfaceHeight;
    if (sw > 0 && sh > 0) {
        // 描画位置
        double scale = std::min((double)sw/w, (double)sh/h);
        int nw = w * scale;
        int nh = h * scale;
        int offx = (sw-nw)/2;
        int offy = (sh-nh)/2;

        // 描画位置を position換算
        int w2 = sw/2;
        int h2 = sh/2;
        float left    = (float)(offx      - w2) / w2;
        float top     = (float)(offy      - h2) / h2;
        float right   = (float)(offx + nw - w2) / w2;
        float bottom  = (float)(offy + nh - h2) / h2;

        //DPRINTF(TJS_W("dest: %f,%f,%f,%f\n"), left, top, right, bottom);
        _video_position[0] = left; // left top
        _video_position[1] = top;
        _video_position[2] = left; // left bottom
        _video_position[3] = bottom;
        _video_position[4] = right; // right top
        _video_position[5] = top;
        _video_position[6] = right; // right bottom
        _video_position[7] = bottom;
    }
}

void 
OGLWindowForm::UpdateVideo(int w, int h, std::function<void(char *dest, int pitch)> updator)
{
    if (!glContext) return;
    glContext->MakeCurrent();

    if (!_video_texture) {
        _video_texture = new GLTexture(w, h, 0, GL_BGRA_EXT);
        UpdateVideoPosition(w, h);
    }
    if (_video_texture) {
        _video_texture->UpdateTexture(0, 0, w, h, updator);
		if( TJSNativeInstance ) {
            TJSNativeInstance->RequestUpdate();
        }
    }
}

void 
OGLWindowForm::ClearVideo()
{
    if (_video_texture) {
        delete _video_texture;
        _video_texture = 0;
    }
}

// 再生状態になった VideoOverlay を登録
void 
OGLWindowForm::AddVideoOverlay( tTJSNI_VideoOverlay *overlay ) 
{
	std::lock_guard<std::mutex> lock( videooverlay_mutex_ );
	VideoOverlays.push_back( overlay );
	CheckVideoOverlay();
}

// 再生完了したものは消す
void 
OGLWindowForm::DelVideoOverlay( tTJSNI_VideoOverlay *overlay ) 
{
	std::lock_guard<std::mutex> lock( videooverlay_mutex_ );
	auto i = std::remove( VideoOverlays.begin(), VideoOverlays.end(), overlay );
	VideoOverlays.erase( i, VideoOverlays.end() );
	CheckVideoOverlay();
}

void 
OGLWindowForm::UpdateVideoOverlay() 
{
	std::lock_guard<std::mutex> lock( videooverlay_mutex_ );
	bool remove = false;
	auto it=VideoOverlays.begin(); 
	while (it!= VideoOverlays.end()) {
		if (!(*it)->Update()) {
			// 再生してないものは消す
			it = VideoOverlays.erase(it);
			remove = true;
		} else {
			it++;
		}
	}
	if (remove) {
		CheckVideoOverlay();
	}
}

void 
OGLWindowForm::CheckVideoOverlay()
{
    int mixer_count = 0;
	for (auto it=VideoOverlays.begin(); it!= VideoOverlays.end(); it++) {
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
OGLWindowForm::GLDrawTexture(class GLTexture *tex, int scr_w, int scr_h, float position[], int tex_w, int tex_h)
{
    mTextureDrawer.DrawTexture(tex, scr_w, scr_h, position, tex_w, tex_h);
}




OGLApplication::OGLApplication() 
: tTVPApplication() 
{
}


#include "OGLDrawDevice.h"

// アプリ処理用の DrawDevice実装を返す
tTJSNativeClass* 
OGLApplication::CreateDrawDevice()
{
    return new tTJSNC_OGLDrawDevice();
}
