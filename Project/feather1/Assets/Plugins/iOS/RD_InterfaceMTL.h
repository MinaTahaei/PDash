#ifndef _RD_INTERFACE_MTL_H_
#define _RD_INTERFACE_MTL_H_

#include "PL_Configure.h"

#ifdef SUPPORT_RENDERER_METAL

#import <Metal/Metal.h>

@class NSBundle;

typedef id<MTLDevice> MTLDeviceRef;
typedef id<MTLCommandEncoder> MTLCommandEncoderRef;
typedef id<MTLCommandQueue> MTLCommandQueueRef;
typedef id<MTLCommandBuffer> MTLCommandBufferRef;


typedef	NSBundle* (*HarmonyGetMetalBundleFunc)();
typedef MTLDeviceRef (*HarmonyGetMetalDeviceFunc)();
typedef MTLCommandEncoderRef (*HarmonyCurrentMTLCommandEncoderFunc)();
typedef void (*HarmonyEndCurrentMTLCommandEncoderFunc)();
typedef MTLCommandBufferRef (*HarmonyCurrentMTLCommandBufferFunc)();
typedef MTLCommandQueueRef (*HarmonyGetMetalCommandQueueFunc)();

extern HarmonyGetMetalBundleFunc HarmonyGetMetalBundle;
extern HarmonyGetMetalDeviceFunc HarmonyGetMetalDevice;
extern HarmonyCurrentMTLCommandEncoderFunc HarmonyCurrentMTLCommandEncoder;
extern HarmonyEndCurrentMTLCommandEncoderFunc HarmonyEndCurrentMTLCommandEncoder;
extern HarmonyCurrentMTLCommandBufferFunc HarmonyCurrentMTLCommandBuffer;
extern HarmonyGetMetalCommandQueueFunc HarmonyGetMetalCommandQueue;

#endif /* SUPPORT_RENDERER_METAL */

#endif /* _RD_INTERFACE_MTL_H_ */
