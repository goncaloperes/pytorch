#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <vector>
#include <ctime>

typedef std::chrono::nanoseconds ns;
constexpr uint64_t warmup_iters = 1000;
constexpr uint64_t iters = 200000;
constexpr int max_value = 50;

int rand(const int max_value) {
  return 1 + std::rand()/((RAND_MAX + 1u)/max_value);
}

int main() {
  std::srand(std::time(0));
  const auto size = ::rand(max_value);
  float* a = (float*)std::malloc(sizeof(float) * size);
  float* b = (float*)std::malloc(sizeof(float) * size);
  for (auto i = decltype(size){0}; i < size; ++i) {
    a[i] = ::rand(max_value);
    b[i] = ::rand(max_value);
  }

  for (auto i = decltype(warmup_iters){0}; i < warmup_iters; ++i) {
    float* c = (float*)std::malloc(sizeof(float) * size);
    for (auto i = decltype(size){0}; i < size; ++i) {
      c[i] = a[i] + b[i];
    }
    std::free(c);
  }

  const auto start = std::chrono::high_resolution_clock::now();

  for (auto i = decltype(iters){0}; i < iters; ++i) {
    float* c = (float*)std::malloc(sizeof(float) * size);
    for (auto i = decltype(size){0}; i < size; ++i) {
      c[i] = a[i] + b[i];
    }
    std::free(c);
  }

/* Loop assembly (from Godbolt)
.L13:
        mov     rdi, QWORD PTR [rsp]
        call    malloc
        cmp     DWORD PTR [rsp+8], 2147483643
        ja      .L15
        xor     edx, edx
.L11:
        movups  xmm0, XMMWORD PTR [r14+rdx]
        movups  xmm3, XMMWORD PTR [r15+rdx]
        addps   xmm0, xmm3
        movups  XMMWORD PTR [rax+rdx], xmm0
        add     rdx, 16
        cmp     rdx, r13
        jne     .L11
        mov     edx, ebx
        cmp     DWORD PTR [rsp+12], ebx
        je      .L12
.L10:
        movsx   rdi, edx
        movss   xmm0, DWORD PTR [r15+rdi*4]
        addss   xmm0, DWORD PTR [r14+rdi*4]
        movss   DWORD PTR [rax+rdi*4], xmm0
        lea     edi, [rdx+1]
        cmp     edi, r12d
        jge     .L12
        movsx   rdi, edi
        add     edx, 2
        movss   xmm0, DWORD PTR [r14+rdi*4]
        addss   xmm0, DWORD PTR [r15+rdi*4]
        movss   DWORD PTR [rax+rdi*4], xmm0
        cmp     edx, r12d
        jge     .L12
        movsx   rdx, edx
        movss   xmm0, DWORD PTR [r15+rdx*4]
        addss   xmm0, DWORD PTR [r14+rdx*4]
        movss   DWORD PTR [rax+rdx*4], xmm0
.L12:
        mov     rdi, rax
        call    free
        sub     rbp, 1
        jne     .L13
*/

  const auto end = std::chrono::high_resolution_clock::now();
  const auto elapsed = end - start;
  const auto elapsed_ns = std::chrono::duration_cast<ns>(elapsed);
  const std::size_t flops = size * iters;
  const std::size_t bytes = sizeof(float) * 3 * size * iters;

  std::cout << "Ran in " << elapsed_ns.count() << "ns\n" << std::endl;
  std::cout << "Array size: " << size << "\n" << std::endl;
  std::cout << "Each iteration took ~" << elapsed_ns.count()/iters << "ns\n" << std::endl;
  std::cout << "(perceived) Flops per nanonsecond: " << flops/((double)elapsed_ns.count()) << "\n" << std::endl;
  std::cout << "Gigabytes per second: " << bytes/((double)elapsed_ns.count()) << "\n" << std::endl;

  std::free(a);
  std::free(b);
  return 0;
}
