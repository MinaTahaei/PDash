
#include "HarmonyRenderer.h"
#include <stdio.h>

#include "MT_Matrix4x4.h"
#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"
#include "PL_Configure.h"
#include "TV_Blending.h"

#include "RD_SpriteSheet.h"
#include "RD_ClipData.h"
#include "RD_RenderScript.h"

#include "RD_RendererGL.h"
#include "RD_RendererGLES2.h"
#include "RD_RendererDX9.h"
#include "RD_RendererDX11.h"
#include "RD_RendererMTL.h"

#include "RD_RenderId.h"
#include "RD_RenderObjectManager.h"
#include "RD_RenderEvent.h"
#include "RD_RenderEventBatch.h"
#include "RD_CameraEvent.h"
#include "RD_RenderTextureEvent.h"

#include "XML_ProjectLoader.h"
#include "BIN_ProjectLoader.h"


#include "TR_NodeTree.h"
#include "TV_NodeTreeView.h"

//  FIXME, should find a better way to enforce that!
#include "IM_PngIOHandler.h"
REGISTER_IMAGE_HANDLER(IM_PngIOHandler);

#include <set>

#define HARMONY_RENDERER_CLEANUP 1
#define HARMONY_RENDERER_FINAL_CLEANUP 2
#define HARMONY_RENDERER_RENDER_TEXTURE 3

#if defined(TARGET_DESKTOP) || defined(TARGET_ANDROID) || defined(TARGET_WEBGL)
#define HARMONY_RENDER_EVENT_FUNC        UnityRenderEvent
#define HARMONY_SET_GRAPHICS_DEVICE_FUNC UnitySetGraphicsDevice
#else
#define HARMONY_RENDER_EVENT_FUNC        HarmonyRenderEvent
#define HARMONY_SET_GRAPHICS_DEVICE_FUNC HarmonySetGraphicsDevice
#endif

#define  LOG_TAG    "LogHarmony"
#define  ERROR_TAG  "ErrorHarmony"

#ifdef TARGET_ANDROID
#include <android/log.h>

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,ERROR_TAG,__VA_ARGS__)
#else
#define  LOGI(...)  fprintf(stderr, "[%s] ", LOG_TAG );   fprintf(stderr, __VA_ARGS__)
#define  LOGE(...)  fprintf(stderr, "[%s] ", ERROR_TAG ); fprintf(stderr, __VA_ARGS__)
#endif

//WARNING: Experimental feature - Use at your own risk.
//#define USE_ANIMATION_BLENDING

static int                             g_rendererType = -1; //  Renderer type sent from Unity.
static RD_Renderer                    *g_renderer = 0;      //  Renderer is set through Unity.
static RD_RenderEventBatch             g_batch;             //  Current frame batch of Render Events.

namespace
{
  void dumpTree( const TV_NodeTreeViewPtr_t &nodeTreeView, const STD_String &prefix )
  {
    LOGI( "%s[%s]\n", prefix.c_str(), nodeTreeView->name().c_str() );

    for ( TV_NodeTreeView::BrotherIterator i = nodeTreeView->childBegin(), iEnd = nodeTreeView->childEnd() ; i!=iEnd ; ++i )
    {
      dumpTree( &(*i), prefix + "  " );
    }
  }

  bool testFeatures( RD_RenderScript::Feature requestedFeatures, RD_RenderScript::Feature supportedFeatures )
  {
    return (requestedFeatures & supportedFeatures) == requestedFeatures;

  }
}

extern "C" int EXPORT_API CreateRenderScript(const char *projectFolder, const char *clipName)
{
  RD_ClipDataKey key(projectFolder, clipName);
  RD_ClipDataPtr_t pClipData = RD_ClipDataManager::instance()->object(key);
  if ( pClipData.get() == 0 )
  {
    LOGE( "Clip '%s' has not been loaded in memory!\n", clipName );
    return -1;
  }

  if ( pClipData->count() > 0 )
  {
    RD_RenderScript::Feature requestedFeatures = RD_RenderScript::ePlainFeature;
    if ( pClipData->fxEnabled() )
      requestedFeatures = RD_RenderScript::Feature(requestedFeatures | RD_RenderScript::eCutterFeature | RD_RenderScript::eDeformationFeature);

    RD_RenderScriptPtr_t pRenderScript;

    //  Plain Script
    if (  testFeatures(requestedFeatures, RD_RenderScriptPlain::supportedFeatures()) )
    {
      pRenderScript = new RD_RenderScriptPlain;
    }
    //  Fx Script
    else // if (  testFeatures(requestedFeatures, RD_RenderScriptFx::supportedFeatures()) )
    {
      pRenderScript = new RD_RenderScriptFx;
    }

    int scriptId = RD_RenderId::uniqueId();
    RD_RenderScriptManager::instance()->addObject( scriptId, pRenderScript );
    return scriptId;
  }

  return -1;
}

extern "C" int EXPORT_API UpdateRenderScript( int scriptId, const char *projectFolder, const char *clipName, const char *sheetResolution, float frame, unsigned int color, int discretizationStep)
{
  RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );

  RD_ClipDataKey key(projectFolder, clipName);
  RD_ClipDataPtr_t pClipData = RD_ClipDataManager::instance()->object(key);
  if ( pClipData.get() == 0 )
  {
    LOGE( "Clip '%s' has not been loaded in memory!\n", clipName );
    return -1;
  }

  if ( pClipData->count() > 0 )
  {
    bool newScript = false;

    RD_RenderScript::Feature requestedFeatures = RD_RenderScript::ePlainFeature;
    if ( pClipData->fxEnabled() )
      requestedFeatures = RD_RenderScript::Feature(requestedFeatures | RD_RenderScript::eCutterFeature | RD_RenderScript::eDeformationFeature);

    //  Todo.  Implement a proper factory class to instantiate render scripts depending on
    //  requested features.
    if ( pRenderScript.isValid() )
    {
      if ( !pRenderScript->supportsFeature(requestedFeatures) )
      {
        //  Forward meta information to new script...
        RD_RenderScriptMetaPtr_t pMeta = pRenderScript->meta();

        //  Create new render script to support requested features.
        RD_RenderScriptManager::instance()->eraseObject( scriptId );

        //  Plain Script
        if ( testFeatures(requestedFeatures, RD_RenderScriptPlain::supportedFeatures()) )
        {
          pRenderScript = new RD_RenderScriptPlain(pMeta);
          newScript = true;
        }
        //  Fx Script
        else // if ( ( testFeatures(requestedFeatures, RD_RenderScriptFx::supportedFeatures()) )
        {
          pRenderScript = new RD_RenderScriptFx(pMeta);
          newScript = true;
        }
      }
    }
    else
    {
      //  Plain Script
      if (  testFeatures(requestedFeatures, RD_RenderScriptPlain::supportedFeatures()) )
      {
        pRenderScript = new RD_RenderScriptPlain;
        newScript = true;
      }
      //  Fx Script
      else // if (  testFeatures(requestedFeatures, RD_RenderScriptFx::supportedFeatures()) )
      {
        pRenderScript = new RD_RenderScriptFx;
        newScript = true;
      }
    }

    pRenderScript->update( pClipData, projectFolder, sheetResolution, frame, color, discretizationStep );

    if ( newScript )
    {
      RD_RenderScriptManager::instance()->addObject( scriptId, pRenderScript );
    }

    return scriptId;
  }

  return -1;
}

extern "C" void EXPORT_API UnloadRenderScript( int scriptId )
{
  LOGI( "Unloading Render Script id: 0x%x\n", scriptId );

  //  Erase render script from manager list.
  RD_RenderScriptManager::instance()->eraseObject( scriptId );
}

extern "C" void EXPORT_API CreateRenderEvent( int frameCount, int depth, int scriptId, int layerMask, float model[] )
{
  RD_RenderEvent *event = new RD_RenderEvent(scriptId, layerMask, Math::Matrix4x4(model));
  g_batch.addEvent( frameCount, depth, event );
}

extern "C" void EXPORT_API CreateRenderTextureEvent( int scriptId, float offsetx, float offsety, float width, float height, float scale, void *texture )
{
  int eventId = RD_RenderId::uniqueId();
  RD_RenderTextureEventPtr_t pEvent = new RD_RenderTextureEvent( scriptId, offsetx, offsety, width, height, scale, texture );
  RD_RenderTextureEventManager::instance()->addObject( eventId, pEvent );
}

extern "C" int EXPORT_API CreateCameraEvent( int /*frameCount*/, int cullingMask, float projection[], float view[] )
{
  RD_CameraEventPtr_t pEvent = new RD_CameraEvent(cullingMask, Math::Matrix4x4(projection), Math::Matrix4x4(view));

  int eventId = RD_RenderId::uniqueId();
  RD_CameraEventManager::instance()->addObject( eventId, pEvent );

  return eventId;
}

extern "C" float EXPORT_API LoadStageClip( const char *projectFolder, const char *clipName, const char *sheetResolution )
{
  //  Load a stage clip.  Build clip data and spritesheets and register in manager.
  //  This bit must be called in the worker thread and is thread-safe with rendering thread
  //  that cannot create rendering objects on its own.

  LOGI( "Loading Stage Clip ('%s':'%s')\n", projectFolder, clipName );
  RD_ClipDataKey key(projectFolder, clipName);
  RD_ClipDataPtr_t pClipData = RD_ClipDataManager::instance()->object(key);
  if (pClipData.isValid())
  {
    return pClipData->totalDuration();
  }

  RD_ClipDataCore *clipDataCore = new RD_ClipDataCore;

  //  Try to load from binary file, fallback on xml format.
  if ( BIN_ProjectLoader::loadStageClip( projectFolder, clipName, clipDataCore ) == false )
    XML_ProjectLoader::loadStageClip( projectFolder, clipName, clipDataCore );

  pClipData = new RD_ClipData(clipDataCore);

  for ( unsigned i = 0, iEnd = (unsigned)clipDataCore->count() ; i<iEnd ; ++i )
  {
    TV_NodeTreeViewPtr_t nodeTreeView = clipDataCore->nodeTreeView(i);
    dumpTree( nodeTreeView, "" );

    //  Load sprite sheets if necessary
    STD_String spriteSheetName;
    if (nodeTreeView->spriteSheet(spriteSheetName))
    {
      RD_SpriteSheetKey key(projectFolder, spriteSheetName, sheetResolution);
      RD_SpriteSheet::createOrLoad( key );
    }
  }

  RD_ClipDataManager::instance()->addObject( key, pClipData );

  return pClipData->totalDuration();
}

extern "C" void EXPORT_API UnloadStageClip( const char *projectFolder, const char *clipName )
{
  LOGI( "Unloading Stage Clip ('%s':'%s')\n", projectFolder, clipName );

  //  Erase clip data from manager list.
  RD_ClipDataKey key(projectFolder, clipName);
  RD_ClipDataManager::instance()->eraseObject(key);
}

extern "C" void EXPORT_API UnloadProject( const char *projectFolder )
{
  LOGI( "Unloading Project '%s'\n", projectFolder );

  //  Erase sprite sheets from manager list.
  //  This bit must be called in the worker thread and is thread-safe with rendering thread
  //  that cannot create rendering objects on its own.

  size_t nSpritesheets = RD_SpriteSheetManager::instance()->size();

  typedef STD_Set< RD_SpriteSheetKey > SpriteSheetKeyCol_t;
  SpriteSheetKeyCol_t toDelete;

  for ( unsigned idx = 0 ; idx < nSpritesheets ; ++idx )
  {
    RD_SpriteSheetPtr_t pSpriteSheet = RD_SpriteSheetManager::instance()->objectAt( idx );
    if ( pSpriteSheet.isValid() )
    {
      //  Sprite sheet is part of project.
      if ( pSpriteSheet->projectFolder().compare( projectFolder ) == 0 )
      {
        toDelete.insert( RD_SpriteSheetKey(pSpriteSheet->projectFolder(), pSpriteSheet->sheetName(), pSpriteSheet->sheetResolution()) );
      }
    }
  }

  //  Erase matching sprite sheets from manager.
  for ( SpriteSheetKeyCol_t::const_iterator i = toDelete.begin(), iEnd = toDelete.end() ; i!=iEnd ; ++i )
  {
    LOGI( "Unloading Sprite Sheet ('%s':'%s') Resolution: '%s'\n", i->_projectFolder.c_str(), i->_sheetName.c_str(), i->_resolutionName.c_str() );
    RD_SpriteSheetManager::instance()->eraseObject( *i );
  }
}

extern "C" bool EXPORT_API CalculateLocatorTransform( const char *projectFolder, const char *clipName, float frame, const char *locatorName, float *position, float *rotation, float *scale )
{
  RD_ClipDataKey key(projectFolder, clipName);
  RD_ClipDataPtr_t pClipData = RD_ClipDataManager::instance()->object(key);
  if ( pClipData.get() == 0 )
  {
    LOGE( "Clip '%s' has not been loaded in memory!\n", clipName );
    return false;
  }

  for ( unsigned i = 0, iEnd = (unsigned)pClipData->count() ; i<iEnd ; ++i )
  {
    TV_NodeTreeViewPtr_t nodeTreeView = pClipData->nodeTreeView(i);
    TV_NodeTreeViewPtr_t locatorNodeTreeView = nodeTreeView->find( locatorName );

    if ( locatorNodeTreeView.get() )
    {
      float pivotx, pivoty, pivotz;
      locatorNodeTreeView->pivot( frame, pivotx, pivoty, pivotz );

      Math::Matrix4x4 modelMatrix = locatorNodeTreeView->modelMatrix(frame);

      modelMatrix.translate( pivotx, pivoty, pivotz );

      double tx, ty, tz, sx, sy, az, sk;
      if ( modelMatrix.extractParameters( true, true, 0.0, 0.0, tx, ty, tz, sx, sy, az, sk ) )
      {
        position[0] = (float)tx;
        position[1] = (float)ty;
        position[2] = (float)tz;

        rotation[0] = rotation[1] = 0.0f;
        rotation[2] = (float)az;

        scale[0] = (float)sx;
        scale[1] = (float)sy;
        scale[2] = 1.0f;

        return true;
      }
    }
  }

  return false;
}

extern "C" bool EXPORT_API CalculateBoundingBox( int scriptId, float *boundingBox )
{
  RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );
  if ( !pRenderScript.isValid() )
    return false;

  RD_RenderScript::BoundingBox box;
  pRenderScript->calculateBoundingBox( box );

  boundingBox[0] = box._x1;
  boundingBox[1] = box._y1;
  boundingBox[2] = box._x2;
  boundingBox[3] = box._y2;

  return true;
}

extern "C" bool EXPORT_API CalculateConvexHull( int scriptId, float **convexHullArray, int *convexHullSize )
{
  RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );
  if ( !pRenderScript.isValid() )
    return false;

  RD_RenderScript::BoundingVertex *convexHullArray2 = 0;
  pRenderScript->calculateConvexHull( convexHullArray2 , *convexHullSize );

  *convexHullArray = reinterpret_cast<float*>( convexHullArray2 );

  return ( *convexHullArray != NULL );
}

extern "C" void EXPORT_API DeallocateConvexHull( float *convexHullArray )
{
  delete [] convexHullArray;
}

extern "C" bool EXPORT_API CalculatePolygons( int scriptId, float **polygonArray, int *polygonSize, int **subPolygonArray, int *subPolygonSize )
{
  RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );
  if ( !pRenderScript.isValid() )
    return false;

  RD_RenderScript::BoundingVertex *polygonArray2 = 0;
  pRenderScript->calculatePolygons( polygonArray2 , *polygonSize, *subPolygonArray, *subPolygonSize );

  *polygonArray = reinterpret_cast<float*>( polygonArray2 );

  return ( *polygonArray != NULL );
}

extern "C" void EXPORT_API DeallocatePolygons( float *polygonArray, int *subPolygonArray )
{
  delete [] polygonArray;
  delete [] subPolygonArray;
}

extern "C" int EXPORT_API CreateProp( int scriptId, const char *projectFolder, const char *clipName, const char *playName )
{
  LOGI( "Creating prop for play sequence '%s' in clip '%s'\n", playName, clipName );
  RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );
  if ( pRenderScript.get() == 0 )
  {
    LOGE( "No valid rendering script for script id '0x%x'\n", scriptId );
    return -1;
  }

  RD_ClipDataKey key(projectFolder, clipName);
  RD_ClipDataPtr_t pPropData = RD_ClipDataManager::instance()->object(key);
  if ( pPropData.get() == 0 )
  {
    LOGE( "Prop clip '%s' has not been loaded.\n", clipName );
    return -1;
  }

  TV_NodeTreeViewPtr_t propNodeTreeView = pPropData->nodeTreeView(playName);
  if (!propNodeTreeView.isValid())
  {
    LOGE( "No valid hierarchy for prop '%s' in prop clip '%s'\n", playName, clipName );
    return -1;
  }

  return pRenderScript->createProp(playName, propNodeTreeView);
}

extern "C" void EXPORT_API UpdateProp( int scriptId, int propId, float frame )
{
  LOGI( "Updating propId '0x%x'\n", propId );
  RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );
  if ( !pRenderScript.isValid() )
  {
    LOGE( "Could not find script with id '0x%x'\n", scriptId );
    return;
  }

  pRenderScript->updateProp(propId, frame);
}

extern "C" void EXPORT_API AnchorProp( int scriptId, int propId, const char *playName, const char *nodeName )
{
  LOGI( "Anchoring propId '0x%x' to '%s' in parent play sequence '%s'\n", propId, nodeName, playName );
  RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );
  if ( !pRenderScript.isValid() )
  {
    LOGE( "Could not find script with id '0x%x'\n", scriptId );
    return;
  }

  pRenderScript->anchorProp(propId, playName, nodeName);
}

extern "C" void EXPORT_API UnanchorProp( int scriptId, int propId )
{
  LOGI( "Unanchoring propId '0x%x'\n", propId);
  RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );
  if ( !pRenderScript.isValid() )
  {
    LOGE( "Could not find script with id '0x%x'\n", scriptId );
    return;
  }

  pRenderScript->unanchorProp(propId);
}

void CallCameraEvent( int eventID )
{
  RD_CameraEventPtr_t pEvent;
  pEvent = RD_CameraEventManager::instance()->eraseObject( eventID );

  if ( pEvent.isValid() )
  {
    //  Render current batch using camera parameters.
    g_renderer->beginRender();
    g_batch.render( g_renderer, pEvent->cullingMask(), pEvent->projectionMatrix(), pEvent->viewMatrix() );
    g_renderer->endRender();
  }
}

void CallRenderTextureEvent()
{
  if ( RD_RenderTextureEventManager::instance()->size() > 0 )
  {
    g_renderer->beginRender();

    RD_RenderTextureEventPtr_t pEvent;
    while ( (pEvent = RD_RenderTextureEventManager::instance()->popFirst()).get() )
    {
      pEvent->render( g_renderer );
    }

    g_renderer->endRender();
  }
}

extern "C" void EXPORT_API InitializeRenderer( int deviceType )
{
  LOGI( "InitializeRenderer(%i)\n", deviceType );

  if ( g_rendererType != deviceType )
  {
    delete g_renderer;
    g_renderer = 0;
  }

  if ( g_renderer == 0 )
  {
#ifdef SUPPORT_RENDERER_GL
    if (deviceType == kGfxRendererOpenGL)
      g_renderer = new RD_RendererGLES2;

    else if (deviceType == kGfxRendererOpenGLCore)
      g_renderer = new RD_RendererGLCore;
#endif

#ifdef SUPPORT_RENDERER_GLES2
    if ( (deviceType == kGfxRendererOpenGLES20) ||
         (deviceType == kGfxRendererOpenGLES30) )
      g_renderer = new RD_RendererGLES2;
#endif

#ifdef SUPPORT_RENDERER_METAL
    if (deviceType == kGfxRendererMetal)
      g_renderer = new RD_RendererMTL;
#endif

#ifdef SUPPORT_RENDERER_DX9
    if (deviceType == kGfxRendererD3D9)
      g_renderer = new RD_RendererDX9;
#endif

#ifdef SUPPORT_RENDERER_DX11
    if (deviceType == kGfxRendererD3D11)
      g_renderer = new RD_RendererDX11;
#endif

    if ( g_renderer )
    {
      g_renderer->initialize();
      g_rendererType = deviceType;
    }
  }
}

extern "C" void EXPORT_API HARMONY_SET_GRAPHICS_DEVICE_FUNC (void* device, int deviceType, int eventType)
{
  LOGI( "HarmonySetGraphicsDevice (device: '%p') (deviceType: '%i') (eventType: '%i')\n", device, deviceType, eventType );
  
  InitializeRenderer(deviceType);

  //  Update renderer device if available.
  if ( g_renderer )
  {
    g_renderer->setDevice( device, eventType );
  }
}

extern "C" void EXPORT_API HARMONY_RENDER_EVENT_FUNC (int eventID)
{
  if (g_renderer == 0)
    return;

  if ( eventID == HARMONY_RENDERER_FINAL_CLEANUP )
  {
    g_renderer->shutdown();
    return;
  }
  else if ( eventID == HARMONY_RENDERER_CLEANUP )
  {
    g_renderer->cleanup();
    return;
  }
  else if ( eventID == HARMONY_RENDERER_RENDER_TEXTURE )
  {
    CallRenderTextureEvent();
    return;
  }

  CallCameraEvent(eventID);

}

#ifdef USE_ANIMATION_BLENDING

TV_NodeTreeViewPtr_t findNodeTreeViewForNameBuilder( const TV_NodeTreeViewPtr_t &nodeTreeView , STD_String &nodeToFind)
{
	STD_String nodeName = nodeTreeView->name();
	if (nodeName.compare(nodeToFind)==0){
		return nodeTreeView;
	}
	TV_NodeTreeViewPtr_t nodeTreeViewPtr = 0;
	for ( TV_NodeTreeView::BrotherIterator i = nodeTreeView->childBegin(), iEnd = nodeTreeView->childEnd() ; i!=iEnd ; ++i )
    {
      nodeTreeViewPtr = findNodeTreeViewForNameBuilder(&*i,nodeToFind);
	  if (nodeTreeViewPtr){
		  return nodeTreeViewPtr;
	  }
    }
	return nodeTreeViewPtr;
}


void findConnectionForTreeViewInToTreeViewRootForClipName(int blendID,const TV_NodeTreeViewPtr_t &focusNodeTreeView,const TV_NodeTreeViewPtr_t &toNodeTreeViewRoot,std::string clipName,std::map<std::string,TV_NodeTreeViewPtr_t> &tempMap){
	STD_String focusNodeName = focusNodeTreeView->name();

	////for debugging purposes:
	//std::ofstream myfile;
	//  myfile.open ("hierarchyDebugging.txt",std::ios::app);
	//  myfile << focusNodeName<<std::endl;;
	//  myfile.close();


	TV_NodeTreeViewPtr_t focusPair = findNodeTreeViewForNameBuilder(toNodeTreeViewRoot,focusNodeName);
	if (tempMap.find(focusNodeName) != tempMap.end()){
		//object with name already found.
		SingletonBlending::getInstance()->blendingIdenticalNameTracker[blendID] = true;
		return;
	}
	if (focusPair){
		tempMap[focusNodeName] = focusPair;
	}else{
		//missing object found.
		SingletonBlending::getInstance()->blendingMissingObjectTracker[blendID] = true;
	}
}

//iterate through the full FromNodeTreeViewRoot
void iterateThroughNodeTreeViewForClipName(int blendID,const TV_NodeTreeViewPtr_t &fromNodeTreeView,const TV_NodeTreeViewPtr_t &toNodeTreeViewRoot, std::string clipName, std::map<std::string,TV_NodeTreeViewPtr_t> &tempMap)
{
	findConnectionForTreeViewInToTreeViewRootForClipName(blendID,fromNodeTreeView,toNodeTreeViewRoot,clipName,tempMap);
	for ( TV_NodeTreeView::BrotherIterator i = fromNodeTreeView->childBegin(), iEnd = fromNodeTreeView->childEnd() ; i!=iEnd ; ++i )
    {
      iterateThroughNodeTreeViewForClipName(blendID,&*i,toNodeTreeViewRoot,clipName,tempMap);
    }
}

void calculateMemberCountOfTree(const TV_NodeTreeViewPtr_t &fromNodeTreeView , int &memberCount){
	memberCount++;
	for ( TV_NodeTreeView::BrotherIterator i = fromNodeTreeView->childBegin(), iEnd = fromNodeTreeView->childEnd() ; i!=iEnd ; ++i )
    {
      calculateMemberCountOfTree(&*i,memberCount);
    }
}

void BuildConnectionBetweenFromAndTo(int blendID, const RD_ClipDataPtr_t &pClipDataFrom,const RD_ClipDataPtr_t &pClipDataTo , std::string clipName, std::map<std::string,TV_NodeTreeViewPtr_t> &tempMap){
	int size = SingletonBlending::getInstance()->BlendingContainer.size();
	if (size > 0){
		return;
	}
	TV_NodeTreeViewPtr_t fromNodeTreeView = pClipDataFrom->nodeTreeView(0);
	TV_NodeTreeViewPtr_t toNodeTreeViewRoot = pClipDataTo->nodeTreeView(0);

	int fromMemberCount = 0;
	int toMemberCount = 0;

	calculateMemberCountOfTree(fromNodeTreeView,fromMemberCount);
	calculateMemberCountOfTree(toNodeTreeViewRoot,toMemberCount);

	if (fromMemberCount != toMemberCount){
		SingletonBlending::getInstance()->blendingMissingObjectTracker[blendID] = false;
	}

	iterateThroughNodeTreeViewForClipName(blendID,fromNodeTreeView,toNodeTreeViewRoot,clipName,tempMap);
	size = SingletonBlending::getInstance()->BlendingContainer.size();
}

extern "C" bool EXPORT_API MissingObjectFoundForBlending(int blendID){
	return SingletonBlending::getInstance()->blendingMissingObjectTracker[blendID];
}

extern "C" bool EXPORT_API IdenticallyNamedObjectsFoundForBlending(int blendID){
	return SingletonBlending::getInstance()->blendingIdenticalNameTracker[blendID];
}

extern "C" int EXPORT_API BuildConnectionForBlending(const char *projectFolder, const char *clipNameFrom, const char *clipNameTo){
	RD_ClipDataKey keyFrom(projectFolder, clipNameFrom);
	RD_ClipDataPtr_t pClipDataFrom = RD_ClipDataManager::instance()->object(keyFrom);
	if ( pClipDataFrom.get() == 0 )
	{
		LOGE( "Clip '%s' has not been loaded in memory!\n", clipNameFrom );
		return -1;
	}

	RD_ClipDataKey keyTo(projectFolder, clipNameTo);
	RD_ClipDataPtr_t pClipDataTo = RD_ClipDataManager::instance()->object(keyTo);
	if ( pClipDataTo.get() == 0 )
	{
		LOGE( "Second Clip '%s' has not been loaded in memory!\n", clipNameTo );
		return -1;
	}
	
	int blendID = RD_RenderId::uniqueId();

	SingletonBlending::getInstance()->blendingIdenticalNameTracker[blendID] = false;
	SingletonBlending::getInstance()->blendingMissingObjectTracker[blendID] = false;

	std::map<std::string,TV_NodeTreeViewPtr_t> tempMap;
	BuildConnectionBetweenFromAndTo(blendID,pClipDataFrom,pClipDataTo,clipNameFrom,tempMap);
	SingletonBlending::getInstance()->BlendingContainer[blendID] = tempMap;
	return blendID;
}

extern "C" int EXPORT_API UpdateRenderScriptWithBlending( int scriptId, const char *projectFolder, const char *clipNameFrom, const char *clipNameTo, const char *sheetResolution, float frameFrom, float frameTo, unsigned int color, int discretizationStep, float fullBlendTime, float currentBlendTime,int blendID)
{
	RD_RenderScriptPtr_t pRenderScript = RD_RenderScriptManager::instance()->object( scriptId );

	RD_ClipDataKey keyFrom(projectFolder, clipNameFrom);
	RD_ClipDataPtr_t pClipDataFrom = RD_ClipDataManager::instance()->object(keyFrom);
	if ( pClipDataFrom.get() == 0 )
	{
		LOGE( "Clip '%s' has not been loaded in memory!\n", clipNameFrom );
		return -1;
	}

	RD_ClipDataKey keyTo(projectFolder, clipNameTo);
	RD_ClipDataPtr_t pClipDataTo = RD_ClipDataManager::instance()->object(keyTo);
	if ( pClipDataTo.get() == 0 )
	{
		float nFrames = LoadStageClip(projectFolder, clipNameTo, sheetResolution);
		pClipDataTo = RD_ClipDataManager::instance()->object(keyTo);

		if ( nFrames == 0.0f )
		{
			UnloadStageClip(projectFolder, clipNameTo);
			LOGE( "Second Clip '%s' has not been loaded in memory!\n", clipNameTo );
			UpdateRenderScript(scriptId,projectFolder,clipNameFrom,sheetResolution,frameFrom,color,discretizationStep);
			return -1;
		}
	}
	
	if ( pClipDataFrom->count() > 0 )
	{
		bool newScript = false;

		RD_RenderScript::Feature requestedFeatures = RD_RenderScript::ePlainFeature;
		if ( pClipDataFrom->fxEnabled() )
			requestedFeatures = RD_RenderScript::Feature(requestedFeatures | RD_RenderScript::eCutterFeature | RD_RenderScript::eDeformationFeature);

		//  Todo.  Implement a proper factory class to instantiate render scripts depending on
		//  requested features.
		if ( pRenderScript.isValid() )
		{
			if ( !pRenderScript->supportsFeature(requestedFeatures) )
			{
				//  Forward meta information to new script...
				RD_RenderScriptMetaPtr_t pMeta = pRenderScript->meta();

				//  Create new render script to support requested features.
				RD_RenderScriptManager::instance()->eraseObject( scriptId );

				//  Plain Script
				if ( testFeatures(requestedFeatures, RD_RenderScriptPlain::supportedFeatures()) )
				{
					pRenderScript = new RD_RenderScriptPlain(pMeta);
					newScript = true;
				}
				//  Fx Script
				else // if ( ( testFeatures(requestedFeatures, RD_RenderScriptFx::supportedFeatures()) )
				{
					pRenderScript = new RD_RenderScriptFx(pMeta);
					newScript = true;
				}
			}
		}
		else
		{
			//  Plain Script
			if (  testFeatures(requestedFeatures, RD_RenderScriptPlain::supportedFeatures()) )
			{
				pRenderScript = new RD_RenderScriptPlain;
				newScript = true;
			}
			//  Fx Script
			else // if (  testFeatures(requestedFeatures, RD_RenderScriptFx::supportedFeatures()) )
			{
				pRenderScript = new RD_RenderScriptFx;
				newScript = true;
			}
		}
		
		pRenderScript->updateWithBlending(pClipDataFrom,projectFolder,sheetResolution, frameFrom, frameTo, color, discretizationStep ,fullBlendTime,currentBlendTime,blendID);
		//pRenderScript->update( pClipDataFrom, projectFolder, sheetResolution, frame, color, discretizationStep );

		if ( newScript )
		{
			RD_RenderScriptManager::instance()->addObject( scriptId, pRenderScript );
		}

		return scriptId;
	}

	return -1;
}

extern "C" void EXPORT_API ClearBlendingValues(int blendID){
	SingletonBlending::getInstance()->BlendingContainer[blendID].clear();
	SingletonBlending::getInstance()->BlendingContainer.erase(blendID);
	SingletonBlending::getInstance()->blendingIdenticalNameTracker.erase(blendID);
	SingletonBlending::getInstance()->blendingMissingObjectTracker.erase(blendID);
}

#endif
