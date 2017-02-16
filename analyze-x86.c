//      analyze-x86: instruction analysis tool
//      please note: vectorization counters do not directly relate to performance!
//
//      Copyright 2010,2011 Meya Argenta <fierevere@ya.ru>
//        Complete rewrite on C, taken instruction arrays from perl tool
//        Added: SSSE3, FMA, AVX, VT-x, SVM and modern CPU (Atom+) support
//      Feb 2017, added: AVX, AVX2, AVX-512 support by Vitalii Kamnev kamnevv@gmail.com
//                source: https://github.com/asmjit/asmjit/blob/master/src/asmjit/x86/x86inst.h

//      This is a _complete_ C rewrite of very slow analyze-x86.pl tool which is
//       (c) Tavis Ormandy <taviso@gentoo.org> 2003
//        Will Woods <wwoods@gentoo.org>, Georgi Georgiev <chutz@gg3.net>,
//        Ryan Hill <dirtyepic.sk@gmail.com>,Ward Poelmans <wpoely86@gmail.com>
//        ONLY Instruction arrays were taken for up to SSE3 and including SSE4(a)
//        those are publicly available via various sources though...

//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#define NUM486 6
#define NUM586 4
#define NUM686 54
#define NUMMMX 47
#define NUMSSE 81
#define NUMSSE2 130
#define NUMSSE3 15
#define NUMSSE41 49
#define NUMSSE42 7
#define NUMSSE4a 6
#define NUM3DNOW 24
#define NUM3DNOWEXT 24
#define NUMSSSE3 16
#define NUMAVX 152
#define NUMAVX2 141
#define NUMAVX512 298
#define NUMFMA3 60
#define NUMFMA4 20
#define NUMXOP 147
#define NUMAES 6
#define NUMVTX 10
#define NUMSVM 8


static char set486 [NUM486] [12] = {
    "bswap",
    "cmpxchg",
    "invd",
    "invlpg",
    "wbinvd",
    "xadd"
};
static char set586 [NUM586] [12] = {
    "cmpxchg8b",
    "rdmsr",
    "rdtsc",
    "wrmsr"
};
static char set686 [NUM686] [12] = {
    "cmova",
    "cmovae",
    "cmovb",
    "cmovbe",
    "cmovc",
    "cmove",
    "cmovg",
    "cmovge",
    "cmovl",
    "cmovle",
    "cmovna",
    "cmovnae",
    "cmovnb",
    "cmovnbe",
    "cmovnc",
    "cmovne",
    "cmovng",
    "cmovnge",
    "cmovnl",
    "cmovnle",
    "cmovno",
    "cmovnp",
    "cmovns",
    "cmovnz",
    "cmovo",
    "cmovp",
    "cmovs",
    "cmovz",
    "fcmova",
    "fcmovae",
    "fcmovb",
    "fcmovbe",
    "fcmove",
    "fcmovna",
    "fcmovnae",
    "fcmovnb",
    "fcmovnbe",
    "fcmovne",
    "fcmovnu",
    "fcmovu",
    "fcomi",
    "fcomip",
    "fcompi",
    "fucomi",
    "fucomip",
    "fucompi",
    "fxrstor",
    "fxsave",
    "rdpmc",
    "sysenter",
    "sysexit",
    "ud2",
    "ud2a",
    "ud2b"
};

static char setmmx [NUMMMX] [12] = {
    "emms",
    "movd",
    "movq",
    "packssdw",
    "packsswb",
    "packuswb",
    "paddb",
    "paddd",
    "paddsb",
    "paddsw",
    "paddusb",
    "paddusw",
    "paddw",
    "pand",
    "pandn",
    "pcmpeqb",
    "pcmpeqd",
    "pcmpeqw",
    "pcmpgtb",
    "pcmpgtd",
    "pcmpgtw",
    "pmaddwd",
    "pmulhw",
    "pmullw",
    "por",
    "pslld",
    "psllq",
    "psllw",
    "psrad",
    "psraw",
    "psrld",
    "psrlq",
    "psrlw",
    "psubb",
    "psubd",
    "psubsb",
    "psubsw",
    "psubusb",
    "psubusw",
    "psubw",
    "punpckhbw",
    "punpckhdq",
    "punpckhwd",
    "punpcklbw",
    "punpckldq",
    "punpcklwd",
    "pxor"
};

static char setsse [NUMSSE] [12] = {
    "addps",
    "addss",
    "andnps",
    "andps",
    "cmpeqps",
    "cmpeqss",
    "cmpleps",
    "cmpless",
    "cmpltps",
    "cmpltss",
    "cmpneqps",
    "cmpneqss",
    "cmpnleps",
    "cmpnless",
    "cmpnltps",
    "cmpnltss",
    "cmpordps",
    "cmpordss",
    "cmpps",
    "cmpss",
    "cmpunordps",
    "cmpunordss",
    "comiss",
    "cvtpi2ps",
    "cvtps2pi",
    "cvtsi2ss",
    "cvtss2si",
    "cvttps2pi",
    "cvttss2si",
    "divps",
    "divss",
    "ldmxcsr",
    "maskmovq",
    "maxps",
    "maxss",
    "minps",
    "minss",
    "movaps",
    "movhlps",
    "movhps",
    "movlhps",
    "movlps",
    "movmskps",
    "movntps",
    "movntq",
    "movss",
    "movups",
    "mulps",
    "mulss",
    "orps",
    "pavgb",
    "pavgw",
    "pextrw",
    "pinsrw",
    "pmaxsw",
    "pmaxub",
    "pminsw",
    "pminub",
    "pmovmskb",
    "pmulhuw",
    "prefetchnta",
    "prefetcht0",
    "prefetcht1",
    "prefetcht2",
    "psadbw",
    "pshufw",
    "rcpps",
    "rcpss",
    "rsqrtps",
    "rsqrtss",
    "sfence",
    "shufps",
    "sqrtps",
    "sqrtss",
    "stmxcsr",
    "subps",
    "subss",
    "ucomiss",
    "unpckhps",
    "unpcklps",
    "xorps"
};

static char setsse2 [NUMSSE2] [12] = {
    "addpd",
    "addsd",
    "andnpd",
    "andpd",
    "clflush",
    "cmpeqpd",
    "cmpeqsd",
    "cmplepd",
    "cmplesd",
    "cmpltpd",
    "cmpltsd",
    "cmpneqpd",
    "cmpneqsd",
    "cmpnlepd",
    "cmpnlesd",
    "cmpnltpd",
    "cmpnltsd",
    "cmpordpd",
    "cmpordsd",
    "cmppd",
    "cmpsd",
    "cmpunordpd",
    "cmpunordsd",
    "comisd",
    "cvtdq2pd",
    "cvtdq2ps",
    "cvtpd2dq",
    "cvtpd2pi",
    "cvtpd2ps",
    "cvtpi2pd",
    "cvtps2dq",
    "cvtps2pd",
    "cvtsd2si",
    "cvtsd2ss",
    "cvtsi2sd",
    "cvtss2sd",
    "cvttpd2dq",
    "cvttpd2pi",
    "cvttps2dq",
    "cvttsd2si",
    "divpd",
    "divsd",
    "lfence",
    "maskmovdqu",
    "maxpd",
    "maxsd",
    "mfence",
    "minpd",
    "minsd",
    "movapd",
    "movd",
    "movdq2q",
    "movdqa",
    "movdqu",
    "movhpd",
    "movlpd",
    "movmskpd",
    "movntdq",
    "movnti",
    "movntpd",
    "movq",
    "movq2dq",
    "movsd",
    "movupd",
    "mulpd",
    "mulsd",
    "orpd",
    "packssdw",
    "packsswb",
    "packuswb",
    "paddb",
    "paddd",
    "paddq",
    "paddsb",
    "paddsw",
    "paddusb",
    "paddusw",
    "paddw",
    "pand",
    "pandn",
    "pause",
    "pavgb",
    "pavgw",
    "pcmpeqb",
    "pcmpeqd",
    "pcmpeqw",
    "pcmpgtb",
    "pcmpgtd",
    "pcmpgtw",
    "pextrw",
    "pinsrw",
    "pmaddwd",
    "pmaxsw",
    "pmaxub",
    "pminsw",
    "pminub",
    "pmovmskb",
    "pmulhuw",
    "pmulhw",
    "pmullw",
    "pmuludq",
    "por",
    "psadbw",
    "pshufd",
    "pshufhw",
    "pshuflw",
    "pslld",
    "pslldq",
    "psllq",
    "psllw",
    "psrad",
    "psraw",
    "psrld",
    "psrldq",
    "psrlq",
    "psrlw",
    "psubb",
    "psubd",
    "psubq",
    "psubsb",
    "psubsw",
    "psubusb",
    "psubusw",
    "psubw",
    "punpckhbw",
    "punpckhdq",
    "punpckhqdq",
    "punpckhwd",
    "punpcklbw",
    "punpckldq"
};

static char set3dnow [NUM3DNOW] [12] = {
    "pavgusb",
    "pfadd",
    "pfsub",
    "pfsubr",
    "pfacc",
    "pfcmpge",
    "pfcmpgt",
    "pfcmpeq",
    "pfmin",
    "pfmax",
    "pi2fw",
    "pi2fd",
    "pf2iw",
    "pf2id",
    "pfrcp",
    "pfrsqrt",
    "pfmul",
    "pfrcpit1",
    "pfrsqit1",
    "pfrcpit2",
    "pmulhrw",
    "pswapw",
    "femms",
    "prefetch"
};

static char set3dnowext [NUM3DNOWEXT] [12] = {
    "pf2iw",
    "pfnacc",
    "pfpnacc",
    "pi2fw",
    "pswapd",
    "maskmovq",
    "movntq",
    "pavgb",
    "pavgw",
    "pextrw",
    "pinsrw",
    "pmaxsw",
    "pmaxub",
    "pminsw",
    "pminub",
    "pmovmskb",
    "pmulhuw",
    "prefetchnta",
    "prefetcht0",
    "prefetcht1",
    "prefetcht2",
    "psadbw",
    "pshufw",
    "sfence"
};

static char setsse3 [NUMSSE3] [12] = {
    "addsubpd",
    "addsubps",
    "fisttp",
    "fisttpl",
    "fisttpll",
    "haddpd",
    "haddps",
    "hsubpd",
    "hsubps",
    "lddqu",
    "monitor",
    "movddup",
    "movshdup",
    "movsldup",
    "mwait"
};

static char setssse3 [NUMSSSE3] [12] = {
    "psignb",
    "psignw",
    "psignd",
    "pabsb",
    "pabsw",
    "pabsw",
    "palignr",
    "pshufb",
    "pmulhrsw",
    "pmaddubsw",
    "phsubw",
    "phsubd",
    "phsubsw",
    "phaddw",
    "phaddd",
    "phaddsw"
};

static char setsse41 [NUMSSE41] [12] = {
    "blendpd",                            //  {SSE4_1}
    "blendps",                            //  {SSE4_1}
    "blendvpd",                           //  {SSE4_1}
    "blendvps",                           //  {SSE4_1}
    "dppd",                               //  {SSE4_1}
    "dpps",                               //  {SSE4_1}
    "extractps",                          //  {SSE4_1}
    "insertps",                           //  {SSE4_1}
    "movntdqa",                           //  {SSE4_1}
    "mpsadbw",                            //  {SSE4_1}
    "packusdw",                           //  {SSE4_1}
    "pblendvb",                           //  {SSE4_1}
    "pblendw",                            //  {SSE4_1}
    "pcmpeqq",                            //  {SSE4_1}
    "pextrb",                             //  {SSE4_1}
    "pextrd",                             //  {SSE4_1}
    "pextrq",                             // [X64] {SSE4_1}
    "pextrw",                             //  {MMX2|SSE2|SSE4_1}
    "phminposuw",                         //  {SSE4_1}
    "pinsrb",                             //  {SSE4_1}
    "pinsrd",                             //  {SSE4_1}
    "pinsrq",                             // [X64] {SSE4_1}
    "pmaxsb",                             //  {SSE4_1}
    "pmaxsd",                             //  {SSE4_1}
    "pmaxud",                             //  {SSE4_1}
    "pmaxuw",                             //  {SSE4_1}
    "pminsb",                             //  {SSE4_1}
    "pminsd",                             //  {SSE4_1}
    "pminud",                             //  {SSE4_1}
    "pminuw",                             //  {SSE4_1}
    "pmovsxbd",                           //  {SSE4_1}
    "pmovsxbq",                           //  {SSE4_1}
    "pmovsxbw",                           //  {SSE4_1}
    "pmovsxdq",                           //  {SSE4_1}
    "pmovsxwd",                           //  {SSE4_1}
    "pmovsxwq",                           //  {SSE4_1}
    "pmovzxbd",                           //  {SSE4_1}
    "pmovzxbq",                           //  {SSE4_1}
    "pmovzxbw",                           //  {SSE4_1}
    "pmovzxdq",                           //  {SSE4_1}
    "pmovzxwd",                           //  {SSE4_1}
    "pmovzxwq",                           //  {SSE4_1}
    "pmuldq",                             //  {SSE4_1}
    "pmulld",                             //  {SSE4_1}
    "ptest",                              //  {SSE4_1}
    "roundpd",                            //  {SSE4_1}
    "roundps",                            //  {SSE4_1}
    "roundsd",                            //  {SSE4_1}
    "roundss"                             //  {SSE4_1}
};

static char setsse42 [NUMSSE42] [12] = {
   "crc32",                             //  {SSE4_2}
   "pcmpestri",                         //  {SSE4_2}
   "pcmpestrm",                         //  {SSE4_2}
   "pcmpgtq",                           //  {SSE4_2}
   "pcmpistri",                         //  {SSE4_2}
   "pcmpistrm",                         //  {SSE4_2}
   "popcnt"                             //  {SSE4_2}
};

static char setsse4a [NUMSSE4a] [12] = {
    "extrq",                           //  {SSE4A}
    "insertq",                         //  {SSE4A}
    "lzcnt",                           //  {SSE4A}
    "movntsd",                         //  {SSE4A}
    "movntss",                         //  {SSE4A}
    "popcnt"                           //  {SSE4A|SSE4_2}

}; 

static char setavx [NUMAVX] [24] = {
    "vaddpd",                               //  {AVX|AVX512_F (VL)}
    "vaddps",                               //  {AVX|AVX512_F (VL)}
    "vaddsd",                               //  {AVX|AVX512_F}
    "vaddss",                               //  {AVX|AVX512_F}
    "vaddsubpd",                            //  {AVX}
    "vaddsubps",                            //  {AVX}
    "vaesdec",                              //  {AES|AVX}
    "vaesdeclast",                          //  {AES|AVX}
    "vaesenc",                              //  {AES|AVX}
    "vaesenclast",                          //  {AES|AVX}
    "vaesimc",                              //  {AES|AVX}
    "vaeskeygenassist",                     //  {AES|AVX}
    "vandnpd",                              //  {AVX|AVX512_DQ (VL)}
    "vandnps",                              //  {AVX|AVX512_DQ (VL)}
    "vandpd",                               //  {AVX|AVX512_DQ (VL)}
    "vandps",                               //  {AVX|AVX512_DQ (VL)}
    "vblendpd",                             //  {AVX}
    "vblendps",                             //  {AVX}
    "vblendvpd",                            //  {AVX}
    "vblendvps",                            //  {AVX}
    "vbroadcastf128",                       //  {AVX}
    "vcmppd",                               //  {AVX|AVX512_F (VL)}
    "vcmpps",                               //  {AVX|AVX512_F (VL)}
    "vcmpsd",                               //  {AVX|AVX512_F}
    "vcmpss",                               //  {AVX|AVX512_F}
    "vcomisd",                              //  {AVX|AVX512_F}
    "vcomiss",                              //  {AVX|AVX512_F}
    "vcvtdq2pd",                            //  {AVX|AVX512_F (VL)}
    "vcvtdq2ps",                            //  {AVX|AVX512_F (VL)}
    "vcvtpd2dq",                            //  {AVX|AVX512_F (VL)}
    "vcvtpd2ps",                            //  {AVX|AVX512_F (VL)}
    "vcvtps2dq",                            //  {AVX|AVX512_F (VL)}
    "vcvtps2pd",                            //  {AVX|AVX512_F (VL)}
    "vcvtsd2si",                            //  {AVX|AVX512_F}
    "vcvtsd2ss",                            //  {AVX|AVX512_F}
    "vcvtsi2sd",                            //  {AVX|AVX512_F}
    "vcvtsi2ss",                            //  {AVX|AVX512_F}
    "vcvtss2sd",                            //  {AVX|AVX512_F}
    "vcvtss2si",                            //  {AVX|AVX512_F}
    "vcvttpd2dq",                           //  {AVX|AVX512_F (VL)}
    "vcvttps2dq",                           //  {AVX|AVX512_F (VL)}
    "vcvttsd2si",                           //  {AVX|AVX512_F}
    "vcvttss2si",                           //  {AVX|AVX512_F}
    "vdivpd",                               //  {AVX|AVX512_F (VL)}
    "vdivps",                               //  {AVX|AVX512_F (VL)}
    "vdivsd",                               //  {AVX|AVX512_F}
    "vdivss",                               //  {AVX|AVX512_F}
    "vdppd",                                //  {AVX}
    "vdpps",                                //  {AVX}
    "vextractf128",                         //  {AVX}
    "vextractps",                           //  {AVX|AVX512_F}
    "vhaddpd",                              //  {AVX}
    "vhaddps",                              //  {AVX}
    "vhsubpd",                              //  {AVX}
    "vhsubps",                              //  {AVX}
    "vinsertf128",                          //  {AVX}
    "vinsertps",                            //  {AVX|AVX512_F}
    "vlddqu",                               //  {AVX}
    "vldmxcsr",                             //  {AVX}
    "vmaskmovdqu",                          //  {AVX}
    "vmaskmovpd",                           //  {AVX}
    "vmaskmovps",                           //  {AVX}
    "vmaxpd",                               //  {AVX|AVX512_F (VL)}
    "vmaxps",                               //  {AVX|AVX512_F (VL)}
    "vmaxsd",                               //  {AVX|AVX512_F (VL)}
    "vmaxss",                               //  {AVX|AVX512_F (VL)}
    "vminpd",                               //  {AVX|AVX512_F (VL)}
    "vminps",                               //  {AVX|AVX512_F (VL)}
    "vminsd",                               //  {AVX|AVX512_F (VL)}
    "vminss",                               //  {AVX|AVX512_F (VL)}
    "vmovapd",                              //  {AVX|AVX512_F (VL)}
    "vmovaps",                              //  {AVX|AVX512_F (VL)}
    "vmovd",                                //  {AVX|AVX512_F}
    "vmovddup",                             //  {AVX|AVX512_F (VL)}
    "vmovdqa",                              //  {AVX}
    "vmovdqu",                              //  {AVX}
    "vmovhlps",                             //  {AVX|AVX512_F}
    "vmovhpd",                              //  {AVX|AVX512_F}
    "vmovhps",                              //  {AVX|AVX512_F}
    "vmovlhps",                             //  {AVX|AVX512_F}
    "vmovlpd",                              //  {AVX|AVX512_F}
    "vmovlps",                              //  {AVX|AVX512_F}
    "vmovmskpd",                            //  {AVX}
    "vmovmskps",                            //  {AVX}
    "vmovntdq",                             //  {AVX|AVX512_F (VL)}
    "vmovntpd",                             //  {AVX|AVX512_F (VL)}
    "vmovntps",                             //  {AVX|AVX512_F (VL)}
    "vmovq",                                //  {AVX|AVX512_F}
    "vmovsd",                               //  {AVX|AVX512_F}
    "vmovshdup",                            //  {AVX|AVX512_F (VL)}
    "vmovsldup",                            //  {AVX|AVX512_F (VL)}
    "vmovss",                               //  {AVX|AVX512_F}
    "vmovupd",                              //  {AVX|AVX512_F (VL)}
    "vmovups",                              //  {AVX|AVX512_F (VL)}
    "vmulsd",                               //  {AVX|AVX512_F}
    "vmulss",                               //  {AVX|AVX512_F}
    "vorpd",                                //  {AVX|AVX512_DQ (VL)}
    "vorps",                                //  {AVX|AVX512_F (VL)}
    "vpavgb",                               //  {AVX|AVX512_BW (VL)}
    "vpclmulqdq",                           //  {AVX|PCLMULQDQ}
    "vpcmpestri",                           //  {AVX}
    "vpcmpestrm",                           //  {AVX}
    "vpcmpistri",                           //  {AVX}
    "vpcmpistrm",                           //  {AVX}
    "vperm2f128",                           //  {AVX}
    "vpermilpd",                            //  {AVX|AVX512_F (VL)}
    "vpermilps",                            //  {AVX|AVX512_F (VL)}
    "vpextrb",                              //  {AVX|AVX512_BW}
    "vpextrd",                              //  {AVX|AVX512_DQ}
    "vpextrq",                              // [X64] {AVX|AVX512_DQ}
    "vpextrw",                              //  {AVX|AVX512_BW}
    "vphminposuw",                          //  {AVX}
    "vpinsrb",                              //  {AVX|AVX512_BW}
    "vpinsrd",                              //  {AVX|AVX512_DQ}
    "vpinsrq",                              // [X64] {AVX|AVX512_DQ}
    "vpinsrw",                              //  {AVX|AVX512_BW}
    "vptest",                               //  {AVX}
    "vrcpps",                               //  {AVX}
    "vrcpss",                               //  {AVX}
    "vroundpd",                             //  {AVX}
    "vroundps",                             //  {AVX}
    "vroundsd",                             //  {AVX}
    "vroundss",                             //  {AVX}
    "vrsqrtps",                             //  {AVX}
    "vrsqrtss",                             //  {AVX}
    "vshufpd",                              //  {AVX|AVX512_F (VL)}
    "vshufps",                              //  {AVX|AVX512_F (VL)}
    "vsqrtpd",                              //  {AVX|AVX512_F (VL)}
    "vsqrtps",                              //  {AVX|AVX512_F (VL)}
    "vsqrtsd",                              //  {AVX|AVX512_F}
    "vsqrtss",                              //  {AVX|AVX512_F}
    "vstmxcsr",                             //  {AVX}
    "vsubsd",                               //  {AVX|AVX512_F}
    "vsubss",                               //  {AVX|AVX512_F}
    "vtestpd",                              //  {AVX}
    "vtestps",                              //  {AVX}
    "vucomisd",                             //  {AVX|AVX512_F}
    "vucomiss",                             //  {AVX|AVX512_F}
    "vunpckhpd",                            //  {AVX|AVX512_F (VL)}
    "vunpckhps",                            //  {AVX|AVX512_F (VL)}
    "vunpcklpd",                            //  {AVX|AVX512_F (VL)}
    "vunpcklps",                            //  {AVX|AVX512_F (VL)}
    "vxorpd",                               //  {AVX|AVX512_DQ (VL)}
    "vxorps",                               //  {AVX|AVX512_DQ (VL)}
    "vzeroall",                             //  {AVX}
    "vzeroupper"                            //  {AVX}
    "vaesdec",                              //  {AES|AVX}
    "vaesdeclast",                          //  {AES|AVX}
    "vaesenc",                              //  {AES|AVX}
    "vaesenclast",                          //  {AES|AVX}
    "vaesimc",                              //  {AES|AVX}
    "vaeskeygenassist"                      //  {AES|AVX}
};

static char setavx2 [NUMAVX2] [20] = {
    "vbroadcasti128",                   //  {AVX2}
    "vbroadcastsd",                     //  {AVX|AVX2|AVX512_F (VL)}
    "vbroadcastss",                     //  {AVX|AVX2|AVX512_F (VL)}
    "vextracti128",                     //  {AVX2}
    "vgatherdpd",                       //  {AVX2|AVX512_F (VL)}
    "vgatherdps",                       //  {AVX2|AVX512_F (VL)}
    "vgatherqpd",                       //  {AVX2|AVX512_F (VL)}
    "vgatherqps",                       //  {AVX2|AVX512_F (VL)}
    "vinserti128",                      //  {AVX2}
    "vmovntdqa",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vmpsadbw",                         //  {AVX|AVX2}
    "vmulpd",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vmulps",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpabsb",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpabsd",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpabsw",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpackssdw",                        //  {AVX|AVX2|AVX512_BW (VL)}
    "vpacksswb",                        //  {AVX|AVX2|AVX512_BW (VL)}
    "vpackusdw",                        //  {AVX|AVX2|AVX512_BW (VL)}
    "vpackuswb",                        //  {AVX|AVX2|AVX512_BW (VL)}
    "vpaddb",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpaddd",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpaddq",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpaddsb",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpaddsw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpaddusb",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpaddusw",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpaddw",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpalignr",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpand",                            //  {AVX|AVX2}
    "vpandn",                           //  {AVX|AVX2}
    "vpavgw",                           //  {AVX2|AVX512_BW (VL)}
    "vpblendd",                         //  {AVX2}
    "vpblendvb",                        //  {AVX|AVX2}
    "vpblendw",                         //  {AVX|AVX2}
    "vpbroadcastb",                     //  {AVX2|AVX512_BW (VL)}
    "vpbroadcastd",                     //  {AVX2|AVX512_F (VL)}
    "vpbroadcastq",                     //  {AVX2|AVX512_F (VL)}
    "vpbroadcastw",                     //  {AVX2|AVX512_BW (VL)}
    "vpcmpeqb",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpcmpeqd",                         //  {AVX|AVX2|AVX512_F (VL)}
    "vpcmpeqq",                         //  {AVX|AVX2|AVX512_F (VL)}
    "vpcmpeqw",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpcmpgtb",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpcmpgtd",                         //  {AVX|AVX2|AVX512_F (VL)}
    "vpcmpgtq",                         //  {AVX|AVX2|AVX512_F (VL)}
    "vpcmpgtw",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vperm2i128",                       //  {AVX2}
    "vpermd",                           //  {AVX2|AVX512_F (VL)}
    "vpermpd",                          //  {AVX2}
    "vpermps",                          //  {AVX2}
    "vpermq",                           //  {AVX2|AVX512_F (VL)}
    "vpgatherdd",                       //  {AVX2|AVX512_F (VL)}
    "vpgatherdq",                       //  {AVX2|AVX512_F (VL)}
    "vpgatherqd",                       //  {AVX2|AVX512_F (VL)}
    "vpgatherqq",                       //  {AVX2|AVX512_F (VL)}
    "vphaddd",                          //  {AVX|AVX2}
    "vphaddsw",                         //  {AVX|AVX2}
    "vphaddw",                          //  {AVX|AVX2}
    "vphsubd",                          //  {AVX|AVX2}
    "vphsubsw",                         //  {AVX|AVX2}
    "vphsubw",                          //  {AVX|AVX2}
    "vpmaddubsw",                       //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmaddwd",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmaskmovd",                       //  {AVX2}
    "vpmaskmovq",                       //  {AVX2}
    "vpmaxsb",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmaxsd",                          //  {AVX|AVX2|AVX512_F (VL)}
    "vpmaxsw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmaxub",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmaxud",                          //  {AVX|AVX2|AVX512_F (VL)}
    "vpmaxuw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpminsb",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpminsd",                          //  {AVX|AVX2|AVX512_F (VL)}
    "vpminsw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpminub",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpminud",                          //  {AVX|AVX2|AVX512_F (VL)}
    "vpminuw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmovmskb",                        //  {AVX|AVX2}
    "vpmovsxbd",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovsxbq",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovsxbw",                        //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmovsxdq",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovsxwd",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovsxwq",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovzxbd",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovzxbq",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovzxbw",                        //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmovzxdq",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovzxwd",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmovzxwq",                        //  {AVX|AVX2|AVX512_F (VL)}
    "vpmuldq",                          //  {AVX|AVX2|AVX512_F (VL)}
    "vpmulhrsw",                        //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmulhuw",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmulhw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmulld",                          //  {AVX|AVX2|AVX512_F (VL)}
    "vpmullw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpmuludq",                         //  {AVX|AVX2|AVX512_F (VL)}
    "vpor",                             //  {AVX|AVX2}
    "vpsadbw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpshufb",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpshufd",                          //  {AVX|AVX2|AVX512_F (VL)}
    "vpshufhw",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpshuflw",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsignb",                          //  {AVX|AVX2}
    "vpsignd",                          //  {AVX|AVX2}
    "vpsignw",                          //  {AVX|AVX2}
    "vpslld",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpslldq",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsllq",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpsllvd",                          //  {AVX2|AVX512_F (VL)}
    "vpsllvq",                          //  {AVX2|AVX512_F (VL)}
    "vpsllw",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsrad",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpsravd",                          //  {AVX2|AVX512_F (VL)}
    "vpsraw",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsrld",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpsrldq",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsrlq",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpsrlvd",                          //  {AVX2|AVX512_F (VL)}
    "vpsrlvq",                          //  {AVX2|AVX512_F (VL)}
    "vpsrlw",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsubb",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsubd",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpsubq",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vpsubsb",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsubsw",                          //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsubusb",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsubusw",                         //  {AVX|AVX2|AVX512_BW (VL)}
    "vpsubw",                           //  {AVX|AVX2|AVX512_BW (VL)}
    "vpunpckhbw",                       //  {AVX|AVX2|AVX512_BW (VL)}
    "vpunpckhdq",                       //  {AVX|AVX2|AVX512_F (VL)}
    "vpunpckhqdq",                      //  {AVX|AVX2|AVX512_F (VL)}
    "vpunpckhwd",                       //  {AVX|AVX2|AVX512_BW (VL)}
    "vpunpcklbw",                       //  {AVX|AVX2|AVX512_BW (VL)}
    "vpunpckldq",                       //  {AVX|AVX2|AVX512_F (VL)}
    "vpunpcklqdq",                      //  {AVX|AVX2|AVX512_F (VL)}
    "vpunpcklwd",                       //  {AVX|AVX2|AVX512_BW (VL)}
    "vpxor",                            //  {AVX|AVX2}
    "vsubpd",                           //  {AVX|AVX2|AVX512_F (VL)}
    "vsubps"                            //  {AVX|AVX2|AVX512_F (VL)}
};

static char setavx512 [NUMAVX512] [20] = {
    "kaddb",                               //  {AVX512_DQ}
    "kaddd",                               //  {AVX512_BW}
    "kaddq",                               //  {AVX512_BW}
    "kaddw",                               //  {AVX512_DQ}
    "kandb",                               //  {AVX512_DQ}
    "kandd",                               //  {AVX512_BW}
    "kandnb",                              //  {AVX512_DQ}
    "kandnd",                              //  {AVX512_BW}
    "kandnq",                              //  {AVX512_BW}
    "kandnw",                              //  {AVX512_F}
    "kandq",                               //  {AVX512_BW}
    "kandw",                               //  {AVX512_F}
    "kmovb",                               //  {AVX512_DQ}
    "kmovd",                               //  {AVX512_BW}
    "kmovq",                               //  {AVX512_BW}
    "kmovw",                               //  {AVX512_F}
    "knotb",                               //  {AVX512_DQ}
    "knotd",                               //  {AVX512_BW}
    "knotq",                               //  {AVX512_BW}
    "knotw",                               //  {AVX512_F}
    "korb",                                //  {AVX512_DQ}
    "kord",                                //  {AVX512_BW}
    "korq",                                //  {AVX512_BW}
    "kortestb",                            //  {AVX512_DQ}
    "kortestd",                            //  {AVX512_BW}
    "kortestq",                            //  {AVX512_BW}
    "kortestw",                            //  {AVX512_F}
    "korw",                                //  {AVX512_F}
    "kshiftlb",                            //  {AVX512_DQ}
    "kshiftld",                            //  {AVX512_BW}
    "kshiftlq",                            //  {AVX512_BW}
    "kshiftlw",                            //  {AVX512_F}
    "kshiftrb",                            //  {AVX512_DQ}
    "kshiftrd",                            //  {AVX512_BW}
    "kshiftrq",                            //  {AVX512_BW}
    "kshiftrw",                            //  {AVX512_F}
    "ktestb",                              //  {AVX512_DQ}
    "ktestd",                              //  {AVX512_BW}
    "ktestq",                              //  {AVX512_BW}
    "ktestw",                              //  {AVX512_DQ}
    "kunpckbw",                            //  {AVX512_F}
    "kunpckdq",                            //  {AVX512_BW}
    "kunpckwd",                            //  {AVX512_BW}
    "kxnorb",                              //  {AVX512_DQ}
    "kxnord",                              //  {AVX512_BW}
    "kxnorq",                              //  {AVX512_BW}
    "kxnorw",                              //  {AVX512_F}
    "kxorb",                               //  {AVX512_DQ}
    "kxord",                               //  {AVX512_BW}
    "kxorq",                               //  {AVX512_BW}
    "kxorw",                               //  {AVX512_F}
    "valignd",                             //  {AVX512_F (VL)}
    "valignq",                             //  {AVX512_F (VL)}
    "vblendmb",                            //  {AVX512_BW (VL)}
    "vblendmd",                            //  {AVX512_F (VL)}
    "vblendmpd",                           //  {AVX512_F (VL)}
    "vblendmps",                           //  {AVX512_F (VL)}
    "vblendmq",                            //  {AVX512_F (VL)}
    "vblendmw",                            //  {AVX512_BW (VL)}
    "vbroadcastf32x2",                     //  {AVX512_DQ (VL)}
    "vbroadcastf32x4",                     //  {AVX512_F}
    "vbroadcastf32x8",                     //  {AVX512_DQ}
    "vbroadcastf64x2",                     //  {AVX512_DQ (VL)}
    "vbroadcastf64x4",                     //  {AVX512_F}
    "vbroadcasti32x2",                     //  {AVX512_DQ (VL)}
    "vbroadcasti32x4",                     //  {AVX512_F (VL)}
    "vbroadcasti32x8",                     //  {AVX512_DQ}
    "vbroadcasti64x2",                     //  {AVX512_DQ (VL)}
    "vbroadcasti64x4",                     //  {AVX512_F}
    "vcompresspd",                         //  {AVX512_F (VL)}
    "vcompressps",                         //  {AVX512_F (VL)}
    "vcvtpd2qq",                           //  {AVX512_DQ (VL)}
    "vcvtpd2udq",                          //  {AVX512_F (VL)}
    "vcvtpd2uqq",                          //  {AVX512_DQ (VL)}
    "vcvtph2ps",                           //  {AVX512_F|F16C (VL)}
    "vcvtps2ph",                           //  {AVX512_F|F16C (VL)}
    "vcvtps2qq",                           //  {AVX512_DQ (VL)}
    "vcvtps2udq",                          //  {AVX512_F (VL)}
    "vcvtps2uqq",                          //  {AVX512_DQ (VL)}
    "vcvtqq2pd",                           //  {AVX512_DQ (VL)}
    "vcvtqq2ps",                           //  {AVX512_DQ (VL)}
    "vcvtsd2usi",                          //  {AVX512_F}
    "vcvtss2usi",                          //  {AVX512_F}
    "vcvttpd2qq",                          //  {AVX512_F (VL)}
    "vcvttpd2udq",                         //  {AVX512_F (VL)}
    "vcvttpd2uqq",                         //  {AVX512_DQ (VL)}
    "vcvttps2qq",                          //  {AVX512_DQ (VL)}
    "vcvttps2udq",                         //  {AVX512_F (VL)}
    "vcvttps2uqq",                         //  {AVX512_DQ (VL)}
    "vcvttsd2usi",                         //  {AVX512_F}
    "vcvttss2usi",                         //  {AVX512_F}
    "vcvtudq2pd",                          //  {AVX512_F (VL)}
    "vcvtudq2ps",                          //  {AVX512_F (VL)}
    "vcvtuqq2pd",                          //  {AVX512_DQ (VL)}
    "vcvtuqq2ps",                          //  {AVX512_DQ (VL)}
    "vcvtusi2sd",                          //  {AVX512_F}
    "vcvtusi2ss",                          //  {AVX512_F}
    "vdbpsadbw",                           //  {AVX512_BW (VL)}
    "vexp2pd",                             //  {AVX512_ERI}
    "vexp2ps",                             //  {AVX512_ERI}
    "vexpandpd",                           //  {AVX512_F (VL)}
    "vexpandps",                           //  {AVX512_F (VL)}
    "vextractf32x4",                       //  {AVX512_F (VL)}
    "vextractf32x8",                       //  {AVX512_DQ}
    "vextractf64x2",                       //  {AVX512_DQ (VL)}
    "vextractf64x4",                       //  {AVX512_F}
    "vextracti32x4",                       //  {AVX512_F (VL)}
    "vextracti32x8",                       //  {AVX512_DQ}
    "vextracti64x2",                       //  {AVX512_DQ (VL)}
    "vextracti64x4",                       //  {AVX512_F}
    "vfixupimmpd",                         //  {AVX512_F (VL)}
    "vfixupimmps",                         //  {AVX512_F (VL)}
    "vfixupimmsd",                         //  {AVX512_F}
    "vfixupimmss",                         //  {AVX512_F}
    "vfpclasspd",                          //  {AVX512_DQ (VL)}
    "vfpclassps",                          //  {AVX512_DQ (VL)}
    "vfpclasssd",                          //  {AVX512_DQ}
    "vfpclassss",                          //  {AVX512_DQ}
    "vgatherpf0dpd",                       //  {AVX512_PFI}
    "vgatherpf0dps",                       //  {AVX512_PFI}
    "vgatherpf0qpd",                       //  {AVX512_PFI}
    "vgatherpf0qps",                       //  {AVX512_PFI}
    "vgatherpf1dpd",                       //  {AVX512_PFI}
    "vgatherpf1dps",                       //  {AVX512_PFI}
    "vgatherpf1qpd",                       //  {AVX512_PFI}
    "vgatherpf1qps",                       //  {AVX512_PFI}
    "vgetexppd",                           //  {AVX512_F (VL)}
    "vgetexpps",                           //  {AVX512_F (VL)}
    "vgetexpsd",                           //  {AVX512_F}
    "vgetexpss",                           //  {AVX512_F}
    "vgetmantpd",                          //  {AVX512_F (VL)}
    "vgetmantps",                          //  {AVX512_F (VL)}
    "vgetmantsd",                          //  {AVX512_F}
    "vgetmantss",                          //  {AVX512_F}
    "vinsertf32x4",                        //  {AVX512_F (VL)}
    "vinsertf32x8",                        //  {AVX512_DQ}
    "vinsertf64x2",                        //  {AVX512_DQ (VL)}
    "vinsertf64x4",                        //  {AVX512_F}
    "vinserti32x4",                        //  {AVX512_F (VL)}
    "vinserti32x8",                        //  {AVX512_DQ}
    "vinserti64x2",                        //  {AVX512_DQ (VL)}
    "vinserti64x4",                        //  {AVX512_F}
    "vmovdqa32",                           //  {AVX512_F (VL)}
    "vmovdqa64",                           //  {AVX512_F (VL)}
    "vmovdqu16",                           //  {AVX512_BW (VL)}
    "vmovdqu32",                           //  {AVX512_F (VL)}
    "vmovdqu64",                           //  {AVX512_F (VL)}
    "vmovdqu8",                            //  {AVX512_BW (VL)}
    "vpabsq",                              //  {AVX512_F (VL)}
    "vpandd",                              //  {AVX512_F (VL)}
    "vpandnd",                             //  {AVX512_F (VL)}
    "vpandnq",                             //  {AVX512_F (VL)}
    "vpandq",                              //  {AVX512_F (VL)}
    "vpbroadcastmb2d",                     //  {AVX512_CDI (VL)}
    "vpbroadcastmb2q",                     //  {AVX512_CDI (VL)}
    "vpcmpb",                              //  {AVX512_BW (VL)}
    "vpcmpd",                              //  {AVX512_F (VL)}
    "vpcmpq",                              //  {AVX512_F (VL)}
    "vpcmpub",                             //  {AVX512_BW (VL)}
    "vpcmpud",                             //  {AVX512_F (VL)}
    "vpcmpuq",                             //  {AVX512_F (VL)}
    "vpcmpuw",                             //  {AVX512_BW (VL)}
    "vpcmpw",                              //  {AVX512_BW (VL)}
    "vpcompressd",                         //  {AVX512_F (VL)}
    "vpcompressq",                         //  {AVX512_F (VL)}
    "vpconflictd",                         //  {AVX512_CDI (VL)}
    "vpconflictq",                         //  {AVX512_CDI (VL)}
    "vpermb",                              //  {AVX512_VBMI (VL)}
    "vpermi2b",                            //  {AVX512_VBMI (VL)}
    "vpermi2d",                            //  {AVX512_F (VL)}
    "vpermi2pd",                           //  {AVX512_F (VL)}
    "vpermi2ps",                           //  {AVX512_F (VL)}
    "vpermi2q",                            //  {AVX512_F (VL)}
    "vpermi2w",                            //  {AVX512_BW (VL)}
    "vpermt2b",                            //  {AVX512_VBMI (VL)}
    "vpermt2d",                            //  {AVX512_F (VL)}
    "vpermt2pd",                           //  {AVX512_F (VL)}
    "vpermt2ps",                           //  {AVX512_F (VL)}
    "vpermt2q",                            //  {AVX512_F (VL)}
    "vpermt2w",                            //  {AVX512_BW (VL)}
    "vpermw",                              //  {AVX512_BW (VL)}
    "vpexpandd",                           //  {AVX512_F (VL)}
    "vpexpandq",                           //  {AVX512_F (VL)}
    "vplzcntd",                            //  {AVX512_CDI (VL)}
    "vplzcntq",                            //  {AVX512_CDI (VL)}
    "vpmadd52huq",                         //  {AVX512_IFMA (VL)}
    "vpmadd52luq",                         //  {AVX512_IFMA (VL)}
    "vpmaxsq",                             //  {AVX512_F (VL)}
    "vpmaxuq",                             //  {AVX512_F (VL)}
    "vpminsq",                             //  {AVX512_F (VL)}
    "vpminuq",                             //  {AVX512_F (VL)}
    "vpmovb2m",                            //  {AVX512_BW (VL)}
    "vpmovd2m",                            //  {AVX512_DQ (VL)}
    "vpmovdb",                             //  {AVX512_F (VL)}
    "vpmovdw",                             //  {AVX512_F (VL)}
    "vpmovm2b",                            //  {AVX512_BW (VL)}
    "vpmovm2d",                            //  {AVX512_DQ (VL)}
    "vpmovm2q",                            //  {AVX512_DQ (VL)}
    "vpmovm2w",                            //  {AVX512_BW (VL)}
    "vpmovq2m",                            //  {AVX512_DQ (VL)}
    "vpmovqb",                             //  {AVX512_F (VL)}
    "vpmovqd",                             //  {AVX512_F (VL)}
    "vpmovqw",                             //  {AVX512_F (VL)}
    "vpmovsdb",                            //  {AVX512_F (VL)}
    "vpmovsdw",                            //  {AVX512_F (VL)}
    "vpmovsqb",                            //  {AVX512_F (VL)}
    "vpmovsqd",                            //  {AVX512_F (VL)}
    "vpmovsqw",                            //  {AVX512_F (VL)}
    "vpmovswb",                            //  {AVX512_BW (VL)}
    "vpmovusdb",                           //  {AVX512_F (VL)}
    "vpmovusdw",                           //  {AVX512_F (VL)}
    "vpmovusqb",                           //  {AVX512_F (VL)}
    "vpmovusqd",                           //  {AVX512_F (VL)}
    "vpmovusqw",                           //  {AVX512_F (VL)}
    "vpmovuswb",                           //  {AVX512_BW (VL)}
    "vpmovw2m",                            //  {AVX512_BW (VL)}
    "vpmovwb",                             //  {AVX512_BW (VL)}
    "vpmullq",                             //  {AVX512_DQ (VL)}
    "vpmultishiftqb",                      //  {AVX512_VBMI (VL)}
    "vpord",                               //  {AVX512_F (VL)}
    "vporq",                               //  {AVX512_F (VL)}
    "vprold",                              //  {AVX512_F (VL)}
    "vprolq",                              //  {AVX512_F (VL)}
    "vprolvd",                             //  {AVX512_F (VL)}
    "vprolvq",                             //  {AVX512_F (VL)}
    "vprord",                              //  {AVX512_F (VL)}
    "vprorq",                              //  {AVX512_F (VL)}
    "vprorvd",                             //  {AVX512_F (VL)}
    "vprorvq",                             //  {AVX512_F (VL)}
    "vpscatterdd",                         //  {AVX512_F (VL)}
    "vpscatterdq",                         //  {AVX512_F (VL)}
    "vpscatterqd",                         //  {AVX512_F (VL)}
    "vpscatterqq",                         //  {AVX512_F (VL)}
    "vpsllvw",                             //  {AVX512_BW (VL)}
    "vpsraq",                              //  {AVX512_F (VL)}
    "vpsravq",                             //  {AVX512_F (VL)}
    "vpsravw",                             //  {AVX512_BW (VL)}
    "vpsrlvw",                             //  {AVX512_BW (VL)}
    "vpternlogd",                          //  {AVX512_F (VL)}
    "vpternlogq",                          //  {AVX512_F (VL)}
    "vptestmb",                            //  {AVX512_BW (VL)}
    "vptestmd",                            //  {AVX512_F (VL)}
    "vptestmq",                            //  {AVX512_F (VL)}
    "vptestmw",                            //  {AVX512_BW (VL)}
    "vptestnmb",                           //  {AVX512_BW (VL)}
    "vptestnmd",                           //  {AVX512_F (VL)}
    "vptestnmq",                           //  {AVX512_F (VL)}
    "vptestnmw",                           //  {AVX512_BW (VL)}
    "vpxord",                              //  {AVX512_F (VL)}
    "vpxorq",                              //  {AVX512_F (VL)}
    "vrangepd",                            //  {AVX512_DQ (VL)}
    "vrangeps",                            //  {AVX512_DQ (VL)}
    "vrangesd",                            //  {AVX512_DQ}
    "vrangess",                            //  {AVX512_DQ}
    "vrcp14pd",                            //  {AVX512_F (VL)}
    "vrcp14ps",                            //  {AVX512_F (VL)}
    "vrcp14sd",                            //  {AVX512_F}
    "vrcp14ss",                            //  {AVX512_F}
    "vrcp28pd",                            //  {AVX512_ERI}
    "vrcp28ps",                            //  {AVX512_ERI}
    "vrcp28sd",                            //  {AVX512_ERI}
    "vrcp28ss",                            //  {AVX512_ERI}
    "vreducepd",                           //  {AVX512_DQ (VL)}
    "vreduceps",                           //  {AVX512_DQ (VL)}
    "vreducesd",                           //  {AVX512_DQ}
    "vreducess",                           //  {AVX512_DQ}
    "vrndscalepd",                         //  {AVX512_F (VL)}
    "vrndscaleps",                         //  {AVX512_F (VL)}
    "vrndscalesd",                         //  {AVX512_F}
    "vrndscaless",                         //  {AVX512_F}
    "vrsqrt14pd",                          //  {AVX512_F (VL)}
    "vrsqrt14ps",                          //  {AVX512_F (VL)}
    "vrsqrt14sd",                          //  {AVX512_F}
    "vrsqrt14ss",                          //  {AVX512_F}
    "vrsqrt28pd",                          //  {AVX512_ERI}
    "vrsqrt28ps",                          //  {AVX512_ERI}
    "vrsqrt28sd",                          //  {AVX512_ERI}
    "vrsqrt28ss",                          //  {AVX512_ERI}
    "vscalefpd",                           //  {AVX512_F (VL)}
    "vscalefps",                           //  {AVX512_F (VL)}
    "vscalefsd",                           //  {AVX512_F}
    "vscalefss",                           //  {AVX512_F}
    "vscatterdpd",                         //  {AVX512_F (VL)}
    "vscatterdps",                         //  {AVX512_F (VL)}
    "vscatterpf0dpd",                      //  {AVX512_PFI}
    "vscatterpf0dps",                      //  {AVX512_PFI}
    "vscatterpf0qpd",                      //  {AVX512_PFI}
    "vscatterpf0qps",                      //  {AVX512_PFI}
    "vscatterpf1dpd",                      //  {AVX512_PFI}
    "vscatterpf1dps",                      //  {AVX512_PFI}
    "vscatterpf1qpd",                      //  {AVX512_PFI}
    "vscatterpf1qps",                      //  {AVX512_PFI}
    "vscatterqpd",                         //  {AVX512_F (VL)}
    "vscatterqps",                         //  {AVX512_F (VL)}
    "vshuff32x4",                          //  {AVX512_F (VL)}
    "vshuff64x2",                          //  {AVX512_F (VL)}
    "vshufi32x4",                          //  {AVX512_F (VL)}
    "vshufi64x2"                           //  {AVX512_F (VL)}
};

static char setfma3 [NUMFMA3] [20] = {
    "vfmadd132pd",                            //  {AVX512_F|FMA (VL)}
    "vfmadd132ps",                            //  {AVX512_F|FMA (VL)}
    "vfmadd132sd",                            //  {AVX512_F|FMA}
    "vfmadd132ss",                            //  {AVX512_F|FMA}
    "vfmadd213pd",                            //  {AVX512_F|FMA (VL)}
    "vfmadd213ps",                            //  {AVX512_F|FMA (VL)}
    "vfmadd213sd",                            //  {AVX512_F|FMA}
    "vfmadd213ss",                            //  {AVX512_F|FMA}
    "vfmadd231pd",                            //  {AVX512_F|FMA (VL)}
    "vfmadd231ps",                            //  {AVX512_F|FMA (VL)}
    "vfmadd231sd",                            //  {AVX512_F|FMA}
    "vfmadd231ss",                            //  {AVX512_F|FMA}
    "vfmaddsub132pd",                         //  {AVX512_F|FMA (VL)}
    "vfmaddsub132ps",                         //  {AVX512_F|FMA (VL)}
    "vfmaddsub213pd",                         //  {AVX512_F|FMA (VL)}
    "vfmaddsub213ps",                         //  {AVX512_F|FMA (VL)}
    "vfmaddsub231pd",                         //  {AVX512_F|FMA (VL)}
    "vfmaddsub231ps",                         //  {AVX512_F|FMA (VL)}
    "vfmsub132pd",                            //  {AVX512_F|FMA (VL)}
    "vfmsub132ps",                            //  {AVX512_F|FMA (VL)}
    "vfmsub132sd",                            //  {AVX512_F|FMA}
    "vfmsub132ss",                            //  {AVX512_F|FMA}
    "vfmsub213pd",                            //  {AVX512_F|FMA (VL)}
    "vfmsub213ps",                            //  {AVX512_F|FMA (VL)}
    "vfmsub213sd",                            //  {AVX512_F|FMA}
    "vfmsub213ss",                            //  {AVX512_F|FMA}
    "vfmsub231pd",                            //  {AVX512_F|FMA (VL)}
    "vfmsub231ps",                            //  {AVX512_F|FMA (VL)}
    "vfmsub231sd",                            //  {AVX512_F|FMA}
    "vfmsub231ss",                            //  {AVX512_F|FMA}
    "vfmsubadd132pd",                         //  {AVX512_F|FMA (VL)}
    "vfmsubadd132ps",                         //  {AVX512_F|FMA (VL)}
    "vfmsubadd213pd",                         //  {AVX512_F|FMA (VL)}
    "vfmsubadd213ps",                         //  {AVX512_F|FMA (VL)}
    "vfmsubadd231pd",                         //  {AVX512_F|FMA (VL)}
    "vfmsubadd231ps",                         //  {AVX512_F|FMA (VL)}
    "vfnmadd132pd",                           //  {AVX512_F|FMA (VL)}
    "vfnmadd132ps",                           //  {AVX512_F|FMA (VL)}
    "vfnmadd132sd",                           //  {AVX512_F|FMA}
    "vfnmadd132ss",                           //  {AVX512_F|FMA}
    "vfnmadd213pd",                           //  {AVX512_F|FMA (VL)}
    "vfnmadd213ps",                           //  {AVX512_F|FMA (VL)}
    "vfnmadd213sd",                           //  {AVX512_F|FMA}
    "vfnmadd213ss",                           //  {AVX512_F|FMA}
    "vfnmadd231pd",                           //  {AVX512_F|FMA (VL)}
    "vfnmadd231ps",                           //  {AVX512_F|FMA (VL)}
    "vfnmadd231sd",                           //  {AVX512_F|FMA}
    "vfnmadd231ss",                           //  {AVX512_F|FMA}
    "vfnmsub132pd",                           //  {AVX512_F|FMA (VL)}
    "vfnmsub132ps",                           //  {AVX512_F|FMA (VL)}
    "vfnmsub132sd",                           //  {AVX512_F|FMA}
    "vfnmsub132ss",                           //  {AVX512_F|FMA}
    "vfnmsub213pd",                           //  {AVX512_F|FMA (VL)}
    "vfnmsub213ps",                           //  {AVX512_F|FMA (VL)}
    "vfnmsub213sd",                           //  {AVX512_F|FMA}
    "vfnmsub213ss",                           //  {AVX512_F|FMA}
    "vfnmsub231pd",                           //  {AVX512_F|FMA (VL)}
    "vfnmsub231ps",                           //  {AVX512_F|FMA (VL)}
    "vfnmsub231sd",                           //  {AVX512_F|FMA}
    "vfnmsub231ss"                            //  {AVX512_F|FMA}
};

static char setfma4 [NUMFMA4] [20] = {
    "vfmaddpd",                            //  {FMA4}
    "vfmaddps",                            //  {FMA4}
    "vfmaddsd",                            //  {FMA4}
    "vfmaddss",                            //  {FMA4}
    "vfmaddsubpd",                         //  {FMA4}
    "vfmaddsubps",                         //  {FMA4}
    "vfmsubaddpd",                         //  {FMA4}
    "vfmsubaddps",                         //  {FMA4}
    "vfmsubpd",                            //  {FMA4}
    "vfmsubps",                            //  {FMA4}
    "vfmsubsd",                            //  {FMA4}
    "vfmsubss",                            //  {FMA4}
    "vfnmaddpd",                           //  {FMA4}
    "vfnmaddps",                           //  {FMA4}
    "vfnmaddsd",                           //  {FMA4}
    "vfnmaddss",                           //  {FMA4}
    "vfnmsubpd",                           //  {FMA4}
    "vfnmsubps",                           //  {FMA4}
    "vfnmsubsd",                           //  {FMA4}
    "vfnmsubss"                            //  {FMA4}
};

static char setxop [NUMXOP] [24] = {
// From http://sourceware.org/cgi-bin/cvsweb.cgi/~checkout~/src/opcodes/i386-opc.tbl?&cvsroot=src
    "vfrczpd",
    "vfrczpd",
    "vfrczps",
    "vfrczps",
    "vfrczsd",
    "vfrczss",
    "vpcmov",
    "vpcmov",
    "vpcmov",
    "vpcmov",
    "vpcomb",
    "vpcomd",
    "vpcomq",
    "vpcomub",
    "vpcomud",
    "vpcomuq",
    "vpcomuw",
    "vpcomw",
    "vpermil2pd",
    "vpermil2pd",
    "vpermil2pd",
    "vpermil2pd",
    "vpermil2ps",
    "vpermil2ps",
    "vpermil2ps",
    "vpermil2ps",
    "vpcomltb",
    "vpcomltd",
    "vpcomltq",
    "vpcomltub",
    "vpcomltud",
    "vpcomltuq",
    "vpcomltuw",
    "vpcomltw",
    "vpcomleb",
    "vpcomled",
    "vpcomleq",
    "vpcomleub",
    "vpcomleud",
    "vpcomleuq",
    "vpcomleuw",
    "vpcomlew",
    "vpcomgtb",
    "vpcomgtd",
    "vpcomgtq",
    "vpcomgtub",
    "vpcomgtud",
    "vpcomgtuq",
    "vpcomgtuw",
    "vpcomgtw",
    "vpcomgeb",
    "vpcomged",
    "vpcomgeq",
    "vpcomgeub",
    "vpcomgeud",
    "vpcomgeuq",
    "vpcomgeuw",
    "vpcomgew",
    "vpcomeqb",
    "vpcomeqd",
    "vpcomeqq",
    "vpcomequb",
    "vpcomequd",
    "vpcomequq",
    "vpcomequw",
    "vpcomeqw",
    "vpcomneqb",
    "vpcomneqd",
    "vpcomneqq",
    "vpcomnequb",
    "vpcomnequd",
    "vpcomnequq",
    "vpcomnequw",
    "vpcomneqw",
    "vpcomfalseb",
    "vpcomfalsed",
    "vpcomfalseq",
    "vpcomfalseub",
    "vpcomfalseud",
    "vpcomfalseuq",
    "vpcomfalseuw",
    "vpcomfalsew",
    "vpcomtrueb",
    "vpcomtrued",
    "vpcomtrueq",
    "vpcomtrueub",
    "vpcomtrueud",
    "vpcomtrueuq",
    "vpcomtrueuw",
    "vpcomtruew",
    "vphaddbd",
    "vphaddbq",
    "vphaddbw",
    "vphadddq",
    "vphaddubd",
    "vphaddubq",
    "vphaddubw",
    "vphaddudq",
    "vphadduwd",
    "vphadduwq",
    "vphaddwd",
    "vphaddwq",
    "vphsubbw",
    "vphsubdq",
    "vphsubwd",
    "vpmacsdd",
    "vpmacsdqh",
    "vpmacsdql",
    "vpmacssdd",
    "vpmacssdqh",
    "vpmacssdql",
    "vpmacsswd",
    "vpmacssww",
    "vpmacswd",
    "vpmacsww",
    "vpmadcsswd",
    "vpmadcswd",
    "vpperm",
    "vpperm",
    "vprotb",
    "vprotb",
    "vprotb",
    "vprotd",
    "vprotd",
    "vprotd",
    "vprotq",
    "vprotq",
    "vprotq",
    "vprotw",
    "vprotw",
    "vprotw",
    "vpshab",
    "vpshab",
    "vpshad",
    "vpshad",
    "vpshaq",
    "vpshaq",
    "vpshaw",
    "vpshaw",
    "vpshlb",
    "vpshlb",
    "vpshld",
    "vpshld",
    "vpshlq",
    "vpshlq",
    "vpshlw",
    "vpshlw"
};

static char setaes [NUMAES] [16] = {
    "aesdec",                          //  {AES}
    "aesdeclast",                      //  {AES}
    "aesenc",                          //  {AES}
    "aesenclast",                      //  {AES}
    "aesimc",                          //  {AES}
    "aeskeygenassist",                 //  {AES}
};

static char setvtx [NUMVTX] [16] = {
    "vmptrld",
    "vmptrst",
    "vmclear",
    "vmread",
    "vmwrite",
    "vmcall",
    "vmlaunch",
    "vmresume",
    "vmxoff",
    "vmxon"
};

static char setsvm [NUMSVM] [12] = {
    "invlpga",
    "clgi",
    "skinit",
    "stgi",
    "vmload",
    "vmmcall",
    "vmrun",
    "vmsave"
};
/* remark: xop cvt16 sse5-amd */


int main(int argc, char** argv)
{   char tmp[8192];
    char *e, *s, *x;
    int ret = 0;
    char itmp[256];
    FILE *f;
    int i, ok;
    long i486=0, i586=0, i686=0, immx=0, isse=0,
         isse2=0, isse3=0, issse3=0, isse41=0,
         isse42=0, isse4a=0, i3dnow=0, i3dnowext=0,
         cpuid=0, nop=0, call=0, count=0, movbecnt=0,
         xsavecnt=0, clmul=0, avx=0, avx2=0, avx512=0,
         fma3=0, fma4=0, vtx=0, svm=0, aes=0, xop=0;


    if (argc != 2) { printf("Syntax: %s <binary>\n", argv[0]); return (-1); }
    snprintf(tmp, 8191, "objdump -d %s", argv[1]);
    f = popen(tmp, "r");
    if (!f) { printf("%s\n", "objdump on argument failed."); return (-1); }

    while ( fgets ( tmp , 8191, f ) ) {
        e = strtok (tmp, "\t");
        if (e) { e = strtok (NULL, "\t");
           if (e) {
           e = strtok (NULL, "\t"); /* 3rd column contains instruction */
           if (!e) continue;
           snprintf(itmp, 255, "%s", e);
           s = strtok (itmp, " "); /* formatting as spaces */
             if (s) { x = s; /* clean, in case instruction is without args, i.e. nop */
                     while(*x) { if (*x == '\n' || *x == '\r')  *x = 0; else x++; }

             /* compare and count */
            ok = 0;
              count++;
              if (!strcmp(s, "cpuid")) { cpuid++; ok = 1; }
              if (!strcmp(s, "nop")) { nop++; ok = 1; }
              if (!strcmp(s, "call")) { call++; ok = 1; }
              /* Intel Atom */
              if (!strcmp(s, "movbe")) { movbecnt++; ok = 1; }
              /* Westmere */
              if (!strcmp(s, "pclmulqdq")) { clmul++; ok = 1; }
              /* Nehalem */
              if (!strcmp(s, "xsave")) { xsavecnt++; ok = 1; }
              if (!strcmp(s, "xrstor")) { xsavecnt++; ok = 1; }

            if (!ok)
            for (i = 0; i < NUM686; i++)
                if (!strcmp(s, set686[i])) { ok = 1; i686++ ; break; }

            if (!ok)
            for (i = 0; i < NUMMMX; i++)
                if (!strcmp(s, setmmx[i])) { ok = 1; immx++ ; break; }

            if (!ok)
            for (i = 0; i < NUMSSE; i++)
                if (!strcmp(s, setsse[i])) { ok = 1; isse++ ; break; }

            if (!ok)
            for (i = 0; i < NUMSSE2; i++)
                if (!strcmp(s, setsse2[i])) { ok = 1; isse2++ ; break; }

            if (!ok)
            for (i = 0; i < NUM486; i++)
               if (!strcmp(s, set486[i])) { ok = 1 ; i486++ ; break; }

            if (!ok)
            for (i = 0; i < NUM586; i++)
                if (!strcmp(s, set586[i])) { ok = 1; i586++ ; break; }

            if (!ok)
            for (i = 0; i < NUMSSE3; i++)
                if (!strcmp(s, setsse3[i])) { ok = 1; isse3++ ; break; }

            if (!ok)
            for (i = 0; i < NUMSSSE3; i++)
                if (!strcmp(s, setssse3[i])) { ok = 1; issse3++ ; break; }

            if (!ok)
            for (i = 0; i < NUMSSE41; i++)
                if (!strcmp(s, setsse41[i])) { ok = 1; isse41++ ; break; }

            if (!ok)
            for (i = 0; i < NUMSSE42; i++)
                if (!strcmp(s, setsse42[i])) { ok = 1; isse42++ ; break; }

            if (!ok)
            for (i = 0; i < NUMSSE4a; i++)
                if (!strcmp(s, setsse4a[i])) { ok = 1; isse4a++ ; break; }

            if (!ok)
            for (i = 0; i < NUM3DNOW; i++)
                if (!strcmp(s, set3dnow[i])) { ok = 1; i3dnow++ ; break; }

            if (!ok)
            for (i = 0; i < NUM3DNOWEXT; i++)
                if (!strcmp(s, set3dnowext[i])) { ok = 1; i3dnowext++ ;  break; }

            if (!ok)
            for (i = 0; i < NUMAVX; i++) 
                if (!strcmp(s, setavx[i])) { ok = 1; avx++ ;
#ifdef SHOW_AVX
                fprintf(stderr,"AVXi: %s\n", s);
#endif
                }
             if (!ok)
            for (i = 0; i < NUMAVX2; i++) 
                if (!strcmp(s, setavx2[i])) { ok = 1; avx2++ ;
#ifdef SHOW_AVX2
                fprintf(stderr,"AVX2i: %s\n", s);
#endif
                }
             if (!ok)
            for (i = 0; i < NUMAVX512; i++) 
                if (!strcmp(s, setavx512[i])) { ok = 1; avx512++ ;
#ifdef SHOW_AVX512
                fprintf(stderr,"AVX512i: %s\n", s);
#endif
                }
            if (!ok)
            for (i = 0; i < NUMAES; i++)
                if (!strcmp(s, setaes[i])) { ok = 1; aes++ ; break; }
            if (!ok)
            for (i = 0; i < NUMFMA3; i++)
                if (!strcmp(s, setfma3[i])) { ok = 1; fma3++ ; break; }
            if (!ok)
            for (i = 0; i < NUMFMA4; i++)
                if (!strcmp(s, setfma4[i])) { ok = 1; fma4++ ; break; }
            if (!ok)
            for (i = 0; i < NUMXOP; i++)
                if (!strcmp(s, setxop[i])) { ok = 1; xop++ ; break; }
            if (!ok)
            for (i = 0; i < NUMVTX; i++)
                if (!strcmp(s, setvtx[i])) { ok = 1; vtx++ ; break; }
            if (!ok)
            for (i = 0; i < NUMSVM; i++)
                if (!strcmp(s, setsvm[i])) { ok = 1; svm++ ; break; }


                    } /* instruction */
                  } /* strtok2: hex */
               } /* strtok1: address */

                                       } /* end parse */
        /* close pipe */
        if (f) pclose(f);
/* print statistics */
printf("instructions:\n cpuid:\t %lu\t nop: %lu\t call: %lu\t count: %lu\n\n",cpuid,nop,call,count);
if (i486) printf(" i486:\t\t %lu\n", i486);
if (i586) printf(" i586:\t\t %lu\n", i586);
if (i686) printf(" i686:\t\t %lu\n", i686);
if (immx) printf(" mmx:\t\t %lu\n", immx);
if (i3dnow) printf(" 3dnow!:\t %lu\n", i3dnow);
if (i3dnowext) printf(" 3dnowext:\t %lu\n", i3dnowext);
if (isse) printf(" sse:\t\t %lu\n", isse);
if (isse2) printf(" sse2:\t\t %lu\n", isse2);
if (isse3) printf(" sse3:\t\t %lu\n", isse3);
if (issse3) printf(" ssse3:\t\t %lu\n", issse3);
if (isse4a || isse41 || isse42) {
if (isse41) printf(" sse4.1:\t %lu\n", isse41);
if (isse42) printf(" sse4.2:\t %lu\n", isse42);
if (isse4a) printf(" sse4a:\t\t %lu\n", isse4a);
printf("%s\n" , "");            }
if (fma3 || fma4)    
{
    if (fma3) printf( " FMA/FMA3:\t %lu\n", fma3);
    if (fma4) printf( " FMA4:\t %lu\n", fma4);
}
if (vtx || svm)    {
    if (vtx) printf( " Intel VT-x: %lu  ", vtx);
    if (svm) printf( " AMD SVM: %lu  ", svm);
printf("%s\n" , ""); }
if (aes || clmul)    
{
    printf("%s\n","Westmere:");
    if (aes) printf( " AES:\t %lu\n", aes);
    if (clmul) printf( " CLMUL:\t %lu\n", clmul);
}
if (avx) printf( " AVX:\t\t %lu\n", avx); 
if (avx2) printf( " AVX2:\t\t %lu\n", avx2);
if (avx512) printf( " AVX-512:\t %lu\n", avx512);
if (xop) printf( " XOP:\t %lu\n", xop);
if (movbecnt) printf("movbe:\t %lu\n", movbecnt);
if (xsavecnt) printf("Nehalem XSAVE/XRSTOR:\t %lu\n", xsavecnt);

return 0;
}    
