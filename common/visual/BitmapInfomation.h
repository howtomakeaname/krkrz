#ifndef __BITMAP_INFOMATION_H__
#define __BITMAP_INFOMATION_H__

class BitmapInfomation {
public:
	virtual ~BitmapInfomation(){}
	virtual unsigned int GetBPP() const = 0;
	inline bool Is32bit() const { return GetBPP() == 32; }
	inline bool Is8bit() const { return GetBPP() == 8; }
	virtual inline int GetWidth() const = 0;
	virtual inline int GetHeight() const = 0;
	virtual inline bool GetUnpadding() const = 0;
	virtual tjs_uint GetImageSize() const = 0;
	inline int GetPitchBytes() const { return GetImageSize()/GetHeight(); }
	virtual BitmapInfomation& operator=(BitmapInfomation& r) = 0;
};

BitmapInfomation *TVPCreateBitmapInfo(tjs_uint width, tjs_uint height, int bpp, bool unpadding=false);

#endif // __BITMAP_INFOMATION_H__

