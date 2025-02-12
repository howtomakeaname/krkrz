#ifndef _MOVIE_PLAYER_H__
#define _MOVIE_PLAYER_H__

/**
 * レイヤ上で動画再生するための汎用インターフェース
*/
class iTVPMoviePlayer {

public:

  struct AudioFormat
  {
	tjs_uint32 Channels;		// チャンネル数
	tjs_uint32 SampleRate;		// サンプリングレート
	tjs_uint32 BitsPerSample;	// サンプル当たりのビット数
	TVPAudioSampleType SampleType;	// サンプルの形式
  };

  // --------------------------------------------------------------------

  virtual void Play(bool loop = false) = 0;
  virtual void Stop() = 0;
  virtual void Pause()  = 0;
  virtual void Resume()  = 0;
  virtual void Seek(int64_t posUs)  = 0;
  virtual void SetLoop(bool loop)  = 0;

  virtual int32_t Width() const  = 0;
  virtual int32_t Height() const  = 0;
  virtual int64_t Duration() const  = 0;
  virtual int64_t Position() const  = 0;
  virtual bool IsPlaying() const  = 0;
  virtual bool Loop() const  = 0;

  // --------------------------------------------------------------------

  // get RGBA frame
  virtual bool GetVideoFrame(uint8_t *dst, int32_t w, int32_t h, int32_t strideBytes) = 0;

  // audio info
  virtual bool IsAudioAvailable() const                  = 0;
  virtual void GetAudioFormat(AudioFormat *format) const = 0;

  // オーディオデコーダコールバック
  typedef void (*OnAudioDecoded)(void *userPtr, const uint8_t *data, size_t sizeBytes);

  // 出力オーディオ通知関数を登録
  virtual void SetOnAudioDecoded(OnAudioDecoded func, void *userPtr) = 0;
};

extern iTVPMoviePlayer*TVPCreateMoviePlayer(const tjs_char *filename);

#endif
