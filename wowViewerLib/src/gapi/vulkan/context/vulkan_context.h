//
// Created by Deamon on 09.02.23.
//

#ifndef AWEBWOWVIEWERCPP_VULKAN_CONTEXT_H
#define AWEBWOWVIEWERCPP_VULKAN_CONTEXT_H

#ifdef LINK_VULKAN
#define VK_NO_PROTOTYPES
#include "../volk.h"

//HACKS for stupid defines in X11
#undef None           // Defined by X11/X.h to 0L
#undef Status         // Defined by X11/Xlib.h to int
#undef True           // Defined by X11/Xlib.h to 1
#undef False          // Defined by X11/Xlib.h to 0
#undef Bool           // Defined by X11/Xlib.h to int
#undef RootWindow     // Defined by X11/Xlib.h
#undef CurrentTime    // Defined by X11/X.h to 0L
#undef Success        // Defined by X11/X.h to 0
#undef DestroyAll     // Defined by X11/X.h to 0
#undef COUNT          // Defined by X11/extensions/XI.h to 0
#undef CREATE         // Defined by X11/extensions/XI.h to 1
#undef DeviceAdded    // Defined by X11/extensions/XI.h to 0
#undef DeviceRemoved  // Defined by X11/extensions/XI.h to 1

#include "../vk_mem_alloc.h"
#endif

#endif //AWEBWOWVIEWERCPP_VULKAN_CONTEXT_H
