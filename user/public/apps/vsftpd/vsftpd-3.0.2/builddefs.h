#ifndef VSF_BUILDDEFS_H
#define VSF_BUILDDEFS_H

#undef VSF_BUILD_TCPWRAPPERS
#ifdef __SC_BUILD__
#undef VSF_BUILD_PAM
#define VSF_BUILD_SSL
#else
#define VSF_BUILD_PAM
#undef VSF_BUILD_SSL
#endif

#endif /* VSF_BUILDDEFS_H */

