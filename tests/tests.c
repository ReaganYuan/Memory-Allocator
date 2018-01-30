#include <criterion/criterion.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sfmm.h"
#include <errno.h>
/**
 *  HERE ARE OUR TEST CASES NOT ALL SHOULD BE GIVEN STUDENTS
 *  REMINDER MAX ALLOCATIONS MAY NOT EXCEED 4 * 4096 or 16384 or 128KB
 */

Test(sf_memsuite, Malloc_an_Integer, .init = sf_mem_init, .fini = sf_mem_fini) {
  int *x = sf_malloc(sizeof(int));
  *x = 4;
  cr_assert(*x == 4, "Failed to properly sf_malloc space for an integer!");
}

Test(sf_memsuite, Free_block_check_header_footer_values, .init = sf_mem_init, .fini = sf_mem_fini) {
  void *pointer = sf_malloc(sizeof(short));
  sf_free(pointer);
  pointer = (char*)pointer - 8;
  sf_header *sfHeader = (sf_header *) pointer;
  cr_assert(sfHeader->alloc == 0, "Alloc bit in header is not 0!\n");
  sf_footer *sfFooter = (sf_footer *) ((char*)pointer + (sfHeader->block_size << 4) - 8);
  cr_assert(sfFooter->alloc == 0, "Alloc bit in the footer is not 0!\n");
}

Test(sf_memsuite, SplinterSize_Check_char, .init = sf_mem_init, .fini = sf_mem_fini){
  void* x = sf_malloc(32);
  void* y = sf_malloc(32);
  (void)y;

  sf_free(x);

  x = sf_malloc(16);

  sf_header *sfHeader = (sf_header *)((char*)x - 8);
  cr_assert(sfHeader->splinter == 1, "Splinter bit in header is not 1!");
  cr_assert(sfHeader->splinter_size == 16, "Splinter size is not 16");
  sf_footer *sfFooter = (sf_footer *)((char*)sfHeader + (sfHeader->block_size << 4) - 8);
  cr_assert(sfFooter->splinter == 1, "Splinter bit in header is not 1!");
}

Test(sf_memsuite, Check_next_prev_pointers_of_free_block_at_head_of_list, .init = sf_mem_init, .fini = sf_mem_fini) {
  int *x = sf_malloc(4);
  memset(x, 0, 0);
  cr_assert(freelist_head->next == NULL);
  cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Coalesce_no_coalescing, .init = sf_mem_init, .fini = sf_mem_fini) {
  int *x = sf_malloc(4);
  int *y = sf_malloc(4);
  memset(y, 0, 0);
  sf_free(x);

  //just simply checking there are more than two things in list
  //and that they point to each other
  cr_assert(freelist_head->next != NULL);
  cr_assert(freelist_head->next->prev != NULL);
}

//#
//STUDENTSNT UNIT TESTS SHOULD BE WRITTEN BELOW
//DO NOT DELETE THESE COMMENTS
//#

Test(sf_memsuite, unit_test_1, .init = sf_mem_init, .fini = sf_mem_fini){

int* x = sf_malloc(16368);
cr_assert(freelist_head == NULL);

void* y = (char*)x - 8;

sf_header* head = y;
cr_assert(head->splinter == 0, " unit_test_1");
cr_assert(head->splinter_size == 0, "unit_test_1");

sf_free(x);
cr_assert(head->alloc == 0);
cr_assert(head->splinter == 0);

}

Test(sf_memsuite, unit_test_2, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(400);
int*y = sf_malloc(400);
int*z = sf_malloc(400);
memset(z,0,0);
sf_free(y);
sf_free(x);

void*a = (char*)x - 8;
sf_header* head = a;
cr_assert(head->alloc == 0);
cr_assert(head->block_size == 52);

}

Test(sf_memsuite, unit_test_3, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(400);
int*y = sf_malloc(400);
int*z = sf_malloc(400);

memset(z,0,0);

sf_free(x);
sf_free(y);

void*a = (char*)x - 8;
sf_header* head = a;
cr_assert(head->alloc == 0);
cr_assert(head->block_size == 52);


}

Test(sf_memsuite, unit_test_4, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(400);
int*y = sf_malloc(400);
int*z = sf_malloc(400);

sf_free(x);
sf_free(y);
sf_free(z);

void*a = (char*)x - 8;
sf_header* head = a;
cr_assert(head->alloc == 0);
cr_assert(head->block_size == 256);

}

Test(sf_memsuite, unit_test_5, .init = sf_mem_init, .fini = sf_mem_fini){

int*x = sf_malloc(400);
int*y = sf_malloc(400);
int*z = sf_malloc(400);


sf_free(x);
sf_free(z);

void*a = (char*)x - 8;
sf_header* head = a;
cr_assert(head->alloc == 0);
cr_assert(head->block_size == 26);

void*b = (char*)y - 8;
sf_header* head_b = b;
cr_assert(head_b->alloc == 1);
cr_assert(head_b->block_size == 26);

}

Test(sf_memsuite, unit_test_6, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(64);
int*z = sf_malloc(64);
z+= 0;
sf_realloc(x,16);


int*y = sf_malloc(16);

void*a = (char*)y - 8;
sf_header* head = a;
cr_assert(head->splinter == 1);
cr_assert(head->splinter_size == 16);
cr_assert(head->block_size == 3);

}

Test(sf_memsuite,unit_test_7, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(64);
sf_realloc(x,16);

void*a = (char*)x - 8;
sf_header* head = a;
cr_assert(head->block_size == 2);
cr_assert(head->alloc == 1);
cr_assert(head->requested_size == 16);
cr_assert(head->splinter == 0);
cr_assert(head->splinter_size == 0);
sf_free(x);
cr_assert(head->alloc == 0);
cr_assert(head->block_size == 256);


}

Test(sf_memsuite, unit_test_8, .init = sf_mem_init, .fini = sf_mem_fini){
  int* x = sf_malloc(400);
  int* y = sf_malloc(400);
  int* z = sf_malloc(400);
  x+=0;
  z+=0;
  sf_free(y);

  y = sf_malloc(384);

  void*a = (char*)y - 8;
  sf_header* head = a;

  cr_assert(head->splinter == 1);
  cr_assert(head->splinter_size == 16);
  cr_assert(head->block_size == 26);
  cr_assert(head->requested_size == 384);
  cr_assert(head->padding_size == 0);

  void* b = (char*)z + 416 - 8;

  cr_assert(freelist_head == b);
  cr_assert(freelist_head->next == NULL);
  cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite,unit_test_9, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(64);
sf_realloc(x,16);

void*a = (char*)x - 8;
sf_header* head = a;
cr_assert(head->block_size == 2);
cr_assert(head->alloc == 1);
cr_assert(head->requested_size == 16);
cr_assert(head->splinter == 0);
cr_assert(head->splinter_size == 0);

void* hopefully_freelist_head_adr = (char*)head + head->block_size * 16;

cr_assert(freelist_head == hopefully_freelist_head_adr);

hopefully_freelist_head_adr = (char*)hopefully_freelist_head_adr - 8;
sf_footer* foot = hopefully_freelist_head_adr;

cr_assert(foot->alloc == 1);
cr_assert(foot->block_size == head->block_size);
cr_assert(foot->splinter == 0);
}

Test(sf_memsuite, unit_test_10, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(4064);
  void*a = (char*)x - 8;
  sf_header* head = a;
  cr_assert(head->splinter_size == 16);
  cr_assert(head->splinter == 1);
  cr_assert(freelist_head == NULL);

  sf_free(x);

  x = sf_malloc(4080);
  a = (char*)x - 8;
  head = a;
  cr_assert(head->splinter == 0);
  cr_assert(head->splinter_size == 0);
  //cr_assert(freelist_head == NULL);
  cr_assert(head->alloc == 1);
  cr_assert(head->block_size == 256);

}
Test(sf_memsuite, unit_test_11, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(400);
x+=0;
cr_assert(freelist_head != NULL);
}
Test(sf_memsuite, unit_test_12, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(4096);
  int*y = sf_malloc(4096);
  int*z = sf_malloc(4096);

  x+=0;
  y+=0;
  z+=0;

  cr_assert(freelist_head->header.alloc == 0);
  cr_assert(freelist_head->header.block_size == 253);
  cr_assert(freelist_head->next == NULL);
  cr_assert(freelist_head->prev == NULL);


  int*o = sf_malloc(4032);
  o+=0;
  cr_assert(freelist_head == NULL);
}

Test(sf_memsuite, unit_test_13, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(4096);
  int*y = sf_malloc(4096);
  int*z = sf_malloc(4096);
  int*a = sf_malloc(4096);
x+=0,y+=0,a+=0;
  sf_free(z);

  int*b = sf_malloc(4000);
b+=0;
  cr_assert(freelist_head->header.block_size == 259);
  cr_assert(freelist_head->header.alloc == 0);
  cr_assert(freelist_head->header.splinter == 0);

}
Test(sf_memsuite, unit_test_14, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(32);
sf_realloc(x,400);

void*a = (char*)x - 8;
sf_header* head = a;

cr_assert(freelist_head->header.block_size == 230);
cr_assert(freelist_head->header.alloc == 0);
cr_assert(head->block_size == 26);
cr_assert(head->requested_size == 400);
cr_assert(head->alloc == 1);
cr_assert(head->splinter == 0);
cr_assert(head->splinter_size == 0);

}
Test(sf_memsuite, unit_test_15, .init = sf_mem_init, .fini = sf_mem_fini){
int*x = sf_malloc(100);
int*y = sf_malloc(400);
x+=0;
sf_realloc(y,1000);

void*a = (char*)y -  8;
sf_header* head = a;

cr_assert(head->block_size == 64);
cr_assert(head->alloc == 1);
cr_assert(head->splinter == 0);

cr_assert(freelist_head->header.block_size == 184);
cr_assert(freelist_head->header.alloc == 0);
cr_assert(freelist_head->header.splinter == 0);
cr_assert(freelist_head->next == NULL);
cr_assert(freelist_head->prev == NULL);


}
Test(sf_memsuite, unit_test_16, .init = sf_mem_init, .fini = sf_mem_fini){
int*a = sf_malloc(100);
int*b = sf_malloc(200);
int*c = sf_malloc(300);
int*d = sf_malloc(400);
int*e = sf_malloc(500);
a+=0,c+=0,e+=0;
sf_free(b);
sf_free(d);

int*f = sf_malloc(350);

void* x = (char*)f - 8;
sf_header* head = x;
head+=0;

cr_assert(head->block_size == 23);
cr_assert(head->requested_size == 350);

cr_assert(freelist_head->header.block_size == 14);
cr_assert(freelist_head->header.alloc == 0);

cr_assert(freelist_head->next->header.block_size == 3);
cr_assert(freelist_head->next->header.alloc == 0);

cr_assert(freelist_head->next->next->header.block_size == 155);
cr_assert(freelist_head->next->next->header.alloc == 0);
}

Test(sf_memsuite, unit_test_17, .init = sf_mem_init, .fini = sf_mem_fini){
  int*a = sf_malloc(100);
  int*b = sf_malloc(100);
  sf_realloc(b,7000);
  a+=0;
  void*p = (char*)b - 8;
  sf_header* head = p;

  cr_assert(freelist_head->header.block_size == 65);
  cr_assert(freelist_head->header.alloc == 0);
  cr_assert(freelist_head->next == NULL);
  cr_assert(freelist_head->prev == NULL);
  cr_assert(head->block_size == 439);
  cr_assert(head->requested_size == 7000);
  cr_assert(head->alloc == 1);
}
Test(sf_memsuite, unit_test_18, .init = sf_mem_init, .fini = sf_mem_fini){
  int*a = sf_malloc(100);
  int*b = sf_malloc(100);
  sf_realloc(b,8032);
  a+= 0;
  cr_assert(freelist_head == NULL);
  void* p = (char*)b - 8;
  sf_header* head = p;

  cr_assert(head->block_size == 504);
  cr_assert(head->requested_size == 8032);
  cr_assert(head->splinter == 1);
  cr_assert(head->splinter_size == 16);
  cr_assert(head->alloc == 1);
  cr_assert(head->padding_size == 0);

  sf_free(b);

  cr_assert(freelist_head->header.block_size == 504);
  cr_assert(freelist_head->header.alloc == 0);

  int* c = sf_malloc(3500);
  p = (char*)c - 8;
  head = p;

  cr_assert(head->block_size == 220);
  cr_assert(head->requested_size == 3500);
  cr_assert(head->alloc == 1);
}
Test(sf_memsuite, unit_test_19, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(100);
  int*y = sf_malloc(3000);
  int*z = sf_malloc(100);
  x+=0,z+=0;
  sf_realloc(y,16);

  void* a = (char*)y - 8;
  sf_header* b = (void*)a;
  cr_assert(b->block_size == 2);
  cr_assert(b->requested_size == 16);
  cr_assert(b->splinter == 0);
  cr_assert(freelist_head != NULL);
  cr_assert(freelist_head->next != NULL);
  cr_assert(freelist_head->next->prev != NULL);
}

Test(sf_memsuite, unit_test_20, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(100);
  int*y = sf_malloc(96);
  int*z = sf_malloc(100);
  x+=0,z+=0;
  sf_realloc(y,80);
  void* a = (char*)y - 8;
  sf_header* head = a;
  cr_assert(head->block_size == 7);
  cr_assert(head->requested_size == 80);
  cr_assert(head->alloc == 1);
  cr_assert(freelist_head->next == NULL);
  cr_assert(freelist_head != NULL);
  cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, unit_test_22, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(1000);
  int*y = sf_malloc(1000);
  int*z = sf_malloc(1000);
  int*a = sf_malloc(980);
  x+=0,y+=0,z+=0,a+=0;
  cr_assert(freelist_head == NULL);

  void* p = (char*)x - 8;
  sf_header* head = p;
  cr_assert(head->block_size == 64);
  cr_assert(head->requested_size == 1000);
  cr_assert(head->alloc == 1);


  p = (char*)head + head->block_size*16 - 8;
  sf_header*foot = p;
  cr_assert(foot->block_size == head->block_size);
  cr_assert(foot->alloc == head->alloc);
  cr_assert(foot->splinter == head->splinter);

  p = (char*)y - 8;
  head = p;
  cr_assert(head->block_size == 64);
  cr_assert(head->requested_size == 1000);
  cr_assert(head->alloc == 1);

  p =(char*) head + head->block_size*16 - 8;
  foot = p;
  cr_assert(foot->block_size == head->block_size);
  cr_assert(foot->alloc == head->alloc);
  cr_assert(foot->splinter == head->splinter);
}
Test(sf_memsuite, unit_test_23, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(1000);
  int*y = sf_malloc(1008);
  int*z = sf_malloc(1000);
  int*a = sf_malloc(400);
  x+=0, a+=0;
  sf_free(y);
  sf_free(z);

  cr_assert(freelist_head->header.block_size == 128);
  cr_assert(freelist_head->header.alloc == 0);

  cr_assert(freelist_head->next->header.block_size == 38);
  cr_assert(freelist_head->next->header.alloc == 0);

}

Test(sf_memsuite, unit_test_24, .init = sf_mem_init, .fini = sf_mem_fini){
  int*a = sf_malloc(400);
  int*b = sf_malloc(28);
  int*c = sf_malloc(400);
  int*d = sf_malloc(980);
  int*e = sf_malloc(1);
  int*f = sf_malloc(33);
  int*g = sf_malloc(33);
  a+=0,e+=0,g+=0;
  sf_free(f);
  sf_free(d);
  sf_free(b);
  sf_free(c);

  cr_assert(freelist_head->header.block_size == 92);
  cr_assert(freelist_head->header.alloc == 0);

  cr_assert(freelist_head->next->header.block_size == 4);
  cr_assert(freelist_head->next->header.alloc == 0 );

  cr_assert(freelist_head->next->next->header.block_size == 128);
  cr_assert(freelist_head->next->next->header.alloc == 0);

  cr_assert(freelist_head->next->next->next == NULL);
}

Test(sf_memsuite,unit_test_25, .init = sf_mem_init, .fini = sf_mem_fini){
  int*a = sf_malloc(400);
  int*b = sf_malloc(28);
  int*c = sf_malloc(400);
  int*d = sf_malloc(980);
  int*e = sf_malloc(1);
  int*f = sf_malloc(33);
  int*g = sf_malloc(5000);
  int*h = sf_malloc(9360);

  sf_free(a);
  sf_free(b);
  sf_free(c);
  sf_free(d);
  sf_free(e);
  sf_free(f);
  sf_free(g);
  sf_free(h);

  cr_assert(freelist_head->header.block_size == 16384/16);
  cr_assert(freelist_head->next == NULL);
  cr_assert(freelist_head->prev == NULL);
  cr_assert(freelist_head != NULL);
}
Test(sf_memsuite, unit_test_26, .init = sf_mem_init, .fini = sf_mem_fini){
 int*a = sf_malloc(400);
  int*b = sf_malloc(28);
  int*c = sf_malloc(400);
  int*d = sf_malloc(980);
  int*e = sf_malloc(1);
  int*f = sf_malloc(33);
  int*g = sf_malloc(5000);
  int*h = sf_malloc(9360);

  info k;

  sf_free(b);
  sf_free(c);
  sf_free(d);
  sf_free(a);
  sf_free(g);
  sf_free(e);
  sf_free(h);
  sf_free(f);

  sf_info(&k);

  cr_assert(k.allocatedBlocks == 0);
  cr_assert(freelist_head->header.block_size == 16386/16);

  a = sf_malloc(9360);
  cr_assert(freelist_head->header.block_size == 438);

  sf_info(&k);
  cr_assert(k.allocatedBlocks == 1);

}

Test(sf_memsuite, unit_test_27, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(400);
  sf_realloc(x,400);

  void*p = (char*)x - 8;
  sf_header* head = p;

  cr_assert(head->block_size == 26);
  cr_assert(head->alloc == 1);
  cr_assert(head->requested_size == 400);

  p = (char*)head + head->block_size*16 - 8;
  sf_footer* foot = p;
  cr_assert(foot->block_size == 26);
  cr_assert(foot->alloc == 1);

}
Test(sf_memsuite, unit_test_28, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(400);
  int*y = sf_malloc(400);
  int*z = sf_malloc(400);
  sf_free(y);
  x+=0,z+=0;
  int*a = sf_malloc(380);
  info k;
  void*p = (char*)a - 8;
  sf_header* h = p;

  cr_assert(h->block_size == 26);
  cr_assert(h->requested_size == 380);
  cr_assert(h->splinter == 1);
  cr_assert(h->splinter_size == 16);

  p = (char*)h + h->block_size*16 - 8;
  sf_footer* f = p;

  cr_assert(f->block_size == h->block_size);
  cr_assert(f->splinter == 1);
  cr_assert(f->alloc == 1);

    int*c = sf_malloc(400);
  sf_free(z);

  int*d = sf_malloc(380);
  c+=0,d+=0;

  p = (char*)d - 8;
  h = p;
  cr_assert(h->block_size == 26);
  cr_assert(h->requested_size == 380);
  cr_assert(h->splinter == 1);

  p = (char*)h + h->block_size*16 - 8;
  f = p;
  cr_assert(f->block_size == h->block_size);
  cr_assert(f->splinter == 1);
  sf_info(&k);

  cr_assert(k.allocatedBlocks == 4);
  cr_assert(k.splinterBlocks == 2);
  cr_assert(k.splintering == 32);
  cr_assert(k.padding == 8);
}

Test(sf_memsuite, unit_test_29, .init = sf_mem_init, .fini = sf_mem_fini){
  int*x = sf_malloc(400);
  int*y = sf_malloc(400);
  int*z = sf_malloc(400);
  sf_free(y);
  x+=0,z+=0;
  int*a = sf_malloc(380);
  info k;
  void*p = (char*)a - 8;
  sf_header* h = p;

  cr_assert(h->block_size == 26);
  cr_assert(h->requested_size == 380);
  cr_assert(h->splinter == 1);
  cr_assert(h->splinter_size == 16);

  p = (char*)h + h->block_size*16 - 8;
  sf_footer* f = p;

  cr_assert(f->block_size == h->block_size);
  cr_assert(f->splinter == 1);
  cr_assert(f->alloc == 1);

  int*c = sf_malloc(400);
  sf_free(z);

  int*d = sf_malloc(380);
  c+=0,d+=0;

  sf_info(&k);

  cr_assert(k.peakMemoryUtilization == 0.390625);
}

Test(sf_memsuite, unit_test_30, .init = sf_mem_init, .fini = sf_mem_fini){
    info k ;
    int* a = sf_malloc(50);
    int* b = sf_malloc(54);
    int* c = sf_malloc(123);
    int* d = sf_malloc(864);
    sf_free(d);
    sf_free(a);
    int*e = sf_malloc(533);
    int*f = sf_malloc(900);
    sf_free(f);
    b+=0,c+=0,e+=0;
    sf_info(&k);

    cr_assert(k.allocatedBlocks == 3);
    cr_assert(k.coalesces == 2);

    cr_assert(freelist_head->header.block_size == 5);
    cr_assert(freelist_head->header.alloc == 0);
    cr_assert(freelist_head->next->header.block_size == 202);
    cr_assert(freelist_head->next->header.alloc == 0);
}
Test(sf_memsuite, unit_test_31, .init = sf_mem_init, .fini = sf_mem_fini){
    int* a = sf_malloc(100);
    int* b = sf_malloc(1000);
    int* c = sf_malloc(100);
    b = sf_realloc(b,500);
    int* d = sf_malloc(464);
    a+=0,c+=0,d+=0;

    void* vp = (char*)d - 8;
    sf_header* h = vp;

    cr_assert(h->splinter_size == 16);
    cr_assert(h->splinter == 1);
    cr_assert(h->requested_size == 464);

    vp = (char*)h + h->block_size*16 - 8;
    sf_footer* f = vp;

    cr_assert(f->splinter == h->splinter);
    cr_assert(f->block_size == h->block_size);

    cr_assert(freelist_head->next == NULL);


}

Test(sf_memsuite, unit_test_32, .init = sf_mem_init, .fini = sf_mem_fini) {
   sf_free(NULL);
   cr_assert(errno == EINVAL, "ERRNO must be EINVAL!");
}
Test(sf_memsuite, unit_test_33, .init = sf_mem_init, .fini = sf_mem_fini){
  sf_realloc(NULL,0);
  cr_assert(errno == EINVAL);
}
Test(sf_memsuite, unit_test_34, .init = sf_mem_init, .fini = sf_mem_fini){
  int* x = sf_malloc(0);
  cr_assert(x == NULL);
  cr_assert(errno == EINVAL);
}

Test(sf_memsuite, unit_test_35, .init = sf_mem_init, .fini = sf_mem_fini){
  int* x = sf_malloc(1);
  x+=0;
  sf_realloc(x,14000);
  cr_assert(errno = ENOMEM);
}
Test(sf_memsuite, unit_test_36, .init = sf_mem_init, .fini = sf_mem_fini){
  info k;
   int* a = sf_malloc(48);
    int* b = sf_malloc(5000);
    int* c = sf_malloc(123);
    int* d = sf_malloc(4000);
    int* e = sf_realloc(a,4000);

    sf_info(&k);
    b+=0,c+=0,d+=0,e+=0;
    cr_assert(k.allocatedBlocks == 4);
    cr_assert(k.padding == 13);
    cr_assert(freelist_head != NULL);
    cr_assert(freelist_head->next != NULL);
    cr_assert(freelist_head->next->header.block_size == 3120/16);
    cr_assert(freelist_head->header.block_size == 64/16);

}

Test(sf_memsuite, unit_test_37, .init = sf_mem_init, .fini = sf_mem_fini){
    int* a = sf_malloc(4060);
    int* b = sf_malloc(4060);
    int* c = sf_malloc(4060);
    int* d = sf_malloc(4060);
    int* e = sf_malloc(1);
    a+=0,b+=0,c+=0,d+=0;
    cr_assert(e == NULL);
    cr_assert(errno == ENOMEM);
    cr_assert(freelist_head == NULL);
}



