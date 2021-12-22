#ifndef FK_TRY_H
#define FK_TRY_H

#include "log.h"

#define FK_TRY_ERRNO(x)                                                \
    if(x < 0) {                                                        \
        fk_traceln("FK_TRY failed at %s: %s,", __FILE__, __LINE__);\
        fk_infoln("Aborting");\
        return -1;\
    }\


#endif //FK_TRY_H

