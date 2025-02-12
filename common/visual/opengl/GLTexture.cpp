#include "tjsCommHead.h"
#include "GLTexture.h"

#include <memory>

extern int TVPOpenGLESVersion;

void 
GLTexture::create( GLuint w, GLuint h, const GLvoid* bits, GLint format) 
{
    int pixel_size = format == GL_ALPHA ? 1 : 4;

    // internal format
    GLuint fmt;
    if (format == GL_RGBA) {
        fmt = GL_RGBA8;
    } else if (format == GL_BGRA_EXT) {
        fmt = GL_BGRA8_EXT;
    } else {
        fmt = GL_R8;
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

    if (format == GL_ALPHA) {
		glTexImage2D( GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, bits );
		glBindTexture( GL_TEXTURE_2D, 0 );
        return;
    }

    glTexStorage2D(GL_TEXTURE_2D, 1, fmt, w, h);
    CheckGLErrorAndLog(TJS_W("glTexStorage2D"));

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
            //glTexImage2D( GL_TEXTURE_2D, 0, fmt, w, h, 0, format, GL_UNSIGNED_BYTE, 0 );
            //CheckGLErrorAndLog(TJS_W("glTexImage2D"));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, 0);
            CheckGLErrorAndLog(TJS_W("glTexSubImage2D"));
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        } else {
            //glTexImage2D( GL_TEXTURE_2D, 0, fmt, w, h, 0, format, GL_UNSIGNED_BYTE, bits );
            //CheckGLErrorAndLog(TJS_W("glTexImage2D"));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, bits);
            CheckGLErrorAndLog(TJS_W("glTexSubImage2D"));
        }
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
        format_ = GL_RGBA;
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
        CheckGLErrorAndLog(TJS_W("glTexSubImage2D"));
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    } else {

        std::unique_ptr<char[]> buffer(new char[size]);
        updator(&buffer[0], pitch);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, format, GL_UNSIGNED_BYTE, &buffer[0]);
        CheckGLErrorAndLog(TJS_W("glTexSubImage2D"));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

void 
GLTexture::UpdateTexture(int x, int y, int w, int h, std::function<void(char *dest, int pitch)> updator)
{
    if (w==0) w = width_;
    if (h==0) h = height_;
    UpdateTexture(texture_id_, pbo_, format_, x, y, w, h, updator);
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


