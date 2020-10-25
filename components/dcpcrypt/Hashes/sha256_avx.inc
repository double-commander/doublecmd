{
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (c) 2012, Intel Corporation
;
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are
; met:
;
; * Redistributions of source code must retain the above copyright
;   notice, this list of conditions and the following disclaimer.
;
; * Redistributions in binary form must reproduce the above copyright
;   notice, this list of conditions and the following disclaimer in the
;   documentation and/or other materials provided with the
;   distribution.
;
; * Neither the name of the Intel Corporation nor the names of its
;   contributors may be used to endorse or promote products derived from
;   this software without specific prior written permission.
;
;
; THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION "AS IS" AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
; PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL CORPORATION OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
; PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
; LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
; NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; This code is described in an Intel White-Paper:
; "Fast SHA-256 Implementations on Intel Architecture Processors"
;
; To find it, surf to http://www.intel.com/p/en_US/embedded
; and search for that title.
; The paper is expected to be released roughly at the end of April, 2012
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This code schedules 2 blocks at a time, with 4 lanes per block
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*/
/*
 * Conversion to GAS assembly and integration to libgcrypt
 *  by Jussi Kivilinna <jussi.kivilinna@iki.fi>
 */
}

procedure sha256_compress_avx(CurrentHash: PLongWord; HashBuffer: PByte; BufferCount: UIntPtr); assembler; nostackframe;
// UNIX    RDI, RSI, RDX
// WIN64:  RCX, RDX, R8
asm
{$IF DEFINED(WIN64)}
  movq   %rsi, 16(%rsp)
  movq   %rdi, 24(%rsp)
  subq   $168, %rsp
  vmovaps %xmm6, (%rsp)
  vmovaps %xmm7, 16(%rsp)
  vmovaps %xmm8, 32(%rsp)
  vmovaps %xmm9, 48(%rsp)
  vmovaps %xmm10, 64(%rsp)
  vmovaps %xmm11, 80(%rsp)
  vmovaps %xmm12, 96(%rsp)
  vmovaps %xmm13, 112(%rsp)
  vmovaps %xmm14, 128(%rsp)
  vmovaps %xmm15, 144(%rsp)
  movq   %rcx, %rdi
  movq   %rdx, %rsi
  movq   %r8,  %rdx
{$ENDIF}
  xchg   %rdi, %rsi
  xor    %eax,%eax
  cmp    $0x0,%rdx
  je     .LB163f
  push   %rbx
  push   %rbp
  push   %r12
  push   %r13
  push   %r14
  push   %r15
  vzeroupper
  vmovdqa .LPSHUFFLE_BYTE_FLIP_MASK(%rip),%ymm13
  vmovdqa .L_SHUF_00BA(%rip),%ymm10
  vmovdqa .L_SHUF_DC00(%rip),%ymm12
  mov    %rsp,%rax
  sub    $0x220,%rsp
  and    $0xffffffffffffffc0,%rsp
  mov    %rax,0x218(%rsp)
  shl    $0x6,%rdx
  lea    -0x40(%rdx,%rdi,1),%rdx
  mov    %rdx,0x200(%rsp)
  mov    (%rsi),%eax
  mov    0x4(%rsi),%ebx
  mov    0x8(%rsi),%ecx
  mov    0xc(%rsi),%r8d
  mov    0x10(%rsi),%edx
  mov    0x14(%rsi),%r9d
  mov    0x18(%rsi),%r10d
  mov    0x1c(%rsi),%r11d
  mov    %rsi,0x210(%rsp)

.LB7b:
  lea    .LK256(%rip),%rbp
  vmovdqu (%rdi),%ymm0
  vmovdqu 0x20(%rdi),%ymm1
  vmovdqu 0x40(%rdi),%ymm2
  vmovdqu 0x60(%rdi),%ymm3
  vpshufb %ymm13,%ymm0,%ymm0
  vpshufb %ymm13,%ymm1,%ymm1
  vpshufb %ymm13,%ymm2,%ymm2
  vpshufb %ymm13,%ymm3,%ymm3
  vperm2i128 $0x20,%ymm2,%ymm0,%ymm4
  vperm2i128 $0x31,%ymm2,%ymm0,%ymm5
  vperm2i128 $0x20,%ymm3,%ymm1,%ymm6
  vperm2i128 $0x31,%ymm3,%ymm1,%ymm7

.LBc1:
  add    $0x40,%rdi
  mov    %rdi,0x208(%rsp)
  xor    %rsi,%rsi
  vpaddd 0x0(%rbp),%ymm4,%ymm9
  vmovdqa %ymm9,(%rsp)
  vpaddd 0x20(%rbp),%ymm5,%ymm9
  vmovdqa %ymm9,0x20(%rsp)
  vpaddd 0x40(%rbp),%ymm6,%ymm9
  vmovdqa %ymm9,0x40(%rsp)
  vpaddd 0x60(%rbp),%ymm7,%ymm9
  vmovdqa %ymm9,0x60(%rsp)
  // nopl   0x0(%rax,%rax,1)
  .byte 0x0f,0x1f,0x44,0x00,0x00

.LB100:
  vpalignr $0x4,%ymm6,%ymm7,%ymm0
  vpaddd %ymm4,%ymm0,%ymm0
  vpalignr $0x4,%ymm4,%ymm5,%ymm1
  vpsrld $0x7,%ymm1,%ymm2
  vpslld $0x19,%ymm1,%ymm3
  vpor   %ymm2,%ymm3,%ymm3
  vpsrld $0x12,%ymm1,%ymm2
  mov    %edx,%edi
  add    (%rsp,%rsi,1),%r11d
  and    %r9d,%edi
  rorx   $0x19,%edx,%r13d
  rorx   $0xb,%edx,%r14d
  lea    (%r11d,%edi,1),%r11d
  andn   %r10d,%edx,%edi
  rorx   $0xd,%eax,%r12d
  xor    %r14d,%r13d
  lea    (%r11d,%edi,1),%r11d
  rorx   $0x16,%eax,%r15d
  rorx   $0x6,%edx,%r14d
  mov    %eax,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ebx,%edi
  lea    (%r11d,%r13d,1),%r11d
  mov    %eax,%r13d
  rorx   $0x2,%eax,%r15d
  add    %r11d,%r8d
  and    %ecx,%edi
  xor    %r15d,%r12d
  lea    (%r11d,%edi,1),%r11d
  lea    (%r11d,%r12d,1),%r11d
  and    %ebx,%r13d
  lea    (%r11d,%r13d,1),%r11d
  vpsrld $0x3,%ymm1,%ymm8
  vpslld $0xe,%ymm1,%ymm1
  vpxor  %ymm1,%ymm3,%ymm3
  vpxor  %ymm2,%ymm3,%ymm3
  vpxor  %ymm8,%ymm3,%ymm1
  vpshufd $0xfa,%ymm7,%ymm2
  vpaddd %ymm1,%ymm0,%ymm0
  vpsrld $0xa,%ymm2,%ymm8
  mov    %r8d,%edi
  add    0x4(%rsp,%rsi,1),%r10d
  and    %edx,%edi
  rorx   $0x19,%r8d,%r13d
  rorx   $0xb,%r8d,%r14d
  lea    (%r10d,%edi,1),%r10d
  andn   %r9d,%r8d,%edi
  rorx   $0xd,%r11d,%r12d
  xor    %r14d,%r13d
  lea    (%r10d,%edi,1),%r10d
  rorx   $0x16,%r11d,%r15d
  rorx   $0x6,%r8d,%r14d
  mov    %r11d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %eax,%edi
  lea    (%r10d,%r13d,1),%r10d
  mov    %r11d,%r13d
  rorx   $0x2,%r11d,%r15d
  add    %r10d,%ecx
  and    %ebx,%edi
  xor    %r15d,%r12d
  lea    (%r10d,%edi,1),%r10d
  lea    (%r10d,%r12d,1),%r10d
  and    %eax,%r13d
  lea    (%r10d,%r13d,1),%r10d
  vpsrlq $0x13,%ymm2,%ymm3
  vpsrlq $0x11,%ymm2,%ymm2
  vpxor  %ymm3,%ymm2,%ymm2
  vpxor  %ymm2,%ymm8,%ymm8
  vpshufb %ymm10,%ymm8,%ymm8
  vpaddd %ymm8,%ymm0,%ymm0
  vpshufd $0x50,%ymm0,%ymm2
  mov    %ecx,%edi
  add    0x8(%rsp,%rsi,1),%r9d
  and    %r8d,%edi
  rorx   $0x19,%ecx,%r13d
  rorx   $0xb,%ecx,%r14d
  lea    (%r9d,%edi,1),%r9d
  andn   %edx,%ecx,%edi
  rorx   $0xd,%r10d,%r12d
  xor    %r14d,%r13d
  lea    (%r9d,%edi,1),%r9d
  rorx   $0x16,%r10d,%r15d
  rorx   $0x6,%ecx,%r14d
  mov    %r10d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r11d,%edi
  lea    (%r9d,%r13d,1),%r9d
  mov    %r10d,%r13d
  rorx   $0x2,%r10d,%r15d
  add    %r9d,%ebx
  and    %eax,%edi
  xor    %r15d,%r12d
  lea    (%r9d,%edi,1),%r9d
  lea    (%r9d,%r12d,1),%r9d
  and    %r11d,%r13d
  lea    (%r9d,%r13d,1),%r9d
  vpsrld $0xa,%ymm2,%ymm11
  vpsrlq $0x13,%ymm2,%ymm3
  vpsrlq $0x11,%ymm2,%ymm2
  vpxor  %ymm3,%ymm2,%ymm2
  vpxor  %ymm2,%ymm11,%ymm11
  vpshufb %ymm12,%ymm11,%ymm11
  vpaddd %ymm0,%ymm11,%ymm4
  vpaddd 0x80(%rbp,%rsi,1),%ymm4,%ymm9
  mov    %ebx,%edi
  add    0xc(%rsp,%rsi,1),%edx
  and    %ecx,%edi
  rorx   $0x19,%ebx,%r13d
  rorx   $0xb,%ebx,%r14d
  lea    (%edx,%edi,1),%edx
  andn   %r8d,%ebx,%edi
  rorx   $0xd,%r9d,%r12d
  xor    %r14d,%r13d
  lea    (%edx,%edi,1),%edx
  vmovdqa %ymm9,0x80(%rsp,%rsi,1)
  rorx   $0x16,%r9d,%r15d
  rorx   $0x6,%ebx,%r14d
  mov    %r9d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r10d,%edi
  lea    (%edx,%r13d,1),%edx
  mov    %r9d,%r13d
  rorx   $0x2,%r9d,%r15d
  add    %edx,%eax
  and    %r11d,%edi
  xor    %r15d,%r12d
  lea    (%edx,%edi,1),%edx
  lea    (%edx,%r12d,1),%edx
  and    %r10d,%r13d
  lea    (%edx,%r13d,1),%edx
  vpalignr $0x4,%ymm7,%ymm4,%ymm0
  vpaddd %ymm5,%ymm0,%ymm0
  vpalignr $0x4,%ymm5,%ymm6,%ymm1
  vpsrld $0x7,%ymm1,%ymm2
  vpslld $0x19,%ymm1,%ymm3
  vpor   %ymm2,%ymm3,%ymm3
  vpsrld $0x12,%ymm1,%ymm2
  mov    %eax,%edi
  add    0x20(%rsp,%rsi,1),%r8d
  and    %ebx,%edi
  rorx   $0x19,%eax,%r13d
  rorx   $0xb,%eax,%r14d
  lea    (%r8d,%edi,1),%r8d
  andn   %ecx,%eax,%edi
  rorx   $0xd,%edx,%r12d
  xor    %r14d,%r13d
  lea    (%r8d,%edi,1),%r8d
  rorx   $0x16,%edx,%r15d
  rorx   $0x6,%eax,%r14d
  mov    %edx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r9d,%edi
  lea    (%r8d,%r13d,1),%r8d
  mov    %edx,%r13d
  rorx   $0x2,%edx,%r15d
  add    %r8d,%r11d
  and    %r10d,%edi
  xor    %r15d,%r12d
  lea    (%r8d,%edi,1),%r8d
  lea    (%r8d,%r12d,1),%r8d
  and    %r9d,%r13d
  lea    (%r8d,%r13d,1),%r8d
  vpsrld $0x3,%ymm1,%ymm8
  vpslld $0xe,%ymm1,%ymm1
  vpxor  %ymm1,%ymm3,%ymm3
  vpxor  %ymm2,%ymm3,%ymm3
  vpxor  %ymm8,%ymm3,%ymm1
  vpshufd $0xfa,%ymm4,%ymm2
  vpaddd %ymm1,%ymm0,%ymm0
  vpsrld $0xa,%ymm2,%ymm8
  mov    %r11d,%edi
  add    0x24(%rsp,%rsi,1),%ecx
  and    %eax,%edi
  rorx   $0x19,%r11d,%r13d
  rorx   $0xb,%r11d,%r14d
  lea    (%ecx,%edi,1),%ecx
  andn   %ebx,%r11d,%edi
  rorx   $0xd,%r8d,%r12d
  xor    %r14d,%r13d
  lea    (%ecx,%edi,1),%ecx
  rorx   $0x16,%r8d,%r15d
  rorx   $0x6,%r11d,%r14d
  mov    %r8d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %edx,%edi
  lea    (%ecx,%r13d,1),%ecx
  mov    %r8d,%r13d
  rorx   $0x2,%r8d,%r15d
  add    %ecx,%r10d
  and    %r9d,%edi
  xor    %r15d,%r12d
  lea    (%ecx,%edi,1),%ecx
  lea    (%ecx,%r12d,1),%ecx
  and    %edx,%r13d
  lea    (%ecx,%r13d,1),%ecx
  vpsrlq $0x13,%ymm2,%ymm3
  vpsrlq $0x11,%ymm2,%ymm2
  vpxor  %ymm3,%ymm2,%ymm2
  vpxor  %ymm2,%ymm8,%ymm8
  vpshufb %ymm10,%ymm8,%ymm8
  vpaddd %ymm8,%ymm0,%ymm0
  vpshufd $0x50,%ymm0,%ymm2
  mov    %r10d,%edi
  add    0x28(%rsp,%rsi,1),%ebx
  and    %r11d,%edi
  rorx   $0x19,%r10d,%r13d
  rorx   $0xb,%r10d,%r14d
  lea    (%ebx,%edi,1),%ebx
  andn   %eax,%r10d,%edi
  rorx   $0xd,%ecx,%r12d
  xor    %r14d,%r13d
  lea    (%ebx,%edi,1),%ebx
  rorx   $0x16,%ecx,%r15d
  rorx   $0x6,%r10d,%r14d
  mov    %ecx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r8d,%edi
  lea    (%ebx,%r13d,1),%ebx
  mov    %ecx,%r13d
  rorx   $0x2,%ecx,%r15d
  add    %ebx,%r9d
  and    %edx,%edi
  xor    %r15d,%r12d
  lea    (%ebx,%edi,1),%ebx
  lea    (%ebx,%r12d,1),%ebx
  and    %r8d,%r13d
  lea    (%ebx,%r13d,1),%ebx
  vpsrld $0xa,%ymm2,%ymm11
  vpsrlq $0x13,%ymm2,%ymm3
  vpsrlq $0x11,%ymm2,%ymm2
  vpxor  %ymm3,%ymm2,%ymm2
  vpxor  %ymm2,%ymm11,%ymm11
  vpshufb %ymm12,%ymm11,%ymm11
  vpaddd %ymm0,%ymm11,%ymm5
  vpaddd 0xa0(%rbp,%rsi,1),%ymm5,%ymm9
  mov    %r9d,%edi
  add    0x2c(%rsp,%rsi,1),%eax
  and    %r10d,%edi
  rorx   $0x19,%r9d,%r13d
  rorx   $0xb,%r9d,%r14d
  lea    (%eax,%edi,1),%eax
  andn   %r11d,%r9d,%edi
  rorx   $0xd,%ebx,%r12d
  xor    %r14d,%r13d
  lea    (%eax,%edi,1),%eax
  vmovdqa %ymm9,0xa0(%rsp,%rsi,1)
  rorx   $0x16,%ebx,%r15d
  rorx   $0x6,%r9d,%r14d
  mov    %ebx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ecx,%edi
  lea    (%eax,%r13d,1),%eax
  mov    %ebx,%r13d
  rorx   $0x2,%ebx,%r15d
  add    %eax,%edx
  and    %r8d,%edi
  xor    %r15d,%r12d
  lea    (%eax,%edi,1),%eax
  lea    (%eax,%r12d,1),%eax
  and    %ecx,%r13d
  lea    (%eax,%r13d,1),%eax
  vpalignr $0x4,%ymm4,%ymm5,%ymm0
  vpaddd %ymm6,%ymm0,%ymm0
  vpalignr $0x4,%ymm6,%ymm7,%ymm1
  vpsrld $0x7,%ymm1,%ymm2
  vpslld $0x19,%ymm1,%ymm3
  vpor   %ymm2,%ymm3,%ymm3
  vpsrld $0x12,%ymm1,%ymm2
  mov    %edx,%edi
  add    0x40(%rsp,%rsi,1),%r11d
  and    %r9d,%edi
  rorx   $0x19,%edx,%r13d
  rorx   $0xb,%edx,%r14d
  lea    (%r11d,%edi,1),%r11d
  andn   %r10d,%edx,%edi
  rorx   $0xd,%eax,%r12d
  xor    %r14d,%r13d
  lea    (%r11d,%edi,1),%r11d
  rorx   $0x16,%eax,%r15d
  rorx   $0x6,%edx,%r14d
  mov    %eax,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ebx,%edi
  lea    (%r11d,%r13d,1),%r11d
  mov    %eax,%r13d
  rorx   $0x2,%eax,%r15d
  add    %r11d,%r8d
  and    %ecx,%edi
  xor    %r15d,%r12d
  lea    (%r11d,%edi,1),%r11d
  lea    (%r11d,%r12d,1),%r11d
  and    %ebx,%r13d
  lea    (%r11d,%r13d,1),%r11d
  vpsrld $0x3,%ymm1,%ymm8
  vpslld $0xe,%ymm1,%ymm1
  vpxor  %ymm1,%ymm3,%ymm3
  vpxor  %ymm2,%ymm3,%ymm3
  vpxor  %ymm8,%ymm3,%ymm1
  vpshufd $0xfa,%ymm5,%ymm2
  vpaddd %ymm1,%ymm0,%ymm0
  vpsrld $0xa,%ymm2,%ymm8
  mov    %r8d,%edi
  add    0x44(%rsp,%rsi,1),%r10d
  and    %edx,%edi
  rorx   $0x19,%r8d,%r13d
  rorx   $0xb,%r8d,%r14d
  lea    (%r10d,%edi,1),%r10d
  andn   %r9d,%r8d,%edi
  rorx   $0xd,%r11d,%r12d
  xor    %r14d,%r13d
  lea    (%r10d,%edi,1),%r10d
  rorx   $0x16,%r11d,%r15d
  rorx   $0x6,%r8d,%r14d
  mov    %r11d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %eax,%edi
  lea    (%r10d,%r13d,1),%r10d
  mov    %r11d,%r13d
  rorx   $0x2,%r11d,%r15d
  add    %r10d,%ecx
  and    %ebx,%edi
  xor    %r15d,%r12d
  lea    (%r10d,%edi,1),%r10d
  lea    (%r10d,%r12d,1),%r10d
  and    %eax,%r13d
  lea    (%r10d,%r13d,1),%r10d
  vpsrlq $0x13,%ymm2,%ymm3
  vpsrlq $0x11,%ymm2,%ymm2
  vpxor  %ymm3,%ymm2,%ymm2
  vpxor  %ymm2,%ymm8,%ymm8
  vpshufb %ymm10,%ymm8,%ymm8
  vpaddd %ymm8,%ymm0,%ymm0
  vpshufd $0x50,%ymm0,%ymm2
  mov    %ecx,%edi
  add    0x48(%rsp,%rsi,1),%r9d
  and    %r8d,%edi
  rorx   $0x19,%ecx,%r13d
  rorx   $0xb,%ecx,%r14d
  lea    (%r9d,%edi,1),%r9d
  andn   %edx,%ecx,%edi
  rorx   $0xd,%r10d,%r12d
  xor    %r14d,%r13d
  lea    (%r9d,%edi,1),%r9d
  rorx   $0x16,%r10d,%r15d
  rorx   $0x6,%ecx,%r14d
  mov    %r10d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r11d,%edi
  lea    (%r9d,%r13d,1),%r9d
  mov    %r10d,%r13d
  rorx   $0x2,%r10d,%r15d
  add    %r9d,%ebx
  and    %eax,%edi
  xor    %r15d,%r12d
  lea    (%r9d,%edi,1),%r9d
  lea    (%r9d,%r12d,1),%r9d
  and    %r11d,%r13d
  lea    (%r9d,%r13d,1),%r9d
  vpsrld $0xa,%ymm2,%ymm11
  vpsrlq $0x13,%ymm2,%ymm3
  vpsrlq $0x11,%ymm2,%ymm2
  vpxor  %ymm3,%ymm2,%ymm2
  vpxor  %ymm2,%ymm11,%ymm11
  vpshufb %ymm12,%ymm11,%ymm11
  vpaddd %ymm0,%ymm11,%ymm6
  vpaddd 0xc0(%rbp,%rsi,1),%ymm6,%ymm9
  mov    %ebx,%edi
  add    0x4c(%rsp,%rsi,1),%edx
  and    %ecx,%edi
  rorx   $0x19,%ebx,%r13d
  rorx   $0xb,%ebx,%r14d
  lea    (%edx,%edi,1),%edx
  andn   %r8d,%ebx,%edi
  rorx   $0xd,%r9d,%r12d
  xor    %r14d,%r13d
  lea    (%edx,%edi,1),%edx
  vmovdqa %ymm9,0xc0(%rsp,%rsi,1)
  rorx   $0x16,%r9d,%r15d
  rorx   $0x6,%ebx,%r14d
  mov    %r9d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r10d,%edi
  lea    (%edx,%r13d,1),%edx
  mov    %r9d,%r13d
  rorx   $0x2,%r9d,%r15d
  add    %edx,%eax
  and    %r11d,%edi
  xor    %r15d,%r12d
  lea    (%edx,%edi,1),%edx
  lea    (%edx,%r12d,1),%edx
  and    %r10d,%r13d
  lea    (%edx,%r13d,1),%edx
  vpalignr $0x4,%ymm5,%ymm6,%ymm0
  vpaddd %ymm7,%ymm0,%ymm0
  vpalignr $0x4,%ymm7,%ymm4,%ymm1
  vpsrld $0x7,%ymm1,%ymm2
  vpslld $0x19,%ymm1,%ymm3
  vpor   %ymm2,%ymm3,%ymm3
  vpsrld $0x12,%ymm1,%ymm2
  mov    %eax,%edi
  add    0x60(%rsp,%rsi,1),%r8d
  and    %ebx,%edi
  rorx   $0x19,%eax,%r13d
  rorx   $0xb,%eax,%r14d
  lea    (%r8d,%edi,1),%r8d
  andn   %ecx,%eax,%edi
  rorx   $0xd,%edx,%r12d
  xor    %r14d,%r13d
  lea    (%r8d,%edi,1),%r8d
  rorx   $0x16,%edx,%r15d
  rorx   $0x6,%eax,%r14d
  mov    %edx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r9d,%edi
  lea    (%r8d,%r13d,1),%r8d
  mov    %edx,%r13d
  rorx   $0x2,%edx,%r15d
  add    %r8d,%r11d
  and    %r10d,%edi
  xor    %r15d,%r12d
  lea    (%r8d,%edi,1),%r8d
  lea    (%r8d,%r12d,1),%r8d
  and    %r9d,%r13d
  lea    (%r8d,%r13d,1),%r8d
  vpsrld $0x3,%ymm1,%ymm8
  vpslld $0xe,%ymm1,%ymm1
  vpxor  %ymm1,%ymm3,%ymm3
  vpxor  %ymm2,%ymm3,%ymm3
  vpxor  %ymm8,%ymm3,%ymm1
  vpshufd $0xfa,%ymm6,%ymm2
  vpaddd %ymm1,%ymm0,%ymm0
  vpsrld $0xa,%ymm2,%ymm8
  mov    %r11d,%edi
  add    0x64(%rsp,%rsi,1),%ecx
  and    %eax,%edi
  rorx   $0x19,%r11d,%r13d
  rorx   $0xb,%r11d,%r14d
  lea    (%ecx,%edi,1),%ecx
  andn   %ebx,%r11d,%edi
  rorx   $0xd,%r8d,%r12d
  xor    %r14d,%r13d
  lea    (%ecx,%edi,1),%ecx
  rorx   $0x16,%r8d,%r15d
  rorx   $0x6,%r11d,%r14d
  mov    %r8d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %edx,%edi
  lea    (%ecx,%r13d,1),%ecx
  mov    %r8d,%r13d
  rorx   $0x2,%r8d,%r15d
  add    %ecx,%r10d
  and    %r9d,%edi
  xor    %r15d,%r12d
  lea    (%ecx,%edi,1),%ecx
  lea    (%ecx,%r12d,1),%ecx
  and    %edx,%r13d
  lea    (%ecx,%r13d,1),%ecx
  vpsrlq $0x13,%ymm2,%ymm3
  vpsrlq $0x11,%ymm2,%ymm2
  vpxor  %ymm3,%ymm2,%ymm2
  vpxor  %ymm2,%ymm8,%ymm8
  vpshufb %ymm10,%ymm8,%ymm8
  vpaddd %ymm8,%ymm0,%ymm0
  vpshufd $0x50,%ymm0,%ymm2
  mov    %r10d,%edi
  add    0x68(%rsp,%rsi,1),%ebx
  and    %r11d,%edi
  rorx   $0x19,%r10d,%r13d
  rorx   $0xb,%r10d,%r14d
  lea    (%ebx,%edi,1),%ebx
  andn   %eax,%r10d,%edi
  rorx   $0xd,%ecx,%r12d
  xor    %r14d,%r13d
  lea    (%ebx,%edi,1),%ebx
  rorx   $0x16,%ecx,%r15d
  rorx   $0x6,%r10d,%r14d
  mov    %ecx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r8d,%edi
  lea    (%ebx,%r13d,1),%ebx
  mov    %ecx,%r13d
  rorx   $0x2,%ecx,%r15d
  add    %ebx,%r9d
  and    %edx,%edi
  xor    %r15d,%r12d
  lea    (%ebx,%edi,1),%ebx
  lea    (%ebx,%r12d,1),%ebx
  and    %r8d,%r13d
  lea    (%ebx,%r13d,1),%ebx
  vpsrld $0xa,%ymm2,%ymm11
  vpsrlq $0x13,%ymm2,%ymm3
  vpsrlq $0x11,%ymm2,%ymm2
  vpxor  %ymm3,%ymm2,%ymm2
  vpxor  %ymm2,%ymm11,%ymm11
  vpshufb %ymm12,%ymm11,%ymm11
  vpaddd %ymm0,%ymm11,%ymm7
  vpaddd 0xe0(%rbp,%rsi,1),%ymm7,%ymm9
  mov    %r9d,%edi
  add    0x6c(%rsp,%rsi,1),%eax
  and    %r10d,%edi
  rorx   $0x19,%r9d,%r13d
  rorx   $0xb,%r9d,%r14d
  lea    (%eax,%edi,1),%eax
  andn   %r11d,%r9d,%edi
  rorx   $0xd,%ebx,%r12d
  xor    %r14d,%r13d
  lea    (%eax,%edi,1),%eax
  vmovdqa %ymm9,0xe0(%rsp,%rsi,1)
  rorx   $0x16,%ebx,%r15d
  rorx   $0x6,%r9d,%r14d
  mov    %ebx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ecx,%edi
  lea    (%eax,%r13d,1),%eax
  mov    %ebx,%r13d
  rorx   $0x2,%ebx,%r15d
  add    %eax,%edx
  and    %r8d,%edi
  xor    %r15d,%r12d
  lea    (%eax,%edi,1),%eax
  lea    (%eax,%r12d,1),%eax
  and    %ecx,%r13d
  lea    (%eax,%r13d,1),%eax
  add    $0x80,%rsi
  cmp    $0x180,%rsi
  jb     .LB100
  mov    %edx,%edi
  add    0x180(%rsp),%r11d
  and    %r9d,%edi
  rorx   $0x19,%edx,%r13d
  rorx   $0xb,%edx,%r14d
  lea    (%r11d,%edi,1),%r11d
  andn   %r10d,%edx,%edi
  rorx   $0xd,%eax,%r12d
  xor    %r14d,%r13d
  lea    (%r11d,%edi,1),%r11d
  rorx   $0x16,%eax,%r15d
  rorx   $0x6,%edx,%r14d
  mov    %eax,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ebx,%edi
  lea    (%r11d,%r13d,1),%r11d
  mov    %eax,%r13d
  rorx   $0x2,%eax,%r15d
  add    %r11d,%r8d
  and    %ecx,%edi
  xor    %r15d,%r12d
  lea    (%r11d,%edi,1),%r11d
  lea    (%r11d,%r12d,1),%r11d
  and    %ebx,%r13d
  lea    (%r11d,%r13d,1),%r11d
  mov    %r8d,%edi
  add    0x184(%rsp),%r10d
  and    %edx,%edi
  rorx   $0x19,%r8d,%r13d
  rorx   $0xb,%r8d,%r14d
  lea    (%r10d,%edi,1),%r10d
  andn   %r9d,%r8d,%edi
  rorx   $0xd,%r11d,%r12d
  xor    %r14d,%r13d
  lea    (%r10d,%edi,1),%r10d
  rorx   $0x16,%r11d,%r15d
  rorx   $0x6,%r8d,%r14d
  mov    %r11d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %eax,%edi
  lea    (%r10d,%r13d,1),%r10d
  mov    %r11d,%r13d
  rorx   $0x2,%r11d,%r15d
  add    %r10d,%ecx
  and    %ebx,%edi
  xor    %r15d,%r12d
  lea    (%r10d,%edi,1),%r10d
  lea    (%r10d,%r12d,1),%r10d
  and    %eax,%r13d
  lea    (%r10d,%r13d,1),%r10d
  mov    %ecx,%edi
  add    0x188(%rsp),%r9d
  and    %r8d,%edi
  rorx   $0x19,%ecx,%r13d
  rorx   $0xb,%ecx,%r14d
  lea    (%r9d,%edi,1),%r9d
  andn   %edx,%ecx,%edi
  rorx   $0xd,%r10d,%r12d
  xor    %r14d,%r13d
  lea    (%r9d,%edi,1),%r9d
  rorx   $0x16,%r10d,%r15d
  rorx   $0x6,%ecx,%r14d
  mov    %r10d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r11d,%edi
  lea    (%r9d,%r13d,1),%r9d
  mov    %r10d,%r13d
  rorx   $0x2,%r10d,%r15d
  add    %r9d,%ebx
  and    %eax,%edi
  xor    %r15d,%r12d
  lea    (%r9d,%edi,1),%r9d
  lea    (%r9d,%r12d,1),%r9d
  and    %r11d,%r13d
  lea    (%r9d,%r13d,1),%r9d
  mov    %ebx,%edi
  add    0x18c(%rsp),%edx
  and    %ecx,%edi
  rorx   $0x19,%ebx,%r13d
  rorx   $0xb,%ebx,%r14d
  lea    (%edx,%edi,1),%edx
  andn   %r8d,%ebx,%edi
  rorx   $0xd,%r9d,%r12d
  xor    %r14d,%r13d
  lea    (%edx,%edi,1),%edx
  rorx   $0x16,%r9d,%r15d
  rorx   $0x6,%ebx,%r14d
  mov    %r9d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r10d,%edi
  lea    (%edx,%r13d,1),%edx
  mov    %r9d,%r13d
  rorx   $0x2,%r9d,%r15d
  add    %edx,%eax
  and    %r11d,%edi
  xor    %r15d,%r12d
  lea    (%edx,%edi,1),%edx
  lea    (%edx,%r12d,1),%edx
  and    %r10d,%r13d
  lea    (%edx,%r13d,1),%edx
  mov    %eax,%edi
  add    0x1a0(%rsp),%r8d
  and    %ebx,%edi
  rorx   $0x19,%eax,%r13d
  rorx   $0xb,%eax,%r14d
  lea    (%r8d,%edi,1),%r8d
  andn   %ecx,%eax,%edi
  rorx   $0xd,%edx,%r12d
  xor    %r14d,%r13d
  lea    (%r8d,%edi,1),%r8d
  rorx   $0x16,%edx,%r15d
  rorx   $0x6,%eax,%r14d
  mov    %edx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r9d,%edi
  lea    (%r8d,%r13d,1),%r8d
  mov    %edx,%r13d
  rorx   $0x2,%edx,%r15d
  add    %r8d,%r11d
  and    %r10d,%edi
  xor    %r15d,%r12d
  lea    (%r8d,%edi,1),%r8d
  lea    (%r8d,%r12d,1),%r8d
  and    %r9d,%r13d
  lea    (%r8d,%r13d,1),%r8d
  mov    %r11d,%edi
  add    0x1a4(%rsp),%ecx
  and    %eax,%edi
  rorx   $0x19,%r11d,%r13d
  rorx   $0xb,%r11d,%r14d
  lea    (%ecx,%edi,1),%ecx
  andn   %ebx,%r11d,%edi
  rorx   $0xd,%r8d,%r12d
  xor    %r14d,%r13d
  lea    (%ecx,%edi,1),%ecx
  rorx   $0x16,%r8d,%r15d
  rorx   $0x6,%r11d,%r14d
  mov    %r8d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %edx,%edi
  lea    (%ecx,%r13d,1),%ecx
  mov    %r8d,%r13d
  rorx   $0x2,%r8d,%r15d
  add    %ecx,%r10d
  and    %r9d,%edi
  xor    %r15d,%r12d
  lea    (%ecx,%edi,1),%ecx
  lea    (%ecx,%r12d,1),%ecx
  and    %edx,%r13d
  lea    (%ecx,%r13d,1),%ecx
  mov    %r10d,%edi
  add    0x1a8(%rsp),%ebx
  and    %r11d,%edi
  rorx   $0x19,%r10d,%r13d
  rorx   $0xb,%r10d,%r14d
  lea    (%ebx,%edi,1),%ebx
  andn   %eax,%r10d,%edi
  rorx   $0xd,%ecx,%r12d
  xor    %r14d,%r13d
  lea    (%ebx,%edi,1),%ebx
  rorx   $0x16,%ecx,%r15d
  rorx   $0x6,%r10d,%r14d
  mov    %ecx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r8d,%edi
  lea    (%ebx,%r13d,1),%ebx
  mov    %ecx,%r13d
  rorx   $0x2,%ecx,%r15d
  add    %ebx,%r9d
  and    %edx,%edi
  xor    %r15d,%r12d
  lea    (%ebx,%edi,1),%ebx
  lea    (%ebx,%r12d,1),%ebx
  and    %r8d,%r13d
  lea    (%ebx,%r13d,1),%ebx
  mov    %r9d,%edi
  add    0x1ac(%rsp),%eax
  and    %r10d,%edi
  rorx   $0x19,%r9d,%r13d
  rorx   $0xb,%r9d,%r14d
  lea    (%eax,%edi,1),%eax
  andn   %r11d,%r9d,%edi
  rorx   $0xd,%ebx,%r12d
  xor    %r14d,%r13d
  lea    (%eax,%edi,1),%eax
  rorx   $0x16,%ebx,%r15d
  rorx   $0x6,%r9d,%r14d
  mov    %ebx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ecx,%edi
  lea    (%eax,%r13d,1),%eax
  mov    %ebx,%r13d
  rorx   $0x2,%ebx,%r15d
  add    %eax,%edx
  and    %r8d,%edi
  xor    %r15d,%r12d
  lea    (%eax,%edi,1),%eax
  lea    (%eax,%r12d,1),%eax
  and    %ecx,%r13d
  lea    (%eax,%r13d,1),%eax
  mov    %edx,%edi
  add    0x1c0(%rsp),%r11d
  and    %r9d,%edi
  rorx   $0x19,%edx,%r13d
  rorx   $0xb,%edx,%r14d
  lea    (%r11d,%edi,1),%r11d
  andn   %r10d,%edx,%edi
  rorx   $0xd,%eax,%r12d
  xor    %r14d,%r13d
  lea    (%r11d,%edi,1),%r11d
  rorx   $0x16,%eax,%r15d
  rorx   $0x6,%edx,%r14d
  mov    %eax,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ebx,%edi
  lea    (%r11d,%r13d,1),%r11d
  mov    %eax,%r13d
  rorx   $0x2,%eax,%r15d
  add    %r11d,%r8d
  and    %ecx,%edi
  xor    %r15d,%r12d
  lea    (%r11d,%edi,1),%r11d
  lea    (%r11d,%r12d,1),%r11d
  and    %ebx,%r13d
  lea    (%r11d,%r13d,1),%r11d
  mov    %r8d,%edi
  add    0x1c4(%rsp),%r10d
  and    %edx,%edi
  rorx   $0x19,%r8d,%r13d
  rorx   $0xb,%r8d,%r14d
  lea    (%r10d,%edi,1),%r10d
  andn   %r9d,%r8d,%edi
  rorx   $0xd,%r11d,%r12d
  xor    %r14d,%r13d
  lea    (%r10d,%edi,1),%r10d
  rorx   $0x16,%r11d,%r15d
  rorx   $0x6,%r8d,%r14d
  mov    %r11d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %eax,%edi
  lea    (%r10d,%r13d,1),%r10d
  mov    %r11d,%r13d
  rorx   $0x2,%r11d,%r15d
  add    %r10d,%ecx
  and    %ebx,%edi
  xor    %r15d,%r12d
  lea    (%r10d,%edi,1),%r10d
  lea    (%r10d,%r12d,1),%r10d
  and    %eax,%r13d
  lea    (%r10d,%r13d,1),%r10d
  mov    %ecx,%edi
  add    0x1c8(%rsp),%r9d
  and    %r8d,%edi
  rorx   $0x19,%ecx,%r13d
  rorx   $0xb,%ecx,%r14d
  lea    (%r9d,%edi,1),%r9d
  andn   %edx,%ecx,%edi
  rorx   $0xd,%r10d,%r12d
  xor    %r14d,%r13d
  lea    (%r9d,%edi,1),%r9d
  rorx   $0x16,%r10d,%r15d
  rorx   $0x6,%ecx,%r14d
  mov    %r10d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r11d,%edi
  lea    (%r9d,%r13d,1),%r9d
  mov    %r10d,%r13d
  rorx   $0x2,%r10d,%r15d
  add    %r9d,%ebx
  and    %eax,%edi
  xor    %r15d,%r12d
  lea    (%r9d,%edi,1),%r9d
  lea    (%r9d,%r12d,1),%r9d
  and    %r11d,%r13d
  lea    (%r9d,%r13d,1),%r9d
  mov    %ebx,%edi
  add    0x1cc(%rsp),%edx
  and    %ecx,%edi
  rorx   $0x19,%ebx,%r13d
  rorx   $0xb,%ebx,%r14d
  lea    (%edx,%edi,1),%edx
  andn   %r8d,%ebx,%edi
  rorx   $0xd,%r9d,%r12d
  xor    %r14d,%r13d
  lea    (%edx,%edi,1),%edx
  rorx   $0x16,%r9d,%r15d
  rorx   $0x6,%ebx,%r14d
  mov    %r9d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r10d,%edi
  lea    (%edx,%r13d,1),%edx
  mov    %r9d,%r13d
  rorx   $0x2,%r9d,%r15d
  add    %edx,%eax
  and    %r11d,%edi
  xor    %r15d,%r12d
  lea    (%edx,%edi,1),%edx
  lea    (%edx,%r12d,1),%edx
  and    %r10d,%r13d
  lea    (%edx,%r13d,1),%edx
  mov    %eax,%edi
  add    0x1e0(%rsp),%r8d
  and    %ebx,%edi
  rorx   $0x19,%eax,%r13d
  rorx   $0xb,%eax,%r14d
  lea    (%r8d,%edi,1),%r8d
  andn   %ecx,%eax,%edi
  rorx   $0xd,%edx,%r12d
  xor    %r14d,%r13d
  lea    (%r8d,%edi,1),%r8d
  rorx   $0x16,%edx,%r15d
  rorx   $0x6,%eax,%r14d
  mov    %edx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r9d,%edi
  lea    (%r8d,%r13d,1),%r8d
  mov    %edx,%r13d
  rorx   $0x2,%edx,%r15d
  add    %r8d,%r11d
  and    %r10d,%edi
  xor    %r15d,%r12d
  lea    (%r8d,%edi,1),%r8d
  lea    (%r8d,%r12d,1),%r8d
  and    %r9d,%r13d
  lea    (%r8d,%r13d,1),%r8d
  mov    %r11d,%edi
  add    0x1e4(%rsp),%ecx
  and    %eax,%edi
  rorx   $0x19,%r11d,%r13d
  rorx   $0xb,%r11d,%r14d
  lea    (%ecx,%edi,1),%ecx
  andn   %ebx,%r11d,%edi
  rorx   $0xd,%r8d,%r12d
  xor    %r14d,%r13d
  lea    (%ecx,%edi,1),%ecx
  rorx   $0x16,%r8d,%r15d
  rorx   $0x6,%r11d,%r14d
  mov    %r8d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %edx,%edi
  lea    (%ecx,%r13d,1),%ecx
  mov    %r8d,%r13d
  rorx   $0x2,%r8d,%r15d
  add    %ecx,%r10d
  and    %r9d,%edi
  xor    %r15d,%r12d
  lea    (%ecx,%edi,1),%ecx
  lea    (%ecx,%r12d,1),%ecx
  and    %edx,%r13d
  lea    (%ecx,%r13d,1),%ecx
  mov    %r10d,%edi
  add    0x1e8(%rsp),%ebx
  and    %r11d,%edi
  rorx   $0x19,%r10d,%r13d
  rorx   $0xb,%r10d,%r14d
  lea    (%ebx,%edi,1),%ebx
  andn   %eax,%r10d,%edi
  rorx   $0xd,%ecx,%r12d
  xor    %r14d,%r13d
  lea    (%ebx,%edi,1),%ebx
  rorx   $0x16,%ecx,%r15d
  rorx   $0x6,%r10d,%r14d
  mov    %ecx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r8d,%edi
  lea    (%ebx,%r13d,1),%ebx
  mov    %ecx,%r13d
  rorx   $0x2,%ecx,%r15d
  add    %ebx,%r9d
  and    %edx,%edi
  xor    %r15d,%r12d
  lea    (%ebx,%edi,1),%ebx
  lea    (%ebx,%r12d,1),%ebx
  and    %r8d,%r13d
  lea    (%ebx,%r13d,1),%ebx
  mov    %r9d,%edi
  add    0x1ec(%rsp),%eax
  and    %r10d,%edi
  rorx   $0x19,%r9d,%r13d
  rorx   $0xb,%r9d,%r14d
  lea    (%eax,%edi,1),%eax
  andn   %r11d,%r9d,%edi
  rorx   $0xd,%ebx,%r12d
  xor    %r14d,%r13d
  lea    (%eax,%edi,1),%eax
  rorx   $0x16,%ebx,%r15d
  rorx   $0x6,%r9d,%r14d
  mov    %ebx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ecx,%edi
  lea    (%eax,%r13d,1),%eax
  mov    %ebx,%r13d
  rorx   $0x2,%ebx,%r15d
  add    %eax,%edx
  and    %r8d,%edi
  xor    %r15d,%r12d
  lea    (%eax,%edi,1),%eax
  lea    (%eax,%r12d,1),%eax
  and    %ecx,%r13d
  lea    (%eax,%r13d,1),%eax
  mov    0x210(%rsp),%rsi
  mov    0x208(%rsp),%rdi
  add    (%rsi),%eax
  mov    %eax,(%rsi)
  add    0x4(%rsi),%ebx
  mov    %ebx,0x4(%rsi)
  add    0x8(%rsi),%ecx
  mov    %ecx,0x8(%rsi)
  add    0xc(%rsi),%r8d
  mov    %r8d,0xc(%rsi)
  add    0x10(%rsi),%edx
  mov    %edx,0x10(%rsi)
  add    0x14(%rsi),%r9d
  mov    %r9d,0x14(%rsi)
  add    0x18(%rsi),%r10d
  mov    %r10d,0x18(%rsi)
  add    0x1c(%rsi),%r11d
  mov    %r11d,0x1c(%rsi)
  cmp    0x200(%rsp),%rdi
  ja     .LB15a5
  xor    %rsi,%rsi
  xchg   %ax,%ax

.LB1170:
  mov    %edx,%edi
  add    0x10(%rsp,%rsi,1),%r11d
  and    %r9d,%edi
  rorx   $0x19,%edx,%r13d
  rorx   $0xb,%edx,%r14d
  lea    (%r11d,%edi,1),%r11d
  andn   %r10d,%edx,%edi
  rorx   $0xd,%eax,%r12d
  xor    %r14d,%r13d
  lea    (%r11d,%edi,1),%r11d
  rorx   $0x16,%eax,%r15d
  rorx   $0x6,%edx,%r14d
  mov    %eax,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ebx,%edi
  lea    (%r11d,%r13d,1),%r11d
  mov    %eax,%r13d
  rorx   $0x2,%eax,%r15d
  add    %r11d,%r8d
  and    %ecx,%edi
  xor    %r15d,%r12d
  lea    (%r11d,%edi,1),%r11d
  lea    (%r11d,%r12d,1),%r11d
  and    %ebx,%r13d
  lea    (%r11d,%r13d,1),%r11d
  mov    %r8d,%edi
  add    0x14(%rsp,%rsi,1),%r10d
  and    %edx,%edi
  rorx   $0x19,%r8d,%r13d
  rorx   $0xb,%r8d,%r14d
  lea    (%r10d,%edi,1),%r10d
  andn   %r9d,%r8d,%edi
  rorx   $0xd,%r11d,%r12d
  xor    %r14d,%r13d
  lea    (%r10d,%edi,1),%r10d
  rorx   $0x16,%r11d,%r15d
  rorx   $0x6,%r8d,%r14d
  mov    %r11d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %eax,%edi
  lea    (%r10d,%r13d,1),%r10d
  mov    %r11d,%r13d
  rorx   $0x2,%r11d,%r15d
  add    %r10d,%ecx
  and    %ebx,%edi
  xor    %r15d,%r12d
  lea    (%r10d,%edi,1),%r10d
  lea    (%r10d,%r12d,1),%r10d
  and    %eax,%r13d
  lea    (%r10d,%r13d,1),%r10d
  mov    %ecx,%edi
  add    0x18(%rsp,%rsi,1),%r9d
  and    %r8d,%edi
  rorx   $0x19,%ecx,%r13d
  rorx   $0xb,%ecx,%r14d
  lea    (%r9d,%edi,1),%r9d
  andn   %edx,%ecx,%edi
  rorx   $0xd,%r10d,%r12d
  xor    %r14d,%r13d
  lea    (%r9d,%edi,1),%r9d
  rorx   $0x16,%r10d,%r15d
  rorx   $0x6,%ecx,%r14d
  mov    %r10d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r11d,%edi
  lea    (%r9d,%r13d,1),%r9d
  mov    %r10d,%r13d
  rorx   $0x2,%r10d,%r15d
  add    %r9d,%ebx
  and    %eax,%edi
  xor    %r15d,%r12d
  lea    (%r9d,%edi,1),%r9d
  lea    (%r9d,%r12d,1),%r9d
  and    %r11d,%r13d
  lea    (%r9d,%r13d,1),%r9d
  mov    %ebx,%edi
  add    0x1c(%rsp,%rsi,1),%edx
  and    %ecx,%edi
  rorx   $0x19,%ebx,%r13d
  rorx   $0xb,%ebx,%r14d
  lea    (%edx,%edi,1),%edx
  andn   %r8d,%ebx,%edi
  rorx   $0xd,%r9d,%r12d
  xor    %r14d,%r13d
  lea    (%edx,%edi,1),%edx
  rorx   $0x16,%r9d,%r15d
  rorx   $0x6,%ebx,%r14d
  mov    %r9d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r10d,%edi
  lea    (%edx,%r13d,1),%edx
  mov    %r9d,%r13d
  rorx   $0x2,%r9d,%r15d
  add    %edx,%eax
  and    %r11d,%edi
  xor    %r15d,%r12d
  lea    (%edx,%edi,1),%edx
  lea    (%edx,%r12d,1),%edx
  and    %r10d,%r13d
  lea    (%edx,%r13d,1),%edx
  mov    %eax,%edi
  add    0x30(%rsp,%rsi,1),%r8d
  and    %ebx,%edi
  rorx   $0x19,%eax,%r13d
  rorx   $0xb,%eax,%r14d
  lea    (%r8d,%edi,1),%r8d
  andn   %ecx,%eax,%edi
  rorx   $0xd,%edx,%r12d
  xor    %r14d,%r13d
  lea    (%r8d,%edi,1),%r8d
  rorx   $0x16,%edx,%r15d
  rorx   $0x6,%eax,%r14d
  mov    %edx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r9d,%edi
  lea    (%r8d,%r13d,1),%r8d
  mov    %edx,%r13d
  rorx   $0x2,%edx,%r15d
  add    %r8d,%r11d
  and    %r10d,%edi
  xor    %r15d,%r12d
  lea    (%r8d,%edi,1),%r8d
  lea    (%r8d,%r12d,1),%r8d
  and    %r9d,%r13d
  lea    (%r8d,%r13d,1),%r8d
  mov    %r11d,%edi
  add    0x34(%rsp,%rsi,1),%ecx
  and    %eax,%edi
  rorx   $0x19,%r11d,%r13d
  rorx   $0xb,%r11d,%r14d
  lea    (%ecx,%edi,1),%ecx
  andn   %ebx,%r11d,%edi
  rorx   $0xd,%r8d,%r12d
  xor    %r14d,%r13d
  lea    (%ecx,%edi,1),%ecx
  rorx   $0x16,%r8d,%r15d
  rorx   $0x6,%r11d,%r14d
  mov    %r8d,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %edx,%edi
  lea    (%ecx,%r13d,1),%ecx
  mov    %r8d,%r13d
  rorx   $0x2,%r8d,%r15d
  add    %ecx,%r10d
  and    %r9d,%edi
  xor    %r15d,%r12d
  lea    (%ecx,%edi,1),%ecx
  lea    (%ecx,%r12d,1),%ecx
  and    %edx,%r13d
  lea    (%ecx,%r13d,1),%ecx
  mov    %r10d,%edi
  add    0x38(%rsp,%rsi,1),%ebx
  and    %r11d,%edi
  rorx   $0x19,%r10d,%r13d
  rorx   $0xb,%r10d,%r14d
  lea    (%ebx,%edi,1),%ebx
  andn   %eax,%r10d,%edi
  rorx   $0xd,%ecx,%r12d
  xor    %r14d,%r13d
  lea    (%ebx,%edi,1),%ebx
  rorx   $0x16,%ecx,%r15d
  rorx   $0x6,%r10d,%r14d
  mov    %ecx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %r8d,%edi
  lea    (%ebx,%r13d,1),%ebx
  mov    %ecx,%r13d
  rorx   $0x2,%ecx,%r15d
  add    %ebx,%r9d
  and    %edx,%edi
  xor    %r15d,%r12d
  lea    (%ebx,%edi,1),%ebx
  lea    (%ebx,%r12d,1),%ebx
  and    %r8d,%r13d
  lea    (%ebx,%r13d,1),%ebx
  mov    %r9d,%edi
  add    0x3c(%rsp,%rsi,1),%eax
  and    %r10d,%edi
  rorx   $0x19,%r9d,%r13d
  rorx   $0xb,%r9d,%r14d
  lea    (%eax,%edi,1),%eax
  andn   %r11d,%r9d,%edi
  rorx   $0xd,%ebx,%r12d
  xor    %r14d,%r13d
  lea    (%eax,%edi,1),%eax
  rorx   $0x16,%ebx,%r15d
  rorx   $0x6,%r9d,%r14d
  mov    %ebx,%edi
  xor    %r15d,%r12d
  xor    %r14d,%r13d
  xor    %ecx,%edi
  lea    (%eax,%r13d,1),%eax
  mov    %ebx,%r13d
  rorx   $0x2,%ebx,%r15d
  add    %eax,%edx
  and    %r8d,%edi
  xor    %r15d,%r12d
  lea    (%eax,%edi,1),%eax
  lea    (%eax,%r12d,1),%eax
  and    %ecx,%r13d
  lea    (%eax,%r13d,1),%eax
  add    $0x40,%rsi
  cmp    $0x200,%rsi
  jb     .LB1170
  mov    0x210(%rsp),%rsi
  mov    0x208(%rsp),%rdi
  add    $0x40,%rdi
  add    (%rsi),%eax
  mov    %eax,(%rsi)
  add    0x4(%rsi),%ebx
  mov    %ebx,0x4(%rsi)
  add    0x8(%rsi),%ecx
  mov    %ecx,0x8(%rsi)
  add    0xc(%rsi),%r8d
  mov    %r8d,0xc(%rsi)
  add    0x10(%rsi),%edx
  mov    %edx,0x10(%rsi)
  add    0x14(%rsi),%r9d
  mov    %r9d,0x14(%rsi)
  add    0x18(%rsi),%r10d
  mov    %r10d,0x18(%rsi)
  add    0x1c(%rsi),%r11d
  mov    %r11d,0x1c(%rsi)
  cmp    0x200(%rsp),%rdi
  jb     .LB7b
  ja     .LB15a5

.LB1535:
  lea    .LK256(%rip),%rbp
  vmovdqu (%rdi),%xmm4
  vmovdqu 0x10(%rdi),%xmm5
  vmovdqu 0x20(%rdi),%xmm6
  vmovdqu 0x30(%rdi),%xmm7
  vpshufb %xmm13,%xmm4,%xmm4
  vpshufb %xmm13,%xmm5,%xmm5
  vpshufb %xmm13,%xmm6,%xmm6
  vpshufb %xmm13,%xmm7,%xmm7
  jmpq    .LBc1
  mov    (%rsi),%eax
  mov    0x4(%rsi),%ebx
  mov    0x8(%rsi),%ecx
  mov    0xc(%rsi),%r8d
  mov    0x10(%rsi),%edx
  mov    0x14(%rsi),%r9d
  mov    0x18(%rsi),%r10d
  mov    0x1c(%rsi),%r11d
  vmovdqa .LPSHUFFLE_BYTE_FLIP_MASK(%rip),%ymm13
  vmovdqa .L_SHUF_00BA(%rip),%ymm10
  vmovdqa .L_SHUF_DC00(%rip),%ymm12
  mov    %rsi,0x210(%rsp)
  jmp    .LB1535

.LB15a5:
  vzeroall
  vmovdqa %ymm0,(%rsp)
  vmovdqa %ymm0,0x20(%rsp)
  vmovdqa %ymm0,0x40(%rsp)
  vmovdqa %ymm0,0x60(%rsp)
  vmovdqa %ymm0,0x80(%rsp)
  vmovdqa %ymm0,0xa0(%rsp)
  vmovdqa %ymm0,0xc0(%rsp)
  vmovdqa %ymm0,0xe0(%rsp)
  vmovdqa %ymm0,0x100(%rsp)
  vmovdqa %ymm0,0x120(%rsp)
  vmovdqa %ymm0,0x140(%rsp)
  vmovdqa %ymm0,0x160(%rsp)
  vmovdqa %ymm0,0x180(%rsp)
  vmovdqa %ymm0,0x1a0(%rsp)
  vmovdqa %ymm0,0x1c0(%rsp)
  vmovdqa %ymm0,0x1e0(%rsp)
  xor    %eax,%eax
  mov    0x218(%rsp),%rsp
  pop    %r15
  pop    %r14
  pop    %r13
  pop    %r12
  pop    %rbp
  pop    %rbx

.LB163f:
{$IF DEFINED(WIN64)}
  vmovaps (%rsp),%xmm6
  vmovaps 0x10(%rsp),%xmm7
  vmovaps 0x20(%rsp),%xmm8
  vmovaps 0x30(%rsp),%xmm9
  vmovaps 0x40(%rsp),%xmm10
  vmovaps 0x50(%rsp),%xmm11
  vmovaps 0x60(%rsp),%xmm12
  vmovaps 0x70(%rsp),%xmm13
  vmovaps 0x80(%rsp),%xmm14
  vmovaps 0x90(%rsp),%xmm15
  addq    $168, %rsp
  movq    16(%rsp), %rsi
  movq    24(%rsp), %rdi
{$ENDIF}
  retq

.balign 64
.LK256:
  .long	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5
  .long	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5
  .long	0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5
  .long	0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5
  .long	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3
  .long	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3
  .long	0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174
  .long	0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174
  .long	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc
  .long	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc
  .long	0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da
  .long	0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da
  .long	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7
  .long	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7
  .long	0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967
  .long	0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967
  .long	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13
  .long	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13
  .long	0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85
  .long	0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85
  .long	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3
  .long	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3
  .long	0xd192e819,0xd6990624,0xf40e3585,0x106aa070
  .long	0xd192e819,0xd6990624,0xf40e3585,0x106aa070
  .long	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5
  .long	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5
  .long	0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3
  .long	0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3
  .long	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208
  .long	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208
  .long	0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
  .long	0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2

.LPSHUFFLE_BYTE_FLIP_MASK:
  .long  66051, 67438087, 134810123, 202182159
  .long  66051, 67438087, 134810123, 202182159

.L_SHUF_00BA:
  .long  50462976, 185207048, -1, -1
  .long  50462976, 185207048, -1, -1

.L_SHUF_DC00:
  .long  -1, -1, 50462976, 185207048
  .long  -1, -1, 50462976, 185207048
end;
