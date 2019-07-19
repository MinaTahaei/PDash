#import "UnityAppController.h"
#import "UnityInterface.h"

#if UNITY_VERSION >= 500
typedef NSBundle* (*HarmonyGetMetalBundleFunc)();
typedef MTLDeviceRef (*HarmonyGetMetalDeviceFunc)();
typedef MTLCommandEncoderRef (*HarmonyCurrentMTLCommandEncoderFunc)();
typedef void (*HarmonyEndCurrentMTLCommandEncoder)();
typedef MTLCommandBufferRef (*HarmonyCurrentMTLCommandBufferFunc)();
typedef MTLCommandQueueRef (*HarmonyGetMetalCommandQueueFunc)();

extern "C" void HarmonySetGetMetalBundleFunc( HarmonyGetMetalBundleFunc func );
extern "C" void HarmonySetGetMetalDeviceFunc( HarmonyGetMetalDeviceFunc func );
extern "C" void HarmonySetCurrentMTLCommandEncoderFunc( HarmonyCurrentMTLCommandEncoderFunc func );
extern "C" void HarmonySetEndCurrentMTLCommandEncoder( HarmonyEndCurrentMTLCommandEncoder func );
extern "C" void HarmonySetCurrentMTLCommandBufferFunc( HarmonyCurrentMTLCommandBufferFunc func );
extern "C" void HarmonySetGetMetalCommandQueueFunc( HarmonyGetMetalCommandQueueFunc func );

static int registerCallbacks()
{
  HarmonySetGetMetalBundleFunc(&UnityGetMetalBundle);
  HarmonySetGetMetalDeviceFunc(&UnityGetMetalDevice);
  HarmonySetCurrentMTLCommandEncoderFunc(&UnityCurrentMTLCommandEncoder);
  HarmonySetEndCurrentMTLCommandEncoder(&UnityEndCurrentMTLCommandEncoder);
  HarmonySetCurrentMTLCommandBufferFunc(&UnityCurrentMTLCommandBuffer);
  HarmonySetGetMetalCommandQueueFunc(&UnityGetMetalCommandQueue);
  return 0;
}

static int registered = registerCallbacks();
#endif // UNITY_VERSION >= 500

extern "C" void HarmonySetGraphicsDevice (void* device, int deviceType, int eventType);
extern "C" void HarmonyRenderEvent (int eventID);

@interface HarmonyAppController : UnityAppController
{
}
- (void)shouldAttachRenderDelegate;
@end

@implementation HarmonyAppController
- (void)shouldAttachRenderDelegate;
{
  UnityRegisterRenderingPlugin(&HarmonySetGraphicsDevice, &HarmonyRenderEvent);
}
@end

IMPL_APP_CONTROLLER_SUBCLASS(HarmonyAppController);

