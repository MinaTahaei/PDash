
#if (UNITY_EDITOR || UNITY_STANDALONE_WIN || UNITY_STANDALONE_OSX)
#define UNITY_MULTITHREADED
#else
#undef  UNITY_MULTITHREADED
#endif

#if (UNITY_ANDROID && !UNITY_EDITOR)
#undef UNITY_ISSUE_RENDER_EVENT
#else
#define UNITY_ISSUE_RENDER_EVENT
#endif

using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

/*!
 *  @class HarmonyCamera
 *  Game object component to be used in a Camera.  This script will initiate
 *  rendering in the HarmonyRenderer plugin.
 */
[ExecuteInEditMode]
[AddComponentMenu("Harmony/Core/HarmonyCamera")]
public class HarmonyCamera : MonoBehaviour
{

  private class Internal
  {
#if (UNITY_IPHONE || UNITY_XBOX360 || UNITY_WEBGL) && !UNITY_EDITOR

#if !UNITY_ISSUE_RENDER_EVENT || UNITY_WEBGL // Use for android
        [DllImport ("__Internal")]
        public static extern void InitializeRenderer (int deviceType);

        [DllImport ("__Internal")]
        public static extern void UnityRenderEvent (int eventID);
#endif // !UNITY_ISSUE_RENDER_EVENT

      [DllImport ("__Internal")]
      public static extern int CreateCameraEvent( int unityFrame, int cullingMask, float[] projection, float[] view);

#else // !((UNITY_IPHONE || UNITY_XBOX360) && !UNITY_EDITOR)

#if !UNITY_ISSUE_RENDER_EVENT // Use for android
        [DllImport ("HarmonyRenderer")]
        public static extern void InitializeRenderer (int deviceType);

        [DllImport ("HarmonyRenderer")]
        public static extern void UnityRenderEvent (int eventID);
#endif // !UNITY_ISSUE_RENDER_EVENT

        [DllImport ("HarmonyRenderer")]
      public static extern int CreateCameraEvent( int unityFrame, int cullingMask, float[] projection, float[] view);

    #endif
  }

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

  public enum RenderOrder
  {
    ePreCamera,
    ePostCamera,
    eEndOfFrame
  };

  public RenderOrder renderOrder = RenderOrder.ePostCamera;

  private bool       renderingWithDirectX = false;

  private static int HARMONY_RENDERER_CLEANUP = 1;
  //private static int HARMONY_RENDERER_FINAL_CLEANUP = 2;
  private static int HARMONY_RENDERER_RENDER_TEXTURE = 3;

  IEnumerator Start()
  {
    PluginInitialize();

    //Message.Log( "Graphics Device Version: " + SystemInfo.graphicsDeviceVersion );
    if ( SystemInfo.graphicsDeviceVersion.Contains( "Direct3D" ) )
      renderingWithDirectX = true;

    yield return StartCoroutine("CallPluginAtEndOfFrames");
  }

  private void OnPreCull()
  {
    //  Somehow using OnPreRender to call a plugin event makes other
    //  game object disappear on Windows.  Still need to investigate.

    //  Render selected game object to texture before Unity renders
    //  its own geometry so that texture is up to date at that time.
    PluginRenderToTexture();
  }

  private void OnPreRender()
  {
    //  Will render plugin before scene renders itself.  Don't
    //  clear with skybox if you want to see something.
    if ( renderOrder == RenderOrder.ePreCamera )
    {
      PluginRenderFrame();
    }
  }

  private void OnPostRender()
  {
    //  Will render plugin after scene renders itself but before
    //  ui renders.
    if ( renderOrder == RenderOrder.ePostCamera )
    {
      PluginRenderFrame();
    }
  }

  private void OnApplicationQuit()
  {
    // GL.IssuePluginEvent (HARMONY_RENDERER_FINAL_CLEANUP);
  }

  private IEnumerator CallPluginAtEndOfFrames ()
  {
    while (true) {

      // Wait until all frame rendering is done
      yield return new WaitForEndOfFrame();

      //  Will render plugin after all cameras finished rendering and
      //  after ui.
      if ( renderOrder == RenderOrder.eEndOfFrame )
      {
        PluginRenderFrame();
      }
    }
  }

  private void PluginInitialize()
  {
#if !UNITY_ISSUE_RENDER_EVENT || (UNITY_WEBGL && !UNITY_EDITOR)
    //  Hardcoded OpenGL ES 2.0 Device (Android).
    Internal.InitializeRenderer( (int)GfxDeviceRenderer.kGfxRendererOpenGLES20 );
    GL.InvalidateState();
#endif
  }

  private void PluginRenderFrame()
  {
    Camera camera = GetComponent<Camera>();
    Matrix4x4 projectionMatrix = camera.projectionMatrix;
    Matrix4x4 viewMatrix = camera.worldToCameraMatrix;

    //  Unless we find a better way of handling that case.
    //  Deferred lighting in unity renders in an offline render target and then
    //  flips it (along with our plugin rendering) in the final render.
    if ( (camera.actualRenderingPath == RenderingPath.DeferredLighting) &&
         renderingWithDirectX )
    {
      Matrix4x4 scaleMatrix = Matrix4x4.Scale( new Vector3(1.0f, -1.0f, 1.0f) );
      projectionMatrix = projectionMatrix * scaleMatrix;
    }

    int unityFrame = Time.frameCount;
    int renderId = Internal.CreateCameraEvent( unityFrame, camera.cullingMask, MatrixToArray(projectionMatrix), MatrixToArray(viewMatrix) );

#if UNITY_ISSUE_RENDER_EVENT && (!UNITY_WEBGL || UNITY_EDITOR)
    GL.IssuePluginEvent(renderId);
#else
    Internal.UnityRenderEvent(renderId);
    GL.InvalidateState();
#endif
  }

  public static void PluginCleanup()
  {
    //  Ask for opengl buffers cleanup in its own thread.
#if UNITY_ISSUE_RENDER_EVENT && (!UNITY_WEBGL || UNITY_EDITOR)
    GL.IssuePluginEvent(HARMONY_RENDERER_CLEANUP);
#else
    Internal.UnityRenderEvent(HARMONY_RENDERER_CLEANUP);
#endif
  }

  public static void PluginRenderToTexture()
  {
#if UNITY_ISSUE_RENDER_EVENT && (!UNITY_WEBGL || UNITY_EDITOR)
    GL.IssuePluginEvent(HARMONY_RENDERER_RENDER_TEXTURE);
#else
    Internal.UnityRenderEvent(HARMONY_RENDERER_RENDER_TEXTURE);
    GL.InvalidateState();
#endif    
  }

  private static float[] MatrixToArray(Matrix4x4 matrix)
  {
    float[] result = new float[16];
    for (int row = 0; row < 4; row++)
    {
      for (int col = 0; col < 4; col++)
      {
        result[col + row * 4] = matrix[col, row];
      }
    }

    return result;
  }

}
