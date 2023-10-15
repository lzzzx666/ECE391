#ifndef IDT_H   

#include "x86_desc.h"
#include "lib.h"

/* dpl: 0 - interrupt  3 - system interrupt */
#define SET_IDT_INTR_ENTRY(str, handler, _seg_selector, _dpl) \
do { \
    SET_IDT_ENTRY(str, handler); \
    (str).seg_selector = _seg_selector; \
    (str).reserved4 = 0; \
    (str).reserved3 = 0; \
    (str).reserved2 = 1; \
    (str).reserved1 = 1; \
    (str).size = 1; \
    (str).reserved0 = 0; \
    (str).dpl = _dpl; \
    (str).present = 1; \
} while(0);

void uni();
void set_IDT();

#endif  /* IDH_H */
