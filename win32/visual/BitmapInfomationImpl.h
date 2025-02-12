#ifndef __BITMAP_INFOMATION_IMPL_H__
#define __BITMAP_INFOMATION_IMPL_H__

#include "BitmapInfomation.h"

class BitmapInfomationImpl : public BitmapInfomation {
public:
	BitmapInfomationImpl( tjs_uint width, tjs_uint height, int bpp, bool unpadding=false );
	~BitmapInfomationImpl();

	virtual inline unsigned int GetBPP() const { return BitmapInfo->bmiHeader.biBitCount; }
	virtual inline int GetWidth() const { return BitmapInfo->bmiHeader.biWidth; }
	virtual inline int GetHeight() const { return BitmapInfo->bmiHeader.biHeight; }
	virtual inline bool GetUnpadding() const { return unpadding; };
	virtual inline tjs_uint GetImageSize() const { return BitmapInfo->bmiHeader.biSizeImage; }
	virtual BitmapInfomation& operator=(BitmapInfomation& r) {
		BitmapInfomationImpl *src = (BitmapInfomationImpl*)&r;
		memcpy(BitmapInfo, src->BitmapInfo, BitmapInfoSize);
		unpadding = r.GetUnpadding();
		return *this;
	}

	// 以下、Win32 のみのメソッド
	const BITMAPINFO* GetBITMAPINFO() const { return BitmapInfo; }
	const BITMAPINFOHEADER * GetBITMAPINFOHEADER() const { 
		return (const BITMAPINFOHEADER*)(BitmapInfo);
	}

private:
	BITMAPINFO* BitmapInfo;
	tjs_int BitmapInfoSize;
	bool unpadding;
};

#endif // __BITMAP_INFOMATION_IMPL_H__
