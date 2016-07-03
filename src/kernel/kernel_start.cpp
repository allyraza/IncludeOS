// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <kernel/os.hpp>
#include <cassert>
//#define ENABLE_STACK_SMASHER
#include <cstdio>

extern "C" void _start(void) __attribute__((visibility("hidden")));
extern "C" void _init_c_runtime();

// enables Streaming SIMD Extensions
static void enableSSE(void)
{
  asm ("mov %cr0, %eax");
  asm ("and $0xFFFB,%ax");
  asm ("or  $0x2,   %ax");
  asm ("mov %eax, %cr0");
  
  asm ("mov %cr4, %eax");
  asm ("or  $0x600,%ax");
  asm ("mov %eax, %cr4");
}

#ifdef ENABLE_STACK_SMASHER
static void __attribute__((noinline))
stack_smasher(const char* src)
{
  char test[8];
  sprintf(test, "%s", src);
}
#endif

void _start(void)
{
  // enable SSE extensions bitmask in CR4 register
  enableSSE();
  
  // Initialize stack-unwinder, call global constructors etc.
  _init_c_runtime();
  
  #ifdef ENABLE_STACK_SMASHER
  // can't detect stack smashing until c runtime is on
  stack_smasher("1234123412341234");
  #endif
  
  // Initialize some OS functionality
  OS::start();
}
