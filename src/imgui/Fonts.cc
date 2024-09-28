// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/Fonts.hh"
#include "imgui.h"
#include "ui/FontSize.hh"

// File: 'fonts/vivict-icons.ttf' (4416 bytes)
// Exported using binary_to_compressed_c.cpp
static const char vivictIconsTtf_compressed_data_base85[3240 + 1] =
    "7])#######uw5U7'/###[),##1xL$#Q6>##e@;*>Oca6MajFJ(]?'o/"
    "fY;99)M7%#>(m<->9Bk0'Tx-3Z8aIg7+:GD=RopAqV,-GB7h<G'Pf%u;'6kC<gXG-%eU`N5i4/"
    "CD6@?$%J%/G"
    "T_Ae?-x9hLc<SS%,5LsC?gSqU0rpWR4JET@b';9CIl:@P$<^ooU2cD4AJr9.L;7#"
    "5IENwKE3n0F3Q[aao8_w'XSk--Y2thFwev6f*L$F7%$A310iR/"
    "Gf=#>P-$D2:v]NA-qmnUCXc2Yr"
    "wAr/qV?kc2@C=GHI]F_ub$L-MI_T_M=_-##tqgR$/"
    "he<6(7I<$VQv&#0#Pe4`gWj0:(wp<VHK`3^WTD34h2W%WgJe$uIbA#p&9H3^UAS@-uY<-v@g;-"
    ":eU&O%FPGMdh8%#2Tl##xeK^#"
    "$,>>#sn&W74xD_&UG:;$d?%;Q(2###83?Seg&%#Q+x9B#CcdC#:UiX-ZpJe$CCbh#2H^e$'"
    "Wxd<3P_-#0vX.q?\?m9M1Z(se.#,F@Fw:'#D7?>#IVkr->I,F%tNj-$H,n.LawZL#e^2U2"
    ",E^2#2G8U)=MR&,Z2c'&LDMq2)Eu'&&ES$9qE>X(lD,U;,,`2q4)>>#BDn+M?3S>-Viq@-*"
    "uKj%Qu68%47no%8ONP&<h/2'@*gi'DBGJ(HZ(,)7`###^XB`NJZdY#%5YY#6+KcM$/MU7"
    "FFRv-ZLP<hGCg+M%.@<A#cr@bO00)3XcZxbiqt(N8I$Atwl'#G(Vk(s()&#YXkZY#E4$##"
    "XsHYY-l68%Y86Q#Af2;]r9T1ph6=W$G(w2$)rus-[B1,M7C>,Mp`7K%d5%V6rd=R*sNAX-"
    "JgnO($w5/(XpLp#D]-&.6W&##S*w,v]D<h#^48]%*L(,)[1pu,jNm4(JDJ1(*34:.N.OF32T/"
    "i);-n)4_TIg)'vsI330GKl&vfUlV=Xt/YDc?Lq*sqK1E(C&sntq)IfCAF6Ao&0]S:wL"
    "aew[b2hUHmK8F#$0=N/)*jeA#j,j'Js`mV7f(<>Z?bRm'W47AO[F?YY%'/"
    "S@-%lA#qE#W-eJSqDX8WlAZFEGVKWv^fZq/"
    "bIQWr,2?ZdA4Zun$$kGUv-*)@e2^^D.3[*;G#ek.tuNPPA4"
    "vDU-2+@Ks$O?9q%Y^Mig810LrCBZL%ho+?,C$L2'wY03&YSc6<'l>AOcR&%#PGSB.:=]:/"
    "8_EN(a&SF4#+VaNePblA;'MhL6iqwM?4Uw/MW3##dD24'7_UAO/n?]Owbgs7U4[wBNF_Y,"
    "L%A-dnAf4(8kI9i56$&4Rx)pAb@wWh7?*XC:/"
    "1RB62oQ-`*m<-C1nT-X+u_.CV$>6v['o-B6&njd(jqLW=WuPM&co7w']3kNxko7+Xj;7%"
    "tUhLI?_o-nxqfHE<.+Ie5w>Omr78%wtNN0"
    "npB'#*?$(#xYCu7VUF&#KUi;-9:_S-(QaU-i<1A49va.3vbLs-;:k=.g-C(4Z:=V/"
    "OF.KN;OTO1VQ(K(>#1/"
    "D#_=U:Im;g(kO)Zu'no>8PAL],U#<AHgX1g#%P(]b&Zvr6FkCC+Kdk;."
    "8#[3Drau#-$w%*+Ih*=188/`GqJNb,R3&[#sSbEN0'co7D.-nXI_^MC=iI8AlQ%W%3W:n/"
    "8&*)#Z=#+#v>jW/&Ae8/+gB.*@uRv$a&SF4g'T,2b@'c3#jWI):m?A4NMbL)aX&<.rnjp%"
    "]AK41WMU$-g]'/)1@U/)R3>j'H*Rq768)<-]jI1(i>I=/93S^550N`+UK<1)fMik'C$5/"
    "(#8;'#_W'd%w3GJ(Wo8>,F7$##Q(tD#sj5/(#?P8/:vx<*+[RW#9TE4O<V?/pTlJ1VW/.84"
    "pK?S4'VU?h>Yk7/"
    "B+]l]Pv%&Ma_e&MKP2V?<aaJs'Zg8%:'[E,E;^`(EEN-<_qMQmlx;GD3pgfLPq$&/"
    "_k,841Z?U)PDgkLH-$YYODoHHa22Fn3ALS.U<t(3fvUO'x[R_#BCI>#vrHR/"
    "*A':Od`77a6hQC#.>.h(2k`%%U5i0(>xUv#'RV.M[rWu5]gBQ&rTY@$?UuUZBlG##vX-lF2+"
    "IP/V<O/)F[N@PZ&<[*>te=-Xqr-$]]/JHTbOQ#HYgF*%dg@M3(H<-lS8N/8B.;6AZ;DN"
    "G;2,)A+A7/CH7f3SK<#PwD>^*P(#@5i0s9r_Ql)MaCE/8+Nr%cxk[/"
    "MuI,O2o@tA#mmAr-`$P-Q<%DdFMo'M99_(a4A9'W$?MqW-%Tw7N01LhL?@,**R(T]uUf[I-"
    "RqWC&RZYc2:`R]4"
    "FRKV6F:7n8p$G&#YLe9.BiMB#%p)9.XVi4'@u2x-%8eUO?wuB6@om[-CNk*?vDmuJWB[K)Dp%$"
    "U)2Y)=E'2-MNEL5:]6mkbLN4E*P9mA#ZXfR0HDa&TFDXB#7PSfL'x+sQ=jqE'%JM)4"
    "u&1N-'ibM-F)<M-4*Y(0HVs)#knl+#Q-k91XMQ59N0Hv$4%E[0n&[gLt%iQ&?N]B5S_<V."
    "OxRm0h<49%simv$C_j5&2A4X%A]>%,(^k.D`fbI3-)Au.9J$98.+ZC+L1;?#hM@1(0[1hL"
    "eLG8.5=S5#e>Ys-O%M.NS@-##we/"
    "4NTF6##3Qi6NTI?##'MDF5N@$##4uU?#Y3xU.81r?#wrG<->9X7/"
    "G6>##_RFgLYV$lL0=`T.<_R%#64xU./`($#QA`T.wZP+#dA`T.@hn@#3@`T."
    "G6OA#VrG<-Lc*c3nc/*#Xst.#e)1/#I<x-#U5C/#x>0,.2_jjL/u@(#0Q?(#Zp/"
    "kLeR#.#9sarL4%C-#2-BP8DX=gG&>.FHMeZ<0-K*.3`c:/D7TA>BWc'vHu/"
    "v-$UBt<1SuD_&p:H]F"
    "N7Fh50L)pD0&VEH7M#hF;jB]OH@)=-7$lZ-x$5F%ov7kOYU9XLx`[9MJ;NY5KF2K?o`^"
    "G3pGfM1I,kM1=ow?^_amdFgkqdF-4%F-)G@@-4Q[=LaJ,H36whO1-qL&#WlwfDR:ta]Xd7JC"
    "8Z?*e0w$LYW=rKGRpYc2iFHL-fB5-2Q:1eG38vLF*<eh273IL2B?I&#OLIW-r(M3bkE8<-w49;"
    "-w*ST%Q8YY#*J(v#.c_V$2%@8%6=wo%:UWP&>n82'B0pi'FHPJ(Ja1,)x':oDFb66N"
    "Whv-NXn).NYt2.NZ$<.N[*E.N]0N.N^6W.N_<a.N`Bj.Nhs]/Ni#g/Nj)p/Nk/"
    "#0Nl5,0Nl2pjMWaQL2Wa&g206@#v%25##3.g,MU-<$#i(ofLAw5l$6rXR*s-ZDkQD>5#]h$'l*"
    "C;:#"
    "YcGxk_Z3`$Pwq8Kn####";

static const ImWchar icons_ranges[] = {0xf000, 0xf011, 0};

static ImFont *iconFont;

ImFont *vivictpp::imgui::getIconFont() { return iconFont; }

void vivictpp::imgui::initFonts() {
  ImGuiIO &io = ImGui::GetIO();
  ImFontConfig fontConfig;
  fontConfig.PixelSnapH = true;
  fontConfig.SizePixels = vivictpp::ui::FontSize(13).scaledSizeFloat();
  io.Fonts->AddFontDefault(&fontConfig);

  iconFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
      vivictIconsTtf_compressed_data_base85,
      vivictpp::ui::FontSize(40.0).scaledSizeFloat(), nullptr, icons_ranges);
  //   io.Fonts->Build();
}
