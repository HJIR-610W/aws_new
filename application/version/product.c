
#include "product.h"


const char* product_name_list[] = {
#define X(code, name) name,
    PRODUCT_LIST
#undef X
};

const char* alias_name_list[] = {
#define X(code, name) name,
    ALIAS_LIST
#undef X
};

const char* section_name_list[] = {
#define X(code, name) name,
    SECTION_LIST
#undef X
};


