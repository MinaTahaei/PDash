#ifndef _PL_CONFIGURE_H_
#define _PL_CONFIGURE_H_

// Target platform
#define TARGET_IOS

// Rendering engine
#define SUPPORT_RENDERER_GLES2

#if UNITY_VERSION >= 500
#define SUPPORT_RENDERER_METAL
#endif

#endif /* _PL_CONFIGURE_H_ */
