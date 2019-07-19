
#ifndef _HARMONY_RENDERER_H_
#define _HARMONY_RENDERER_H_

// Which platform we are on?
#if _MSC_VER
#define UNITY_WIN 1
#else
#define UNITY_OSX 1
#endif

// Attribute to make function be exported from a plugin
#if UNITY_WIN
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

// Graphics device identifiers in Unity
enum GfxDeviceRenderer
{
    kGfxRendererOpenGL = 0,           // desktop OpenGL (Legacy)
    kGfxRendererD3D9 = 1,             // Direct3D 9
    kGfxRendererD3D11 = 2,            // Direct3D 11
    kGfxRendererGCM = 3,              // PlayStation 3
    kGfxRendererNull = 4,             // "null" device (used in batch mode)
    kGfxRendererXenon = 6,            // Xbox 360
    kGfxRendererOpenGLES20 = 8,       // OpenGL ES 2.0
    kGfxRendererOpenGLES30 = 11,      // OpenGL ES 3.0
    kGfxRendererGXM = 12,             // PlayStation Vita
    kGfxRendererPS4 = 13,             // PlayStation 4
    kGfxRendererXboxOne = 14,         // Xbox One        
    kGfxRendererMetal = 16,           // iOS Metal
	kGfxRendererOpenGLCore = 17,      // OpenGL Core
};


// Event types for UnitySetGraphicsDevice
enum GfxDeviceEventType {
	kGfxDeviceEventInitialize = 0,
	kGfxDeviceEventShutdown,
	kGfxDeviceEventBeforeReset,
	kGfxDeviceEventAfterReset,
};

// If exported by a plugin, this function will be called when graphics device is created, destroyed,
// before it's being reset (i.e. resolution changed), after it's being reset, etc.
extern "C" void EXPORT_API UnitySetGraphicsDevice (void* device, int deviceType, int eventType);

// If exported by a plugin, this function will be called for GL.IssuePluginEvent script calls.
// The function will be called on a rendering thread; note that when multithreaded rendering is used,
// the rendering thread WILL BE DIFFERENT from the thread that all scripts & other game logic happens!
// You have to ensure any synchronization with other plugin script calls is properly done by you.
extern "C" void EXPORT_API UnityRenderEvent (int eventID);

#endif /* _HARMONY_RENDERER_H_ */
