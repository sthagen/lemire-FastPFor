/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire
 */

/**
 * Implementation of
 *
 * Willhalm T, Popovici N, Boshmaf Y, Plattner H, Zeier A, Schaffner J.
 * SIMD-scan: ultra fast in-memory table scan using on-chip vector processing
 * units.
 * Proceedings of the VLDB Endowment Aug 2009; 2(1):385-394.
 *
 * Optimized for a recent Intel core i7 processor by D. Lemire on Oct. 2012.
 */

#include "horizontalbitpacking.h"

namespace FastPForLib {

static void SIMD_nullunpacker32(const uint8_t *, uint32_t *out) {
  memset(out, 0, 32 * 4 * 4);
}

// Did not get good results with this:
/*


static void simdhunpack1(const uint8_t *  in,uint32_t *  out) {
    __m128i* pCurr = reinterpret_cast<__m128i*>(out);
    const static __m128i andmask = _mm_set_epi8
((char)-128,(char)(1<<6),(char)(1<<5),(char)(1<<4),(char)(1<<3),(char)(1<<2),(char)(1<<1),(char)(1<<0),(char)-128,(char)(1<<6),(char)(1<<5),(char)(1<<4),(char)(1<<3),(char)(1<<2),(char)(1<<1),(char)(1<<0));
    const static __m128i shufmask = _mm_set_epi8
((char)1,(char)1,(char)1,(char)1,(char)1,(char)1,(char)1,(char)1,(char)0,(char)0,(char)0,(char)0,(char)0,(char)0,(char)0,(char)0);
    const static __m128i allones = _mm_set1_epi8 ((char)1);
    const static __m128i finalshufmask1 = _mm_set_epi8
((char)-128,(char)-128,(char)-128,(char)3,(char)-128,(char)-128,(char)-128,(char)2,(char)-128,(char)-128,(char)-128,(char)1,(char)-128,(char)-128,(char)-128,(char)0);
    const static __m128i finalshufmask2 = _mm_set_epi8
((char)-128,(char)-128,(char)-128,(char)7,(char)-128,(char)-128,(char)-128,(char)6,(char)-128,(char)-128,(char)-128,(char)5,(char)-128,(char)-128,(char)-128,(char)4);
    const static __m128i finalshufmask3 = _mm_set_epi8
((char)-128,(char)-128,(char)-128,(char)11,(char)-128,(char)-128,(char)-128,(char)10,(char)-128,(char)-128,(char)-128,(char)9,(char)-128,(char)-128,(char)-128,(char)8);
    const static __m128i finalshufmask4 = _mm_set_epi8
((char)-128,(char)-128,(char)-128,(char)15,(char)-128,(char)-128,(char)-128,(char)14,(char)-128,(char)-128,(char)-128,(char)13,(char)-128,(char)-128,(char)-128,(char)12);
    for (uint32_t j = 0; j<8;++j) {
        __m128i ba = _mm_loadu_si128(reinterpret_cast<const __m128i*>(in + 2 *
j));
        __m128i ca = _mm_shuffle_epi8(ba, shufmask);
        ca=_mm_and_si128(ca,andmask);
        ca = _mm_cmpeq_epi8(ca,andmask);
        ca = _mm_and_si128(ca,allones);
        _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,finalshufmask1));
        //ca=_mm_srli_si128 (ca, 4);
        //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,finalshufmask1));
        //ca=_mm_srli_si128 (ca, 4);
        //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,finalshufmask1));
        //ca=_mm_srli_si128 (ca, 4);
        //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,finalshufmask1));
         _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,finalshufmask2));
         _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,finalshufmask3));
         _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,finalshufmask4));
    }
}


 */
const static __m128i shufkey1_1 = {(long long)-9187202500191551488, (long long)-9187202500191551488};
const static __m128i shufkey2_1 = {(long long)-9187202500191551488, (long long)-9187202500191551488};
const static __m128i multi1_1 = {(long long)17179869192, (long long)4294967298};
const static __m128i multi2_1 = {(long long)17179869192, (long long)4294967298};
const static __m128i mask_1 = {(long long)4294967297, (long long)4294967297};

static void simdhunpack1(const uint8_t *in, uint32_t *out) {
  enum { b = 1 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 3 };
  enum { shift2 = 7 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_1);
    ca = _mm_mullo_epi32(ca, multi1_1);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_1);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_1);
    ca = _mm_mullo_epi32(ca, multi2_1);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_1);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_2 = {(long long)-9187202500191551488, (long long)-9187202500191551488};
const static __m128i shufkey2_2 = {(long long)-9187202500191551488, (long long)-9187202500191551488};
const static __m128i multi1_2 = {(long long)68719476800, (long long)4294967300};
const static __m128i multi2_2 = {(long long)68719476800, (long long)4294967300};
const static __m128i mask_2 = {(long long)12884901891, (long long)12884901891};

static void simdhunpack2(const uint8_t *in, uint32_t *out) {
  enum { b = 2 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_2);
    ca = _mm_mullo_epi32(ca, multi1_2);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_2);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_2);
    ca = _mm_mullo_epi32(ca, multi2_2);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_2);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_3 = {(long long)-9187202500191551488, (long long)-9187202495896616704};
const static __m128i shufkey2_3 = {(long long)-9187342138168279040, (long long)-9187202495896584191};
const static __m128i multi1_3 = {(long long)34359738432, (long long)137438953473};
const static __m128i multi2_3 = {(long long)4294967304, (long long)17179869216};
const static __m128i mask_3 = {(long long)30064771079, (long long)30064771079};

static void simdhunpack3(const uint8_t *in, uint32_t *out) {
  enum { b = 3 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 7 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_3);
    ca = _mm_mullo_epi32(ca, multi1_3);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_3);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_3);
    ca = _mm_mullo_epi32(ca, multi2_3);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_3);
    _mm_storeu_si128(pCurr++, ca);
  }
}
// Did not get good results with this:
/*


static  void simdhunpack4(const uint8_t *  in,uint32_t *  out) {

    enum{ b = 4};
    __m128i* pCurr = reinterpret_cast<__m128i*>(out);
    const static __m128i shuf8 =  _mm_set_epi8 ((char)13,(char)12, (char)15,(char)14, (char)9,(char)8, (char)11,(char)10, (char)5,(char)4,
(char)7,(char)6, (char)1,(char)0, (char)3,(char)2);
    const static __m128i shufoneoutoftwo =  _mm_set1_epi32 (252645135);
    const static __m128i shufmid8_1 =  _mm_set_epi8 ((char)-128, (char)-128, (char)-128, (char)3, (char)-128,
(char)-128, (char)-128, (char)1, (char)-128, (char)-128, (char)-128, (char)2, (char)-128, (char)-128, (char)-128, (char)0);
    const static __m128i shufmid8_2 =  _mm_set_epi8 ((char)-128, (char)-128, (char)-128, (char)7, (char)-128,
(char)-128, (char)-128, (char)5, (char)-128, (char)-128, (char)-128, (char)6, (char)-128, (char)-128, (char)-128, (char)4);
    const static __m128i shufmid8_3 =  _mm_set_epi8 ((char)-128, (char)-128, (char)-128, (char)11, (char)-128,
(char)-128, (char)-128, (char)9, (char)-128, (char)-128, (char)-128, (char)10, (char)-128, (char)-128, (char)-128, (char)8);
    const static __m128i shufmid8_4 =  _mm_set_epi8 ((char)-128, (char)-128, (char)-128, (char)15, (char)-128,
(char)-128, (char)-128, (char)13, (char)-128, (char)-128, (char)-128, (char)14, (char)-128, (char)-128, (char)-128, (char)12);
    for (uint32_t j = 0; j<4;++j) {
        const __m128i ba = _mm_loadu_si128(reinterpret_cast<const __m128i*>(in +
16 * j));
        __m128i ca = _mm_srli_epi16(ba,4);
        ca = _mm_shuffle_epi8(ca,shuf8);
        ca = _mm_blend_epi16 (ba, ca,170);
        ca =  _mm_and_si128(ca,shufoneoutoftwo);
       const __m128i altba = _mm_shuffle_epi8(ba,shuf8);
       __m128i altca = _mm_srli_epi16(altba,4);
       altca = _mm_shuffle_epi8(altca,shuf8);
       altca = _mm_blend_epi16 (altba, altca,170);
       altca =  _mm_and_si128(altca,shufoneoutoftwo);
       _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,shufmid8_1) );
       _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(altca,shufmid8_1) );
       //ca=_mm_srli_si128 (ca, 4);
       //altca=_mm_srli_si128 (altca, 4);
       //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,shufmid8_1) );
       //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(altca,shufmid8_1) );
       //ca=_mm_srli_si128 (ca, 4);
       //altca=_mm_srli_si128 (altca, 4);
       //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,shufmid8_1) );
       //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(altca,shufmid8_1) );
       //ca=_mm_srli_si128 (ca, 4);
       //altca=_mm_srli_si128 (altca, 4);
       //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,shufmid8_1) );
       //_mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(altca,shufmid8_1) );
       _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,shufmid8_2) );
       _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(altca,shufmid8_2) );
       _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,shufmid8_3) );
       _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(altca,shufmid8_3) );
       _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(ca,shufmid8_4) );
       _mm_storeu_si128(pCurr++ , _mm_shuffle_epi8(altca,shufmid8_4) );
    }
}


 */
const static __m128i shufkey1_4 = {(long long)-9187202500191551488, (long long)-9187202495896584191};
const static __m128i shufkey2_4 = {(long long)-9187202500191551488, (long long)-9187202495896584191};
const static __m128i multi1_4 = {(long long)4294967312, (long long)4294967312};
const static __m128i multi2_4 = {(long long)4294967312, (long long)4294967312};
const static __m128i mask_4 = {(long long)64424509455, (long long)64424509455};

static void simdhunpack4(const uint8_t *in, uint32_t *out) {
  enum { b = 4 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 4 };
  enum { shift2 = 4 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_4);
    ca = _mm_mullo_epi32(ca, multi1_4);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_4);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_4);
    ca = _mm_mullo_epi32(ca, multi2_4);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_4);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_5 = {(long long)-9187342138168279040, (long long)-9187341034361683967};
const static __m128i shufkey2_5 = {(long long)-9187202495896616704, (long long)-9187202491601649151};
const static __m128i multi1_5 = {(long long)17179869312, (long long)4294967328};
const static __m128i multi2_5 = {(long long)137438953476, (long long)34359738369};
const static __m128i mask_5 = {(long long)133143986207, (long long)133143986207};

static void simdhunpack5(const uint8_t *in, uint32_t *out) {
  enum { b = 5 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 7 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_5);
    ca = _mm_mullo_epi32(ca, multi1_5);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_5);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_5);
    ca = _mm_mullo_epi32(ca, multi2_5);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_5);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_6 = {(long long)-9187342138168279040, (long long)-9187202491601649151};
const static __m128i shufkey2_6 = {(long long)-9187342138168279040, (long long)-9187202491601649151};
const static __m128i multi1_6 = {(long long)4294967360, (long long)68719476740};
const static __m128i multi2_6 = {(long long)4294967360, (long long)68719476740};
const static __m128i mask_6 = {(long long)270582939711, (long long)270582939711};

static void simdhunpack6(const uint8_t *in, uint32_t *out) {
  enum { b = 6 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_6);
    ca = _mm_mullo_epi32(ca, multi1_6);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_6);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_6);
    ca = _mm_mullo_epi32(ca, multi2_6);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_6);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_7 = {(long long)-9187342138168279040, (long long)-9187339930555121151};
const static __m128i shufkey2_7 = {(long long)-9187341034361716480, (long long)-9187202487306681598};
const static __m128i multi1_7 = {(long long)4294967424, (long long)17179869186};
const static __m128i multi2_7 = {(long long)8589934593, (long long)34359738372};
const static __m128i mask_7 = {(long long)545460846719, (long long)545460846719};

static void simdhunpack7(const uint8_t *in, uint32_t *out) {
  enum { b = 7 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 7 };
  enum { shift2 = 4 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_7);
    ca = _mm_mullo_epi32(ca, multi1_7);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_7);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_7);
    ca = _mm_mullo_epi32(ca, multi2_7);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_7);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_8 = {(long long)-9187202495896584192, (long long)-9187202487306649598};
const static __m128i shufkey2_8 = {(long long)-9187202495896584192, (long long)-9187202487306649598};

static void simdhunpack8(const uint8_t *in, uint32_t *out) {
  enum { b = 8 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_8);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_8);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_9 = {(long long)-9187341034361716480, (long long)-9187338826748525822};
const static __m128i shufkey2_9 = {(long long)-9187341034361716480, (long long)-9187338826748525822};
const static __m128i multi1_9 = {(long long)17179869192, (long long)4294967298};
const static __m128i multi2_9 = {(long long)17179869192, (long long)4294967298};
const static __m128i mask_9 = {(long long)2194728288767, (long long)2194728288767};

static void simdhunpack9(const uint8_t *in, uint32_t *out) {
  enum { b = 9 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 3 };
  enum { shift2 = 7 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_9);
    ca = _mm_mullo_epi32(ca, multi1_9);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_9);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_9);
    ca = _mm_mullo_epi32(ca, multi2_9);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_9);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_10 = {(long long)-9187341034361716480, (long long)-9187338826748525822};
const static __m128i shufkey2_10 = {(long long)-9187341034361716480, (long long)-9187338826748525822};
const static __m128i multi1_10 = {(long long)68719476800, (long long)4294967300};
const static __m128i multi2_10 = {(long long)68719476800, (long long)4294967300};
const static __m128i mask_10 = {(long long)4393751544831, (long long)4393751544831};

static void simdhunpack10(const uint8_t *in, uint32_t *out) {
  enum { b = 10 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_10);
    ca = _mm_mullo_epi32(ca, multi1_10);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_10);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_10);
    ca = _mm_mullo_epi32(ca, multi2_10);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_10);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_11 = {(long long)-9187341034361716480, (long long)-9187337722950057214};
const static __m128i shufkey2_11 = {(long long)-9222525406450548480, (long long)-9187337722941930493};
const static __m128i multi1_11 = {(long long)34359738432, (long long)137438953473};
const static __m128i multi2_11 = {(long long)4294967304, (long long)17179869216};
const static __m128i mask_11 = {(long long)8791798056959, (long long)8791798056959};

static void simdhunpack11(const uint8_t *in, uint32_t *out) {
  enum { b = 11 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 7 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_11);
    ca = _mm_mullo_epi32(ca, multi1_11);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_11);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_11);
    ca = _mm_mullo_epi32(ca, multi2_11);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_11);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_12 = {(long long)-9187341034361716480, (long long)-9187337722941930493};
const static __m128i shufkey2_12 = {(long long)-9187341034361716480, (long long)-9187337722941930493};
const static __m128i multi1_12 = {(long long)4294967312, (long long)4294967312};
const static __m128i multi2_12 = {(long long)4294967312, (long long)4294967312};
const static __m128i mask_12 = {(long long)17587891081215, (long long)17587891081215};

static void simdhunpack12(const uint8_t *in, uint32_t *out) {
  enum { b = 12 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 4 };
  enum { shift2 = 4 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_12);
    ca = _mm_mullo_epi32(ca, multi1_12);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_12);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_12);
    ca = _mm_mullo_epi32(ca, multi2_12);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_12);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_13 = {(long long)-9222525406450548480, (long long)-9221677670100630525};
const static __m128i shufkey2_13 = {(long long)-9187339930563378944, (long long)-9187336619143396349};
const static __m128i multi1_13 = {(long long)17179869312, (long long)4294967328};
const static __m128i multi2_13 = {(long long)137438953476, (long long)34359738369};
const static __m128i mask_13 = {(long long)35180077129727, (long long)35180077129727};

static void simdhunpack13(const uint8_t *in, uint32_t *out) {
  enum { b = 13 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 7 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_13);
    ca = _mm_mullo_epi32(ca, multi1_13);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_13);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_13);
    ca = _mm_mullo_epi32(ca, multi2_13);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_13);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_14 = {(long long)-9222525406450548480, (long long)-9187336619143396349};
const static __m128i shufkey2_14 = {(long long)-9222525406450548480, (long long)-9187336619143396349};
const static __m128i multi1_14 = {(long long)4294967360, (long long)68719476740};
const static __m128i multi2_14 = {(long long)4294967360, (long long)68719476740};
const static __m128i mask_14 = {(long long)70364449226751, (long long)70364449226751};

static void simdhunpack14(const uint8_t *in, uint32_t *out) {
  enum { b = 14 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_14);
    ca = _mm_mullo_epi32(ca, multi1_14);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_14);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_14);
    ca = _mm_mullo_epi32(ca, multi2_14);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_14);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_15 = {(long long)-9222525406450548480, (long long)-9221395091325385725};
const static __m128i shufkey2_15 = {(long long)-9222242827675500288, (long long)-9187335515336735484};
const static __m128i multi1_15 = {(long long)4294967424, (long long)17179869186};
const static __m128i multi2_15 = {(long long)8589934593, (long long)34359738372};
const static __m128i mask_15 = {(long long)140733193420799, (long long)140733193420799};

static void simdhunpack15(const uint8_t *in, uint32_t *out) {
  enum { b = 15 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 7 };
  enum { shift2 = 4 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_15);
    ca = _mm_mullo_epi32(ca, multi1_15);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_15);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_15);
    ca = _mm_mullo_epi32(ca, multi2_15);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_15);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_16 = {(long long)-9187339930555121408, (long long)-9187335515328740092};
const static __m128i shufkey2_16 = {(long long)-9187339930555121408, (long long)-9187335515328740092};

static void simdhunpack16(const uint8_t *in, uint32_t *out) {
  enum { b = 16 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_16);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_16);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_17 = {(long long)-9222242827675500288, (long long)-9221112512542014204};
const static __m128i shufkey2_17 = {(long long)-9222242827675500288, (long long)-9221112512542014204};
const static __m128i multi1_17 = {(long long)17179869192, (long long)4294967298};
const static __m128i multi2_17 = {(long long)17179869192, (long long)4294967298};
const static __m128i mask_17 = {(long long)562945658585087, (long long)562945658585087};

static void simdhunpack17(const uint8_t *in, uint32_t *out) {
  enum { b = 17 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 3 };
  enum { shift2 = 7 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_17);
    ca = _mm_mullo_epi32(ca, multi1_17);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_17);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_17);
    ca = _mm_mullo_epi32(ca, multi2_17);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_17);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_18 = {(long long)-9222242827675500288, (long long)-9221112512542014204};
const static __m128i shufkey2_18 = {(long long)-9222242827675500288, (long long)-9221112512542014204};
const static __m128i multi1_18 = {(long long)68719476800, (long long)4294967300};
const static __m128i multi2_18 = {(long long)68719476800, (long long)4294967300};
const static __m128i mask_18 = {(long long)1125895612137471, (long long)1125895612137471};

static void simdhunpack18(const uint8_t *in, uint32_t *out) {
  enum { b = 18 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_18);
    ca = _mm_mullo_epi32(ca, multi1_18);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_18);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_18);
    ca = _mm_mullo_epi32(ca, multi2_18);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_18);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_19 = {(long long)-9222242827675500288, (long long)-9220829935788751612};
const static __m128i shufkey2_19 = {(long long)361417179368915200, (long long)-9220829933758642683};
const static __m128i multi1_19 = {(long long)34359738432, (long long)137438953473};
const static __m128i multi2_19 = {(long long)4294967304, (long long)17179869216};
const static __m128i mask_19 = {(long long)2251795519242239, (long long)2251795519242239};

static void simdhunpack19(const uint8_t *in, uint32_t *out) {
  enum { b = 19 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 7 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_19);
    ca = _mm_mullo_epi32(ca, multi1_19);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_19);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_19);
    ca = _mm_mullo_epi32(ca, multi2_19);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_19);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_20 = {(long long)-9222242827675500288, (long long)-9220829933758642683};
const static __m128i shufkey2_20 = {(long long)-9222242827675500288, (long long)-9220829933758642683};
const static __m128i multi1_20 = {(long long)4294967312, (long long)4294967312};
const static __m128i multi2_20 = {(long long)4294967312, (long long)4294967312};
const static __m128i mask_20 = {(long long)4503595333451775, (long long)4503595333451775};

static void simdhunpack20(const uint8_t *in, uint32_t *out) {
  enum { b = 20 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 4 };
  enum { shift2 = 4 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_20);
    ca = _mm_mullo_epi32(ca, multi1_20);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_20);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_20);
    ca = _mm_mullo_epi32(ca, multi2_20);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_20);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_21 = {(long long)361417179368915200, (long long)723118043475412485};
const static __m128i shufkey2_21 = {(long long)-9221960250989346560, (long long)-9220547356988602875};
const static __m128i multi1_21 = {(long long)17179869312, (long long)4294967328};
const static __m128i multi2_21 = {(long long)137438953476, (long long)34359738369};
const static __m128i mask_21 = {(long long)9007194961870847, (long long)9007194961870847};

static void simdhunpack21(const uint8_t *in, uint32_t *out) {
  enum { b = 21 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 7 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_21);
    ca = _mm_mullo_epi32(ca, multi1_21);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_21);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_21);
    ca = _mm_mullo_epi32(ca, multi2_21);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_21);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_22 = {(long long)361417179368915200, (long long)-9220547356988602875};
const static __m128i shufkey2_22 = {(long long)361417179368915200, (long long)-9220547356988602875};
const static __m128i multi1_22 = {(long long)4294967360, (long long)68719476740};
const static __m128i multi2_22 = {(long long)4294967360, (long long)68719476740};
const static __m128i mask_22 = {(long long)18014394218708991, (long long)18014394218708991};

static void simdhunpack22(const uint8_t *in, uint32_t *out) {
  enum { b = 22 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_22);
    ca = _mm_mullo_epi32(ca, multi1_22);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_22);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_22);
    ca = _mm_mullo_epi32(ca, multi2_22);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_22);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_23 = {(long long)361417179368915200, (long long)795458214283380229};
const static __m128i shufkey2_23 = {(long long)433757350092996864, (long long)-9220264778188454138};
const static __m128i multi1_23 = {(long long)4294967424, (long long)17179869186};
const static __m128i multi2_23 = {(long long)8589934593, (long long)34359738372};
const static __m128i mask_23 = {(long long)36028792732385279, (long long)36028792732385279};

static void simdhunpack23(const uint8_t *in, uint32_t *out) {
  enum { b = 23 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 7 };
  enum { shift2 = 4 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_23);
    ca = _mm_mullo_epi32(ca, multi1_23);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_23);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_23);
    ca = _mm_mullo_epi32(ca, multi2_23);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_23);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_24 = {(long long)-9221960248892194560, (long long)-9220264776191965434};
const static __m128i shufkey2_24 = {(long long)-9221960248892194560, (long long)-9220264776191965434};

static void simdhunpack24(const uint8_t *in, uint32_t *out) {
  enum { b = 24 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_24);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_24);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_25 = {(long long)433757350092996864, (long long)867798387121456902};
const static __m128i shufkey2_25 = {(long long)433757350092996864, (long long)867798387121456902};
const static __m128i multi1_25 = {(long long)17179869192, (long long)4294967298};
const static __m128i multi2_25 = {(long long)17179869192, (long long)4294967298};
const static __m128i mask_25 = {(long long)144115183814443007, (long long)144115183814443007};

static void simdhunpack25(const uint8_t *in, uint32_t *out) {
  enum { b = 25 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 3 };
  enum { shift2 = 7 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_25);
    ca = _mm_mullo_epi32(ca, multi1_25);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_25);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_25);
    ca = _mm_mullo_epi32(ca, multi2_25);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_25);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_26 = {(long long)433757350092996864, (long long)867798387121456902};
const static __m128i shufkey2_26 = {(long long)433757350092996864, (long long)867798387121456902};
const static __m128i multi1_26 = {(long long)68719476800, (long long)4294967300};
const static __m128i multi2_26 = {(long long)68719476800, (long long)4294967300};
const static __m128i mask_26 = {(long long)288230371923853311, (long long)288230371923853311};

static void simdhunpack26(const uint8_t *in, uint32_t *out) {
  enum { b = 26 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 6 };
  enum { shift2 = 6 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_26);
    ca = _mm_mullo_epi32(ca, multi1_26);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_26);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_26);
    ca = _mm_mullo_epi32(ca, multi2_26);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_26);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_27 = {(long long)433757350092996864, (long long)940138559942690566};
const static __m128i shufkey2_27 = {(long long)506097522914230528, (long long)940138559959533575};
const static __m128i multi1_27 = {(long long)4294967304, (long long)17179869192};
const static __m128i multi2_27 = {(long long)137438953474, (long long)4294967304};
const static __m128i mask_27 = {(long long)576460748142673919, (long long)576460748142673919};

static void simdhunpack27(const uint8_t *in, uint32_t *out) {
  enum { b = 27 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 3 };
  enum { shift2 = 5 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_27);
    ca = _mm_blend_epi16(ca, _mm_srli_epi64(ca, 6), 48);
    ca = _mm_mullo_epi32(ca, multi1_27);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_27);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_27);
    ca = _mm_blend_epi16(ca, _mm_slli_epi64(ca, 1), 12);
    ca = _mm_mullo_epi32(ca, multi2_27);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_27);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_28 = {(long long)433757350092996864, (long long)940138559959533575};
const static __m128i shufkey2_28 = {(long long)433757350092996864, (long long)940138559959533575};
const static __m128i multi1_28 = {(long long)4294967312, (long long)4294967312};
const static __m128i multi2_28 = {(long long)4294967312, (long long)4294967312};
const static __m128i mask_28 = {(long long)1152921500580315135, (long long)1152921500580315135};

static void simdhunpack28(const uint8_t *in, uint32_t *out) {
  enum { b = 28 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 4 };
  enum { shift2 = 4 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_28);
    ca = _mm_mullo_epi32(ca, multi1_28);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_28);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_28);
    ca = _mm_mullo_epi32(ca, multi2_28);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_28);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey2_29 = {(long long)506097522914230528, (long long)1012478732780767239};
const static __m128i multi1_29 = {(long long)17179869188, (long long)4294967297};
const static __m128i multi2_29 = {(long long)17179869192, (long long)4294967298};
const static __m128i mask_29 = {(long long)2305843005455597567, (long long)2305843005455597567};

static void simdhunpack29(const uint8_t *in, uint32_t *out) {
  enum { b = 29 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 2 };
  enum { shift2 = 3 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i tmp =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    __m128i ca = _mm_blend_epi16(tmp, _mm_slli_epi64(tmp, 3), 236);
    const static __m128i x2 =
        _mm_set_epi8((char)14, (char)13, (char)12, (char)11, (char)10, (char)9, (char)8, (char)7, (char)7, (char)6, (char)5, (char)4, (char)3, (char)2, (char)1, (char)0);
    ca = _mm_shuffle_epi8(ca, x2);
    const static __m128i x1 =
        _mm_set_epi8((char)14, (char)13, (char)12, (char)11, (char)10, (char)9, (char)8, (char)7, (char)7, (char)6, (char)5, (char)4, (char)3, (char)2, (char)1, (char)0);
    ca = _mm_blend_epi16(ca, _mm_shuffle_epi8(tmp, x1), 51);
    ca = _mm_mullo_epi32(ca, multi1_29);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_29);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_29);
    ca = _mm_blend_epi16(ca, _mm_srli_epi64(ca, 4), 51);
    ca = _mm_mullo_epi32(ca, multi2_29);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_29);
    _mm_storeu_si128(pCurr++, ca);
  }
}
const static __m128i shufkey1_30 = {(long long)433757350092996864, (long long)1012478732780767239};
const static __m128i shufkey2_30 = {(long long)433757350092996864, (long long)1012478732780767239};
const static __m128i multi1_30 = {(long long)4294967300, (long long)4294967300};
const static __m128i multi2_30 = {(long long)4294967300, (long long)4294967300};
const static __m128i mask_30 = {(long long)4611686015206162431, (long long)4611686015206162431};

static void simdhunpack30(const uint8_t *in, uint32_t *out) {
  enum { b = 30 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  enum { shift1 = 2 };
  enum { shift2 = 2 };
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i tmp =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    __m128i ca = _mm_shuffle_epi8(tmp, shufkey1_30);
    ca = _mm_blend_epi16(ca, _mm_srli_epi64(ca, 4), 60);
    ca = _mm_blend_epi16(ca, _mm_slli_epi64(tmp, 4), 12);
    ca = _mm_mullo_epi32(ca, multi1_30);
    ca = _mm_srli_epi32(ca, shift1);
    ca = _mm_and_si128(ca, mask_30);
    _mm_storeu_si128(pCurr++, ca);
    tmp =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(tmp, shufkey2_30);
    ca = _mm_blend_epi16(ca, _mm_srli_epi64(ca, 4), 60);
    ca = _mm_blend_epi16(ca, _mm_slli_epi64(tmp, 4), 12);
    ca = _mm_mullo_epi32(ca, multi2_30);
    ca = _mm_srli_epi32(ca, shift2);
    ca = _mm_and_si128(ca, mask_30);
    _mm_storeu_si128(pCurr++, ca);
  }
}

static void simdhunpack31(const uint8_t *in, uint32_t *out) {
  enum { b = 31 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  const static __m128i mask = _mm_set1_epi32((1U << 31) - 1);
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i tmp =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    __m128i ca = _mm_blend_epi16(tmp, _mm_slli_epi64(tmp, 1), 12);
    ca = _mm_blend_epi16(ca, _mm_srli_epi64(_mm_slli_si128(tmp, 1), 6), 48);
    ca = _mm_blend_epi16(ca, _mm_slli_epi64(tmp, 3), 192);
    ca = _mm_and_si128(ca, mask);
    _mm_storeu_si128(pCurr++, ca);
    tmp =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_blend_epi16(_mm_srli_epi64(tmp, 4),
                         _mm_slli_epi64(_mm_srli_si128(tmp, 1), 5), 12);
    ca = _mm_blend_epi16(ca, _mm_srli_epi64(tmp, 2), 48);
    ca = _mm_blend_epi16(ca, _mm_srli_epi64(tmp, 1), 192);
    ca = _mm_and_si128(ca, mask);
    _mm_storeu_si128(pCurr++, ca);
  }
}

const static __m128i shufkey1_32 = {506097522914230528, 1084818905618843912};
const static __m128i shufkey2_32 = {506097522914230528, 1084818905618843912};

static void simdhunpack32(const uint8_t *in, uint32_t *out) {
  enum { b = 32 };
  __m128i *pCurr = reinterpret_cast<__m128i *>(out);
  for (uint32_t j = 0; j < 16; ++j) {
    __m128i ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j));
    ca = _mm_shuffle_epi8(ca, shufkey1_32);
    _mm_storeu_si128(pCurr++, ca);
    ca = _mm_loadu_si128(reinterpret_cast<const __m128i *>(in + b * j + b / 2));
    ca = _mm_shuffle_epi8(ca, shufkey2_32);
    _mm_storeu_si128(pCurr++, ca);
  }
}

void simdhunpack(const uint8_t *in, uint32_t *out, uint32_t bit) {
  switch (bit) {
  case 0:
    SIMD_nullunpacker32(in, out);
    return;

  case 1:
    simdhunpack1(in, out);
    return;

  case 2:
    simdhunpack2(in, out);
    return;

  case 3:
    simdhunpack3(in, out);
    return;

  case 4:
    simdhunpack4(in, out);
    return;

  case 5:
    simdhunpack5(in, out);
    return;

  case 6:
    simdhunpack6(in, out);
    return;

  case 7:
    simdhunpack7(in, out);
    return;

  case 8:
    simdhunpack8(in, out);
    return;

  case 9:
    simdhunpack9(in, out);
    return;

  case 10:
    simdhunpack10(in, out);
    return;

  case 11:
    simdhunpack11(in, out);
    return;

  case 12:
    simdhunpack12(in, out);
    return;

  case 13:
    simdhunpack13(in, out);
    return;

  case 14:
    simdhunpack14(in, out);
    return;

  case 15:
    simdhunpack15(in, out);
    return;

  case 16:
    simdhunpack16(in, out);
    return;

  case 17:
    simdhunpack17(in, out);
    return;

  case 18:
    simdhunpack18(in, out);
    return;

  case 19:
    simdhunpack19(in, out);
    return;

  case 20:
    simdhunpack20(in, out);
    return;

  case 21:
    simdhunpack21(in, out);
    return;

  case 22:
    simdhunpack22(in, out);
    return;

  case 23:
    simdhunpack23(in, out);
    return;

  case 24:
    simdhunpack24(in, out);
    return;

  case 25:
    simdhunpack25(in, out);
    return;

  case 26:
    simdhunpack26(in, out);
    return;

  case 27:
    simdhunpack27(in, out);
    return;

  case 28:
    simdhunpack28(in, out);
    return;

  case 29:
    simdhunpack29(in, out);
    return;

  case 30:
    simdhunpack30(in, out);
    return;

  case 31:
    simdhunpack31(in, out);
    return;

  case 32:
    simdhunpack32(in, out);
    return;

  default:
    break;
  }
  throw std::logic_error("number of bits is unsupported");
}

} // namespace FastPForLib
