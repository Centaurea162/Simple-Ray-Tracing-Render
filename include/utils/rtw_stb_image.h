//
// Created by 86153 on 2023/5/19.
//

#ifndef RAYTRACING_RTW_STB_IMAGE_H
#define RAYTRACING_RTW_STB_IMAGE_H


// Disable pedantic warnings for this external library.
#ifdef _MSC_VER
// Microsoft Visual C++ Compiler
    #pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

// Restore warning levels.
#ifdef _MSC_VER
// Microsoft Visual C++ Compiler
    #pragma warning (pop)
#endif

#endif //RAYTRACING_RTW_STB_IMAGE_H
