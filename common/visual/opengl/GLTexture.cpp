#include "tjsCommHead.h"
#include "LogIntf.h"
#include "GLTexture.h"

#include <memory>

extern int TVPOpenGLESVersion;

void 
GLTexture::create( GLuint w, GLuint h, const GLvoid* bits, tTVPTextureColorFormat format) 
{
    int pixel_size;
    GLuint fmt;
    
    if (format == tTVPTextureColorFormat::Alpha) {
        pixel_size = 1;
        glformat_ = GL_ALPHA;
        fmt = GL_R8;
    } else {
        pixel_size = 4;
        if (GLTexture::SupportBGRAFormat()) {
            glformat_ = GL_BGRA_EXT;
            fmt = GL_BGRA8_EXT;
        } else {
            glformat_ = GL_RGBA;
            fmt = GL_RGBA8;
        }
    }

    format_ = format;
    width_ = w;
    height_ = h;

    glPixelStorei( GL_UNPACK_ALIGNMENT, pixel_size);
    glGenTextures( 1, &texture_id_ );
    glBindTexture( GL_TEXTURE_2D, texture_id_ );

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,stretchType_);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,stretchType_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT_);

    if (format == tTVPTextureColorFormat::Alpha) {
		glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bits );
		glBindTexture( GL_TEXTURE_2D, 0 );
        return;
    }

    glTexStorage2D(GL_TEXTURE_2D, 1, fmt, w, h);
    CheckGLErrorAndLog("glTexStorage2D");

    // PBO を作成
    int size = w * h * pixel_size;
    glGenBuffers(1, &pbo_);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    if (bits) {
        if (pbo_) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
            GLubyte *texPixels = (GLubyte *)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, GL_MAP_WRITE_BIT);
            if (texPixels) {
                memcpy(texPixels, bits, size);
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            }
            //glTexImage2D( GL_TEXTURE_2D, 0, fmt, w, h, 0, xformat, GL_UNSIGNED_BYTE, 0 );
            //CheckGLErrorAndLog("glTexImage2D");
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, glformat_, GL_UNSIGNED_BYTE, 0);
            CheckGLErrorAndLog("glTexSubImage2D");
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        } else {
            //glTexImage2D( GL_TEXTURE_2D, 0, fmt, w, h, 0, format, GL_UNSIGNED_BYTE, bits );
            //CheckGLErrorAndLog("glTexImage2D");
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, glformat_, GL_UNSIGNED_BYTE, bits);
            CheckGLErrorAndLog("glTexSubImage2D");
        }
    }

    if (glformat_ == GL_RGBA && _support_swizzle) {
        // スウィズルで R と B を入れ替える 
        TVPLOG_DEBUG("GLES: Create Texture && Set Swizzle");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    }

    glBindTexture( GL_TEXTURE_2D, 0 );
}

void 
GLTexture::createMipmapTexture( std::vector<GLTextreImageSet>& img ) 
{
    if( img.size() > 0 ) {
        GLuint w = img[0].width;
        GLuint h = img[0].height;
        glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
        glGenTextures( 1, &texture_id_ );
        glBindTexture( GL_TEXTURE_2D, texture_id_ );

        GLint count = img.size();
        if( count > 1 ) hasMipmap_ = true;

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, stretchType_ );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getMipmapFilter( stretchType_ ) );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS_ );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT_ );
        // ミップマップの最小と最大レベルを指定する、これがないと存在しないレベルを参照しようとすることが発生しうる
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, count - 1 );
        if( TVPOpenGLESVersion < 300 ) {
            // OpenGL ES2.0 の時は、glGenerateMipmap しないと正しくミップマップ描画できない模様
            GLTextreImageSet& tex = img[0];
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
            glHint( GL_GENERATE_MIPMAP_HINT, GL_FASTEST );
            glGenerateMipmap( GL_TEXTURE_2D );
            // 自前で生成したものに一部置き換える
            for( GLint i = 1; i < count; i++ ) {
                GLTextreImageSet& tex = img[i];
                glTexSubImage2D( GL_TEXTURE_2D, i, 0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
            }
        } else {
            for( GLint i = 0; i < count; i++ ) {
                GLTextreImageSet& tex = img[i];
                glTexImage2D( GL_TEXTURE_2D, i, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits );
            }

        }
        glBindTexture( GL_TEXTURE_2D, 0 );
        format_ = tTVPTextureColorFormat::RGBA;
        glformat_ = GL_RGBA;
        width_ = w;
        height_ = h;
    }
}

void 
GLTexture::destory() 
{
    if( texture_id_ != 0 ) {
        glDeleteTextures( 1, &texture_id_ );
        texture_id_ = 0;
        hasMipmap_ = false;
    }
    if (pbo_) {
        glDeleteBuffers(1, &pbo_);
        pbo_ = 0;
    }
}


void 
GLTexture::UpdateTexture(GLuint tex_id, GLuint pbo, int format, int x, int y, int w, int h, std::function<void(char *dest, int pitch)> updator)
{
    int size = w*h*4;
    int pitch = w*4;

    if (pbo) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        GLubyte *texPixels = (GLubyte *)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, GL_MAP_WRITE_BIT);
        if (texPixels) {
            updator((char*)texPixels, pitch);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }

        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, format, GL_UNSIGNED_BYTE, 0);
        CheckGLErrorAndLog("glTexSubImage2D");
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    } else {

        std::unique_ptr<char[]> buffer(new char[size]);
        updator(&buffer[0], pitch);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, format, GL_UNSIGNED_BYTE, &buffer[0]);
        CheckGLErrorAndLog("glTexSubImage2D");
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

void 
GLTexture::UpdateTexture(int x, int y, int w, int h, std::function<void(char *dest, int pitch)> updator)
{
    if (w==0) w = width_;
    if (h==0) h = height_;
    UpdateTexture(texture_id_, pbo_, glformat_, x, y, w, h, updator);
}

//---------------------------------------------------------------------------
// テクスチャべた書き
//---------------------------------------------------------------------------

#include "GLShaderUtil.h"

static const char *vsSource = 
"attribute vec2 a_position;"
"attribute vec2 a_texCoord;"
"varying vec2 v_texCoord;"
"void main()"
"{"
"gl_Position = vec4( a_position, 0.0, 1.0 );"
"v_texCoord = a_texCoord;"
"}"
;

static const char *fsSource = 
"precision mediump float;"
"varying vec2 v_texCoord;"
"uniform sampler2D s_texture;"
"void main()"
"{"
"gl_FragColor = texture2D( s_texture, v_texCoord );"
"}"
;


GLTextureDrawer::GLTextureDrawer()
    : _shader_program(0)
    , _attr_position(0)
    , _attr_texCoord(0)
    , _unif_texture(0)
{
}

GLTextureDrawer::~GLTextureDrawer()
{
    Done();
}

void
GLTextureDrawer::Init()
{
    if (!_shader_program) {
        // べた書き用シェーダー
        _shader_program = CompileProgram(vsSource, fsSource);
        _attr_position  = glGetAttribLocation(_shader_program, "a_position");
        _attr_texCoord  = glGetAttribLocation(_shader_program, "a_texCoord");
        _unif_texture   = glGetUniformLocation(_shader_program, "s_texture");
    }
}

void
GLTextureDrawer::Done()
{
    if (_shader_program) {
        glDeleteProgram(_shader_program);
        _shader_program = 0;
    }
}

// 描画範囲にべた書き処理
void
GLTextureDrawer::DrawTexture(GLTexture *tex, int scr_w, int scr_h, float position[], int tex_w, int tex_h)
{
	if (_shader_program && tex) {

        glViewport(0, 0, scr_w, scr_h);

        GLfloat u = tex_w == 0 ? 1.0 : (float)tex_w / tex->width();
        GLfloat v = tex_h == 0 ? 1.0 : (float)tex_h / tex->height();

		// UV補正
    	GLfloat _uv[8];
		_uv[0] = 0;
		_uv[1] = v;
		_uv[2] = 0;
		_uv[3] = 0;
		_uv[4] = u;
		_uv[5] = v;
		_uv[6] = u;
		_uv[7] = 0;

        // 描画調整
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_STENCIL_TEST );
		glDisable( GL_SCISSOR_TEST );
		glDisable( GL_CULL_FACE );
		glDisable( GL_BLEND );

		// シェーダー設定
		glUseProgram(_shader_program);
		glEnableVertexAttribArray(_attr_position);
    	glEnableVertexAttribArray(_attr_texCoord);
		glUniform1i(_unif_texture, 0);

		// テクスチャをバインド
		glBindTexture(GL_TEXTURE_2D, tex->id());
		// パラメータ設定
        glVertexAttribPointer(_attr_position, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) position);
        glVertexAttribPointer(_attr_texCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) _uv);
		// 描画実行
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

bool GLTexture::_support_inited = false;
bool GLTexture::_support_bgra = false;
bool GLTexture::_support_swizzle = false;
bool GLTexture::_support_copy_image = false;

void 
GLTexture::InitSupported()
{
    if (!_support_inited) {

        bool ext_texture_bgra = false;
        bool ext_texture_storage = false;
        bool ext_copy_image = false;

        int NumberOfExtensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
        for (int i=0; i<NumberOfExtensions; i++) {
            const char *name = (const char*)glGetStringi(GL_EXTENSIONS, i);
			TVPLOG_DEBUG("OpenGL Extension:{}", name);
            if (strcmp(name, "GL_EXT_texture_format_BGRA8888") == 0 ||
                strcmp(name, "GL_APPLE_texture_format_BGRA8888") == 0) {
                ext_texture_bgra = true;
            } else if (strcmp(name, "GL_EXT_texture_storage") == 0) {
                ext_texture_storage = true;
            } else if (strcmp(name, "GL_EXT_copy_image") == 0) {
                ext_copy_image = true;
            }
        }
        _support_bgra = ext_texture_bgra && ext_texture_storage;
        _support_copy_image = ext_copy_image;

        GLint swizzleR;
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, &swizzleR);
        _support_swizzle = glGetError() == GL_NO_ERROR;
        _support_inited = true;

		if (_support_bgra) {
			TVPLOG_INFO("GLES: BGRA Texture Supported");
		}
		if (_support_swizzle) {
			TVPLOG_INFO("GLES: Texture Swizzle Supported");
		}
		if (_support_copy_image) {
			TVPLOG_INFO("GLES: Copy Image Extension Supported");
		}
    }
}

void 
GLTexture::copyFrom(const GLTexture& source)
{
    if (source.texture_id_ == 0) {
        return; // ソーステクスチャが無効
    }

    // 既存のテクスチャを削除
    destory();

    // ソーステクスチャのパラメータをコピー
    format_ = source.format_;
    glformat_ = source.glformat_;
    width_ = source.width_;
    height_ = source.height_;
    stretchType_ = source.stretchType_;
    wrapS_ = source.wrapS_;
    wrapT_ = source.wrapT_;
    hasMipmap_ = source.hasMipmap_;

    // 空のテクスチャを作成
    create(width_, height_, nullptr, format_);

    // GPU上でコピー実行
    copyTextureOnGPU(source);

    // パラメータを設定
    setStretchType(stretchType_);
    setWrapS(wrapS_);
    setWrapT(wrapT_);
}

void 
GLTexture::copyTextureOnGPU(const GLTexture& source)
{
    if (texture_id_ == 0 || source.texture_id_ == 0) {
        return;
    }

    // OpenGL ES 3.2 以上の場合は glCopyImageSubData を使用
    if (TVPOpenGLESVersion >= 320) {
        // 直接テクスチャ間でコピー
        glCopyImageSubData(source.texture_id_, GL_TEXTURE_2D, 0, 0, 0, 0,
                          texture_id_, GL_TEXTURE_2D, 0, 0, 0, 0,
                          width_, height_, 1);
        CheckGLErrorAndLog("glCopyImageSubData");
        return;
    }

    // GL_EXT_copy_image 拡張がある場合は glCopyImageSubDataEXT を使用
    if (SupportCopyImage()) {
        glCopyImageSubDataEXT(source.texture_id_, GL_TEXTURE_2D, 0, 0, 0, 0,
                             texture_id_, GL_TEXTURE_2D, 0, 0, 0, 0,
                             width_, height_, 1);
        CheckGLErrorAndLog("glCopyImageSubDataEXT");
        return;
    }

    // フォールバック: フレームバッファブリットを使用
    // 現在のフレームバッファを保存
    GLint currentFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

    // フレームバッファオブジェクトを作成
    GLuint srcFBO, dstFBO;
    glGenFramebuffers(1, &srcFBO);
    glGenFramebuffers(1, &dstFBO);

    // ソーステクスチャをリードフレームバッファにアタッチ
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, source.texture_id_, 0);

    // ターゲットテクスチャをドローフレームバッファにアタッチ
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id_, 0);

    // フレームバッファの完全性をチェック
    if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE &&
        glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        
        // GPU上でブリット（コピー）実行
        glBlitFramebuffer(0, 0, width_, height_, 
                         0, 0, width_, height_, 
                         GL_COLOR_BUFFER_BIT, GL_NEAREST);
        CheckGLErrorAndLog("glBlitFramebuffer");
    }

    // リソースをクリーンアップ
    glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
    glDeleteFramebuffers(1, &srcFBO);
    glDeleteFramebuffers(1, &dstFBO);
}

