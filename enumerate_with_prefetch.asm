f(std::span<unsigned int, 18446744073709551615ul>, std::span<std::pair<double, double>, 18446744073709551615ul>):
        push    rbp
        vpxor   xmm0, xmm0, xmm0
        mov     rbp, rsp
        and     rsp, -32 # stack pointer 32 bit aligned
        sub     rsp, 136
        vmovdqa YMMWORD PTR [rsp-120], ymm0 # zeroes 32 * 4 bytes below the stack pointer
        vmovdqa YMMWORD PTR [rsp-88], ymm0
        vmovdqa YMMWORD PTR [rsp-56], ymm0
        vmovdqa YMMWORD PTR [rsp-24], ymm0
        vmovdqa YMMWORD PTR [rsp+8], ymm0 # zeroes 32 * 4 bytes above the stack pointer
        vmovdqa YMMWORD PTR [rsp+40], ymm0
        vmovdqa YMMWORD PTR [rsp+72], ymm0
        vmovdqa YMMWORD PTR [rsp+104], ymm0
        cmp     rsi, 16 # compares second arg with 16 i.e. the end ptr of sorted_indeces
        jbe     .L7
        vxorpd  xmm1, xmm1, xmm1
        vmovsd  xmm13, QWORD PTR .LC1[rip]
        vmovsd  xmm9, QWORD PTR .LC2[rip]
        mov     r8, rdx
        vmovsd  xmm10, QWORD PTR .LC3[rip]
        vmovsd  xmm11, QWORD PTR .LC4[rip]
        xor     ecx, ecx
        xor     edx, edx
        vmovsd  xmm12, xmm1, xmm1
        vmovsd  xmm8, xmm1, xmm1
        lea     r9, [rsp+136]
.L6:
        mov     eax, ecx
        lea     r10, [rsp-120]
        sal     rax, 4
        add     rax, r8
        # fetches data from derivatives array
        vmovupd ymm7, YMMWORD PTR [rax+32]
        vmovupd ymm6, YMMWORD PTR [rax+64]
        vmovupd ymm5, YMMWORD PTR [rax+96]
        vmovupd ymm4, YMMWORD PTR [rax+128]
        vmovupd ymm3, YMMWORD PTR [rax+160]
        vmovupd ymm2, YMMWORD PTR [rax+192]
        # migrates them to the buffer
        vmovapd YMMWORD PTR [rsp-88], ymm7
        vmovupd ymm0, YMMWORD PTR [rax+224]
        vmovupd ymm14, YMMWORD PTR [rax]
        vmovapd YMMWORD PTR [rsp-56], ymm6
        mov     rax, rdi
        vmovapd YMMWORD PTR [rsp-24], ymm5
        vmovapd YMMWORD PTR [rsp-120], ymm14
        vmovapd YMMWORD PTR [rsp+8], ymm4
        vmovapd YMMWORD PTR [rsp+40], ymm3
        vmovapd YMMWORD PTR [rsp+72], ymm2
        vmovapd YMMWORD PTR [rsp+104], ymm0
.L5:
        vmovsd  xmm0, QWORD PTR [r10]
        vaddsd  xmm8, xmm8, xmm0
        vsubsd  xmm9, xmm9, xmm0
        vmovsd  xmm0, QWORD PTR [r10+8]
        vaddsd  xmm12, xmm12, xmm0
        vsubsd  xmm13, xmm13, xmm0
        vmulsd  xmm0, xmm8, xmm8
        vaddsd  xmm2, xmm12, xmm10
        vaddsd  xmm3, xmm13, xmm10
        vdivsd  xmm0, xmm0, xmm2
        vmulsd  xmm2, xmm9, xmm9
        vdivsd  xmm2, xmm2, xmm3
        vaddsd  xmm0, xmm0, xmm2
        vsubsd  xmm0, xmm0, xmm11
        vcomisd xmm1, xmm0
        jbe     .L3
        mov     edx, DWORD PTR [rax]
        vmovsd  xmm1, xmm0, xmm0
.L3:
        add     r10, 16
        add     rax, 4
        cmp     r9, r10
        jne     .L5
        lea     eax, [rcx+32]
        add     ecx, 16
        cmp     rax, rsi
        jb      .L6
        vmovsd  xmm0, xmm1, xmm1
        vzeroupper
        mov     rax, rdx
        leave
        ret
.L7:
        vxorpd  xmm1, xmm1, xmm1
        xor     edx, edx
        vmovsd  xmm0, xmm1, xmm1
        vzeroupper
        mov     rax, rdx
        leave
        ret
.LC1:
        .long   0
        .long   1088421888
.LC2:
        .long   0
        .long   1083179008
.LC3:
        .long   0
        .long   1071644672
.LC4:
        .long   2097120
        .long   1077936096