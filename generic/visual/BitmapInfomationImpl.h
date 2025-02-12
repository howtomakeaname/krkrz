#ifndef __BITMAP_INFOMATION_IMPL_H__
#define __BITMAP_INFOMATION_IMPL_H__

#include "BitmapInfomation.h"

class BitmapInfomationImpl : public BitmapInfomation {

public:
	BitmapInfomationImpl( tjs_uint width, tjs_uint height, int bpp, bool unpadding=false );
	~BitmapInfomationImpl();

	virtual inline unsigned int GetBPP() const { return bpp_; }
	virtual inline int GetWidth() const { return width_; }
	virtual inline int GetHeight() const { return height_; }
	virtual inline bool GetUnpadding() const { return unpadding_; };
	virtual inline tjs_uint GetImageSize() const { return image_size_; }
	virtual BitmapInfomation& operator=(BitmapInfomation& r) {
		BitmapInfomationImpl *src = (BitmapInfomationImpl*)&r;
		bpp_ = r.GetBPP();
		width_ = r.GetWidth();
		height_ = r.GetHeight();
		unpadding_ = r.GetUnpadding();
		image_size_ = r.GetImageSize();
		bitmap_width_ = src->bitmap_width_;
		return *this;
	}

private:
	tjs_uint bpp_;
	tjs_int width_;
	tjs_int height_;
	tjs_uint image_size_;
	tjs_uint bitmap_width_;
	bool unpadding_;

};

#endif // __BITMAP_INFOMATION_H__

