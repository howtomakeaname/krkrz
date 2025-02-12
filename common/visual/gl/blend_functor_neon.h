#ifndef __BLEND_FUNCTOR_NEON_H__
#define __BLEND_FUNCTOR_NEON_H__

#include "neonutil.h"

extern "C"
{
    extern unsigned char TVPOpacityOnOpacityTable[256 * 256];
    extern unsigned char TVPNegativeMulTable[256 * 256];
};

// 端数処理にC実装版を参照する用
#include "blend_functor_c.h"

// ベーシックなアルファブレンド処理のベース
struct neon_alpha_blend : public alpha_blend_func
{
    // C言語版をそのまま呼び出す
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s, tjs_uint32 a) const
    {
        return alpha_blend_func::operator()(d, s, a);
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms, uint8x8_t ma) const
    {
        // ma_inv = (255 - ma)
        // (ms * ma + md * ma_inv) >> 8
        uint8x8_t ma_inv = vmvn_u8(ma);
        B_VEC(md) = vaddhn_u16(vmull_u8(B_VEC(ms), ma), vmull_u8(B_VEC(md), ma_inv));
        G_VEC(md) = vaddhn_u16(vmull_u8(G_VEC(ms), ma), vmull_u8(G_VEC(md), ma_inv));
        R_VEC(md) = vaddhn_u16(vmull_u8(R_VEC(ms), ma), vmull_u8(R_VEC(md), ma_inv));
        return md;
    }
};

// ソースのアルファを使う
template<typename blend_func>
struct neon_variation : public blend_func
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        tjs_uint32 a = (s >> 24);
        return blend_func::operator()(d, s, a);
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        return blend_func::operator()(md, ms, A_VEC(ms));
    }
};

// ソースのアルファとopacity値を使う
template<typename blend_func>
struct neon_variation_opa : public blend_func
{
    const tjs_int32 opa_;
    const uint8x8_t mopa;
    inline neon_variation_opa(tjs_int32 opa)
    : opa_(opa)
    , mopa(vdup_n_u8(opa & 0xff))
    {}

    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        // tjs_uint32 a = ((s>>24)*opa_) >> 8;
        tjs_uint32 a = (tjs_uint32)(((tjs_uint64)s * (tjs_uint64)opa_) >> 32);
        return blend_func::operator()(d, s, a);
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        uint16x8_t msa_mopa_16 = vmull_u8(A_VEC(ms), mopa);
        return blend_func::operator()(md, ms, vmovn_u16(vshrq_n_u16(msa_mopa_16, 8)));
    }
};

// ソースとデスティネーションのアルファを使う
struct neon_alpha_blend_d_functor : public neon_alpha_blend
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        tjs_uint32 addr      = ((s >> 16) & 0xff00) + (d >> 24);
        tjs_uint32 destalpha = TVPNegativeMulTable[addr] << 24;
        tjs_uint32 sopa      = TVPOpacityOnOpacityTable[addr];
        return neon_alpha_blend::operator()(d, s, sopa) + destalpha;
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        // ※基本的には neon_const_alpha_blend_d_functor と同じ処理なので
        // 修正時は合わせて対応のこと

        // テーブルを利用したコードを組む
        // 遅そうだが TVPAlphaBlend を TVPAlphaBlend_d に置き換えてのテストでは
        // c版が15msのところこのneon版で9.5msだったので、マシはマシみたい

        // src/dst のαをメモリにストア
        tjs_uint8 md_a[8], ms_a[8];
        vst1_u8(md_a, A_VEC(md));
        vst1_u8(ms_a, A_VEC(ms));

        // メモリから複数レーンにロードする命令はないので一旦リニアメモリに
        // 配置後、ロード命令でαベクタに読み込んだ上でブレンド処理へ渡す
        tjs_uint8 tmp_sopa[8];
        tjs_uint8 tmp_dopa[8];
        for (int i = 0; i < 8; i++) {
            tmp_sopa[i] = TVPOpacityOnOpacityTable[ms_a[i] << 8 | md_a[i]];
            tmp_dopa[i] = TVPNegativeMulTable[ms_a[i] << 8 | md_a[i]];
        }
        uint8x8x4_t ret = neon_alpha_blend::operator()(md, ms, vld1_u8(tmp_sopa));
        A_VEC(ret)      = vld1_u8(tmp_dopa);

        return ret;
    }
};

template<typename blend_func>
struct neon_variation_hda : public blend_func
{
    inline neon_variation_hda() {}
    inline neon_variation_hda(tjs_int32 opa)
    : blend_func(opa)
    {}

    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        tjs_uint32 dstalpha = d & 0xff000000;
        tjs_uint32 ret      = blend_func::operator()(d, s);
        return (ret & 0x00ffffff) | dstalpha;
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        uint8x8_t dstalpha = A_VEC(md);
        uint8x8x4_t ret    = blend_func::operator()(md, ms);
        A_VEC(ret)         = dstalpha;
        return ret;
    }
};

// もっともシンプルなコピー dst = src
struct neon_const_copy_functor
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const { return s; }
    inline uint8x8x4_t operator()(uint8x8x4_t md1, uint8x8x4_t ms1) const { return ms1; }
};

// 単純コピーだけど alpha をコピーしない(HDAと同じ)
struct neon_color_copy_functor
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        return (d & 0xff000000) + (s & 0x00ffffff);
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md1, uint8x8x4_t ms1) const
    {
        A_VEC(ms1) = A_VEC(md1);
        return ms1;
    }
};

// alphaだけコピーする : color_copy の src destを反転しただけ
struct neon_alpha_copy_functor : public neon_color_copy_functor
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        return neon_color_copy_functor::operator()(s, d);
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md1, uint8x8x4_t ms1) const
    {
        return neon_color_copy_functor::operator()(ms1, md1);
    }
};

// このままコピーするがアルファを0xffで埋める dst = 0xff000000 | src
struct neon_color_opaque_functor
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        return 0xff000000 | s;
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md1, uint8x8x4_t ms1) const
    {
        A_VEC(ms1) = vdup_n_u8(0xff);
        return ms1;
    }
};

// AVX2 版由来だが、元々未使用っぽい。一応ダミーのガワをおいておく
struct neon_alpha_blend_a_functor
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const { return s; }
    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const { return ms; }
};

typedef neon_variation<neon_alpha_blend> neon_alpha_blend_functor;
typedef neon_variation_opa<neon_alpha_blend> neon_alpha_blend_o_functor;
typedef neon_variation_hda<neon_variation<neon_alpha_blend>> neon_alpha_blend_hda_functor;
typedef neon_variation_hda<neon_variation_opa<neon_alpha_blend>>
    neon_alpha_blend_hda_o_functor;
// neon_alpha_blend_d_functor
// neon_alpha_blend_a_functor

struct neon_premul_alpha_blend_functor
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        // TODO ベクトル化不要？
        return s; // TODO 暫定
    }
    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        return ms; // TODO 暫定
    }

#if REFERENCE_AVX2
    const uint8x8x4_t zero_;
    inline neon_premul_alpha_blend_functor()
    : zero_(_mm256_setzero_si256())
    {}
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        __m128i ms     = _mm_cvtsi32_si128(s);
        tjs_int32 sopa = s >> 24;
        __m128i mo     = _mm_cvtsi32_si128(sopa);
        __m128i md     = _mm_cvtsi32_si128(d);
        mo             = _mm_shufflelo_epi16(
            mo, _MM_SHUFFLE(0, 0, 0, 0)); // 0000000000000000 00oo00oo00oo00oo
        md          = _mm_cvtepu8_epi16(md);          // 00dd00dd00dd00dd
        __m128i md2 = md;
        md          = _mm_mullo_epi16(md, mo);    // md * sopa
        md          = _mm_srli_epi16(md, 8);      // md >>= 8
        md2         = _mm_sub_epi16(md2, md);     // d - (d*sopa)>>8
        md2         = _mm_packus_epi16(md2, md2); // pack
        md2         = _mm_adds_epu8(md2, ms);     // d - ((d*sopa)>>8) + src
        return _mm_cvtsi128_si32(md2);
    }
    inline uint8x8x4_t operator()(uint8x8x4_t d, uint8x8x4_t s) const
    {
        uint8x8x4_t ma1 = s;
        ma1             = _mm256_srli_epi32(ma1, 24);      // s >> 24
        ma1             = _mm256_packs_epi32(ma1, ma1);    // 0 1 2 3 0 1 2 3
        ma1             = _mm256_unpacklo_epi16(ma1, ma1); // 0 0 1 1 2 2 3 3
        uint8x8x4_t ma2 = ma1;
        ma1             = _mm256_unpacklo_epi16(ma1, ma1); // 0 0 0 0 1 1 1 1

        uint8x8x4_t md2 = d;
        d               = _mm256_unpacklo_epi8(d, zero_);
        uint8x8x4_t md1 = d;
        d               = _mm256_mullo_epi16(d, ma1); // md * sopa
        d               = _mm256_srli_epi16(d, 8);    // md >>= 8
        md1             = _mm256_sub_epi16(md1, d);   // d - (d*sopa)>>8

        ma2              = _mm256_unpackhi_epi16(ma2, ma2); // 2 2 2 2 3 3 3 3
        md2              = _mm256_unpackhi_epi8(md2, zero_);
        uint8x8x4_t md2t = md2;
        md2              = _mm256_mullo_epi16(md2, ma2); // md * sopa
        md2              = _mm256_srli_epi16(md2, 8);    // md >>= 8
        md2t             = _mm256_sub_epi16(md2t, md2);  // d - (d*sopa)>>8

        md1 = _mm256_packus_epi16(md1, md2t);
        return _mm256_adds_epu8(md1, s); // d - ((d*sopa)>>8) + src
    }
#endif
};

//--------------------------------------------------------------------
// di = di - di*a*opa + si*opa
//              ~~~~~Df ~~~~~~ Sf
//           ~~~~~~~~Ds
//      ~~~~~~~~~~~~~Dq
// additive alpha blend with opacity
struct neon_premul_alpha_blend_o_functor
{
    const tjs_uint8 opa8_;
    inline neon_premul_alpha_blend_o_functor(tjs_int32 opa)
    : opa8_(opa & 0xff)
    {}

    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        // TODO ベクトル化不要？
        return s; // TODO 暫定
    }
    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        return ms; // TODO 暫定
    }

#if REFERENCE_AVX2

    const uint8x8x4_t zero_;
    const uint8x8x4_t opa_;
    inline neon_premul_alpha_blend_o_functor(tjs_int opa)
    : zero_(_mm256_setzero_si256())
    , opa_(_mm256_set1_epi16((short)opa))
    {}
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        const __m128i opa = _mm256_extracti128_si256(opa_, 0);
        __m128i ms        = _mm_cvtsi32_si128(s);
        __m128i md        = _mm_cvtsi32_si128(d);
        ms                = _mm_cvtepu8_epi16(ms);    // 00ss00ss00ss00ss
        md                = _mm_cvtepu8_epi16(md);    // 00dd00dd00dd00dd
        ms                = _mm_mullo_epi16(ms, opa); // 00Sf00Sf00Sf00Sf s * opa
        __m128i md2       = md;
        ms                = _mm_srli_epi16(ms, 8); // s >> 8
        __m128i ms2       = ms;
        ms2               = _mm_srli_epi64(ms2, 48); // s >> 48 | sopa
        ms2               = _mm_unpacklo_epi16(ms2, ms2);
        ms2               = _mm_unpacklo_epi16(ms2, ms2); // 00Df00Df00Df00Df

        md  = _mm_mullo_epi16(md, ms2); // 00Ds00Ds00Ds00Ds
        md  = _mm_srli_epi16(md, 8);    // d >> 8
        md2 = _mm_sub_epi16(md2, md);   // 00Dq00Dq00Dq00Dq
        md2 = _mm_add_epi16(md2, ms);   // d + s
        md2 = _mm_packus_epi16(md2, md2);
        return _mm_cvtsi128_si32(md2);
    }
    inline uint8x8x4_t operator()(uint8x8x4_t d, uint8x8x4_t s) const
    {
        uint8x8x4_t ms  = s;
        uint8x8x4_t md  = d;
        ms              = _mm256_unpackhi_epi8(ms, zero_);
        md              = _mm256_unpackhi_epi8(md, zero_);
        ms              = _mm256_mullo_epi16(ms, opa_); // 00Sf00Sf00Sf00Sf s * opa
        uint8x8x4_t md2 = md;
        ms              = _mm256_srli_epi16(ms, 8); // s >> 8
        uint8x8x4_t ma  = ms;
        ma              = _mm256_srli_epi64(ma, 48); // s >> 48 : sopa 0 0 0 1 0 0 0 2
        ma = _mm256_shuffle_epi32(ma, _MM_SHUFFLE(2, 2, 0, 0)); // 0 1 0 1 0 2 0 2
        ma = _mm256_packs_epi32(ma, ma);                        // 1 1 2 2 1 1 2 2
        ma = _mm256_unpacklo_epi32(ma, ma);                     // 1 1 1 1 2 2 2 2
        // 00Df00Df00Df00Df

        md  = _mm256_mullo_epi16(md, ma); // 00Ds00Ds00Ds00Ds
        md  = _mm256_srli_epi16(md, 8);   // d >> 8
        md2 = _mm256_sub_epi16(md2, md);  // 00Dq00Dq00Dq00Dq
        md2 = _mm256_add_epi16(md2, ms);  // d + s

        s               = _mm256_unpacklo_epi8(s, zero_);
        d               = _mm256_unpacklo_epi8(d, zero_);
        s               = _mm256_mullo_epi16(s, opa_); // 00Sf00Sf00Sf00Sf s * opa
        uint8x8x4_t md1 = d;
        s               = _mm256_srli_epi16(s, 8); // s >> 8
        ma              = s;
        ma              = _mm256_srli_epi64(ma, 48);            // s >> 48 | sopa
        ma = _mm256_shuffle_epi32(ma, _MM_SHUFFLE(2, 2, 0, 0)); // 0 1 0 1 0 2 0 2
        ma = _mm256_packs_epi32(ma, ma);                        // 1 1 2 2 1 1 2 2
        ma = _mm256_unpacklo_epi32(ma, ma);                     // 1 1 1 1 2 2 2 2
        // 00Df00Df00Df00Df

        d   = _mm256_mullo_epi16(d, ma); // 00Ds00Ds00Ds00Ds
        d   = _mm256_srli_epi16(d, 8);   // d >> 8
        md1 = _mm256_sub_epi16(md1, d);  // 00Dq00Dq00Dq00Dq
        md1 = _mm256_add_epi16(md1, s);  // d + s
        return _mm256_packus_epi16(md1, md2);
    }
#endif
};

/*
        Di = sat(Si, (1-Sa)*Di)
        Da = Sa + Da - SaDa
*/
// additive alpha blend holding destination alpha
struct neon_premul_alpha_blend_hda_functor
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        // TODO ベクトル化不要？
        return s; // TODO 暫定
    }
    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        return ms; // TODO 暫定
    }
#if REFERENCE_AVX2
    const uint8x8x4_t zero_;
    const uint8x8x4_t alphamask_;
    const uint8x8x4_t colormask_;
    inline neon_premul_alpha_blend_hda_functor()
    : zero_(_mm256_setzero_si256())
    , alphamask_(_mm256_set_epi32(0x0000ffff, 0xffffffff, 0x0000ffff, 0xffffffff,
                                  0x0000ffff, 0xffffffff, 0x0000ffff, 0xffffffff))
    , colormask_(_mm256_set1_epi32(0x00FFFFFF))
    {}
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        const __m128i alphamask = _mm256_extracti128_si256(alphamask_, 0);
        const __m128i colormask = _mm256_extracti128_si256(colormask_, 0);
        __m128i ms              = _mm_cvtsi32_si128(s);
        __m128i mo              = _mm_cvtsi32_si128(s >> 24);
        __m128i md              = _mm_cvtsi32_si128(d);
        ms                      = _mm_and_si128(ms, colormask); // 0000000000ssssss
        md                      = _mm_cvtepu8_epi16(md);        // 00dd00dd00dd00dd
        mo = _mm_shufflelo_epi16(mo, _MM_SHUFFLE(0, 0, 0, 0));  // 00oo00oo00oo00oo

        __m128i md2 = md;
        md          = _mm_mullo_epi16(md, mo);      // d * opa
        md          = _mm_srli_epi16(md, 8);        // d >> 8
        md          = _mm_and_si128(md, alphamask); // d & 0x00ffffff
        md2         = _mm_sub_epi16(md2, md);       // d - d*opa
        md2         = _mm_packus_epi16(md2, md2);
        md2         = _mm_adds_epu8(md2, ms); // d + src
        return _mm_cvtsi128_si32(md2);
    }
    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t s) const
    {
        uint8x8x4_t mo0 = s;
        mo0             = _mm256_srli_epi32(mo0, 24);
        mo0             = _mm256_packs_epi32(mo0, mo0);    // 0 1 2 3 0 1 2 3
        mo0             = _mm256_unpacklo_epi16(mo0, mo0); // 0 0 1 1 2 2 3 3
        uint8x8x4_t mo1 = mo0;
        mo1             = _mm256_unpacklo_epi16(mo1, mo1); // 0 0 0 0 1 1 1 1 o[1]
        mo0             = _mm256_unpackhi_epi16(mo0, mo0); // 2 2 2 2 3 3 3 3 o[0]

        uint8x8x4_t md0  = md;
        md               = _mm256_unpacklo_epi8(md, zero_); // 00dd00dd00dd00dd d[1]
        uint8x8x4_t md12 = md;
        md               = _mm256_mullo_epi16(md, mo1);      // d[1] * o[1]
        md               = _mm256_srli_epi16(md, 8);         // d[1] >> 8
        md               = _mm256_and_si256(md, alphamask_); // d[1] & 0x00ffffff
        md12             = _mm256_sub_epi16(md12, md);       // d[1] - d[1]*opa

        md0              = _mm256_unpackhi_epi8(md0, zero_); // 00dd00dd00dd00dd d[0]
        uint8x8x4_t md02 = md0;
        md0              = _mm256_mullo_epi16(md0, mo0);      // d[0] * o[0]
        md0              = _mm256_srli_epi16(md0, 8);         // d[0] >> 8
        md0              = _mm256_and_si256(md0, alphamask_); // d[0] & 0x00ffffff
        md02             = _mm256_sub_epi16(md02, md0);       // d[0] - d[0]*opa
        md02             = _mm256_packus_epi16(md12, md02);   // pack( d[1], d[0] )

        s = _mm256_and_si256(s, colormask_); // s & 0x00ffffff00ffffff
        return _mm256_adds_epu8(md02, s);    // d + s
    }
#endif
};

// additive alpha blend on additive alpha
struct neon_premul_alpha_blend_a_functor
{
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        // TODO ベクトル化不要？
        return s; // TODO 暫定
    }
    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        return ms; // TODO 暫定
    }
#if REFERENCE_AVX2

    const uint8x8x4_t zero_;
    inline neon_premul_alpha_blend_a_functor()
    : zero_(_mm256_setzero_si256())
    {}
    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        __m128i ms = _mm_cvtsi32_si128(s);
        __m128i mo = ms;
        mo         = _mm_srli_epi64(mo, 24);                           // sopa
        mo         = _mm_shufflelo_epi16(mo, _MM_SHUFFLE(0, 0, 0, 0)); // 00Sa00Sa00Sa00Sa
        ms         = _mm_cvtepu8_epi16(ms);                            // 00Sa00Si00Si00Si
        __m128i md = _mm_cvtsi32_si128(d);
        md         = _mm_cvtepu8_epi16(md); // 00Da00Di00Di00Di
        __m128i md2 = md;
        md2         = _mm_mullo_epi16(md2, mo); // d * sopa
        md2         = _mm_srli_epi16(md2, 8);   // 00 SaDa 00 SaDi 00 SaDi 00 SaDi
        md          = _mm_sub_epi16(md, md2);   // d - d*sopa
        md          = _mm_add_epi16(md, ms);    // (d-d*sopa) + s
        md          = _mm_packus_epi16(md, md);
        return _mm_cvtsi128_si32(md);
    }
    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        uint8x8x4_t mo0 = ms;
        mo0             = _mm256_srli_epi32(mo0, 24);
        mo0             = _mm256_packs_epi32(mo0, mo0);    // 0 1 2 3 0 1 2 3
        mo0             = _mm256_unpacklo_epi16(mo0, mo0); // 0 0 1 1 2 2 3 3
        uint8x8x4_t mo1 = mo0;
        mo1             = _mm256_unpacklo_epi16(mo1, mo1); // 0 0 0 0 1 1 1 1 o[1]
        mo0             = _mm256_unpackhi_epi16(mo0, mo0); // 2 2 2 2 3 3 3 3 o[0]

        uint8x8x4_t md1  = md;
        uint8x8x4_t ms1  = ms;
        md               = _mm256_unpackhi_epi8(md, zero_); // 00dd00dd00dd00dd d[0]
        uint8x8x4_t md02 = md;
        ms               = _mm256_unpackhi_epi8(ms, zero_);
        md02             = _mm256_mullo_epi16(md02, mo0); // d * sopa | d[0]
        md02 = _mm256_srli_epi16(md02, 8); // 00 SaDa 00 SaDi 00 SaDi 00 SaDi | d[0]
        md   = _mm256_sub_epi16(md, md02); // d - d*sopa | d[0]
        md   = _mm256_add_epi16(md, ms);   // d - d*sopa + s | d[0]

        md1              = _mm256_unpacklo_epi8(md1, zero_); // 00dd00dd00dd00dd d[1]
        uint8x8x4_t md12 = md1;
        ms1              = _mm256_unpacklo_epi8(ms1, zero_);
        md12             = _mm256_mullo_epi16(md12, mo1); // d * sopa | d[1]
        md12 = _mm256_srli_epi16(md12, 8);  // 00 SaDa 00 SaDi 00 SaDi 00 SaDi | d[1]
        md1  = _mm256_sub_epi16(md1, md12); // d - d*sopa | d[1]
        md1  = _mm256_add_epi16(md1, ms1);  // d - d*sopa + s | d[1]

        return _mm256_packus_epi16(md1, md);
    }
#endif
};

// opacity値を使う
struct neon_const_alpha_blend_functor : public neon_alpha_blend
{
    const tjs_int32 opa_;
    const uint8x8_t ma;
    inline neon_const_alpha_blend_functor(tjs_int32 opa)
    : opa_(opa)
    , ma(vdup_n_u8(opa & 0xff))
    {}

    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        return neon_alpha_blend::operator()(d, s, opa_);
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        return neon_alpha_blend::operator()(md, ms, ma);
    }
};
typedef neon_variation_hda<neon_const_alpha_blend_functor>
    neon_const_alpha_blend_hda_functor;

struct neon_const_alpha_blend_d_functor : public neon_alpha_blend
{
    const tjs_uint32 opa_;
    inline neon_const_alpha_blend_d_functor(tjs_int32 opa)
    : opa_((opa & 0xff) << 8)
    {}

    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        tjs_uint32 addr      = opa_ + (d >> 24);
        tjs_uint32 destalpha = TVPNegativeMulTable[addr] << 24;
        tjs_uint32 sopa      = TVPOpacityOnOpacityTable[addr];
        return neon_alpha_blend::operator()(d, s, sopa) + destalpha;
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md, uint8x8x4_t ms) const
    {
        // ※基本的には neon_alpha_blend_d_functor と同じ処理なので
        // 修正時は合わせて対応のこと

        // dst のαをメモリにストア
        tjs_uint8 md_a[8];
        vst1_u8(md_a, A_VEC(md));

        // メモリから複数レーンにロードする命令はないので一旦リニアメモリに
        // 配置後、ロード命令でαベクタに読み込んだ上でブレンド処理へ渡す
        tjs_uint8 tmp_sopa[8];
        tjs_uint8 tmp_dopa[8];
        for (int i = 0; i < 8; i++) {
            tmp_sopa[i] = TVPOpacityOnOpacityTable[opa_ | md_a[i]];
            tmp_dopa[i] = TVPNegativeMulTable[opa_ | md_a[i]];
        }
        uint8x8x4_t ret = neon_alpha_blend::operator()(md, ms, vld1_u8(tmp_sopa));
        A_VEC(ret)      = vld1_u8(tmp_dopa);

        return ret;
    }
};

struct neon_const_alpha_blend_a_functor
{
    const tjs_uint32 opa32_;
    const tjs_uint8 opa8_;
    const struct neon_premul_alpha_blend_a_functor blend_;
    inline neon_const_alpha_blend_a_functor(tjs_int32 opa)
    : opa32_(opa << 24)
    , opa8_(opa & 0xff)
    {}

    inline tjs_uint32 operator()(tjs_uint32 d, tjs_uint32 s) const
    {
        return blend_(d, (s & 0x00ffffff) | opa32_);
    }

    inline uint8x8x4_t operator()(uint8x8x4_t md1, uint8x8x4_t ms1) const
    {
        A_VEC(ms1) = vdup_n_u8(opa8_);
        return blend_(md1, ms1);
    }
};

// neon_const_alpha_blend_functor;
typedef neon_const_alpha_blend_functor neon_const_alpha_blend_sd_functor;

// ※以下AVX2コードのコメントそのまま
//
// tjs_uint32 neon_const_alpha_blend_functor::operator()( tjs_uint32 d, tjs_uint32 s )
// tjs_uint32 neon_const_alpha_blend_sd_functor::operator()( tjs_uint32 s1, tjs_uint32 s2
// ) と引数は異なるが、処理内容は同じ const_alpha_blend は、dest と src1
// を共有しているようなもの dest = dest * src const_alpha_blend_sd は、dest = src1 * src2

// neon_const_copy_functor = TVPCopy はない、memcpy になってる
// neon_color_copy_functor = TVPCopyColor / TVPLinTransColorCopy
// neon_alpha_copy_functor = TVPCopyMask
// neon_color_opaque_functor = TVPCopyOpaqueImage
// neon_const_alpha_blend_functor = TVPConstAlphaBlend
// neon_const_alpha_blend_hda_functor = TVPConstAlphaBlend_HDA
// neon_const_alpha_blend_d_functor = TVPConstAlphaBlend_a
// neon_const_alpha_blend_a_functor = TVPConstAlphaBlend_a

//--------------------------------------------------------------------
// ここまでアルファブレンド
// 加算合成などはNEONでは未対応
//--------------------------------------------------------------------

#endif // __BLEND_FUNCTOR_NEON_H__
