
#include "RD_InterfaceMTL.h"

#ifdef SUPPORT_RENDERER_METAL

HarmonyGetMetalBundleFunc HarmonyGetMetalBundle = NULL;
HarmonyGetMetalDeviceFunc HarmonyGetMetalDevice = NULL;
HarmonyCurrentMTLCommandEncoderFunc HarmonyCurrentMTLCommandEncoder = NULL;
HarmonyEndCurrentMTLCommandEncoderFunc HarmonyEndCurrentMTLCommandEncoder = NULL;
HarmonyCurrentMTLCommandBufferFunc HarmonyCurrentMTLCommandBuffer = NULL;
HarmonyGetMetalCommandQueueFunc HarmonyGetMetalCommandQueue = NULL;

extern "C" void HarmonySetGetMetalBundleFunc( HarmonyGetMetalBundleFunc func )
{
  HarmonyGetMetalBundle = func;
}

extern "C" void HarmonySetGetMetalDeviceFunc( HarmonyGetMetalDeviceFunc func )
{
  HarmonyGetMetalDevice = func;
}

extern "C" void HarmonySetCurrentMTLCommandEncoderFunc( HarmonyCurrentMTLCommandEncoderFunc func )
{
  HarmonyCurrentMTLCommandEncoder = func;
}

extern "C" void HarmonySetEndCurrentMTLCommandEncoder( HarmonyEndCurrentMTLCommandEncoderFunc func )
{
  HarmonyEndCurrentMTLCommandEncoder = func;
}

extern "C" void HarmonySetCurrentMTLCommandBufferFunc( HarmonyCurrentMTLCommandBufferFunc func )
{
  HarmonyCurrentMTLCommandBuffer = func;
}

extern "C" void HarmonySetGetMetalCommandQueueFunc( HarmonyGetMetalCommandQueueFunc func )
{
  HarmonyGetMetalCommandQueue = func;
}

#endif /* SUPPORT_RENDERER_METAL */

