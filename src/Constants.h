#pragma once

#define APP_NAME "NRDAVPlayer"

#define APP_VERSION_MAJOR 5
#define APP_VERSION_MINOR 1
#define APP_VERSION_PATCH 2

#define APP_YEAR    "2022"

#if 0
    #include "../NRDAVPlayer.README.txt"     /* Release notes */
#endif

#define APP_MODULE  APP_NAME
#define APP_PRODUCT "NRDAVPlayer"
#define APP_COMPANY "NRD Multimedia"

#define APP__makestr_aux(y) #y
#define APP__makestr(x)     APP__makestr_aux(x)
#define APP_VERSION         APP__makestr(APP_VERSION_MAJOR) "." \
                            APP__makestr(APP_VERSION_MINOR) "." \
                            APP__makestr(APP_VERSION_PATCH)

#define APP_VERSION_STR     APP_VERSION ".0"
#define APP_VERSION_NUM     APP_VERSION_MAJOR,   \
                            APP_VERSION_MINOR,   \
                            APP_VERSION_PATCH,   \
                            0

#define EFFECT_VER_REFL  "Vertical reflection"
#define EFFECT_HOR_REFL  "Horizontal reflection"
#define EFFECT_BOTH_REFL "Both reflection"
