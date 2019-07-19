
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.IO;
using System;

using WebGL_FS;

/*!
 *  @class HarmonyRenderer
 *  Main Harmony game object component.
 *  This script will schedule rendering events for the current game object
 *  and will synchronize with the other Harmony game object components to
 *  update the character display.
 */
[ExecuteInEditMode]
[AddComponentMenu("Harmony/Core/HarmonyRenderer")]
public class HarmonyRenderer : MonoBehaviour, ISerializationCallbackReceiver
{
  /*!
   * @enum RenderTarget
   */
  public enum RenderTarget
  {
    eScreen,                        //  Render directly to screen.
    eRenderTexture                  //  Render to existing Unity texture.  Requires renderable object attached to current game object.
  };

  /*!
   * @enum SyncCollider
   */
  public enum SyncCollider
  {
    eNever,                        //  Never synchronize collider bounding box.
    eOnce,                         //  Only synchronize collider bounding box once.
    eAlways                        //  Synchronize collider bounding box at every Update.
  };

  /*!
   * @enum ColliderShape
   */
  public enum ColliderShape
  {
    eBox,                          //  screen aligned bounding box.
    eConvexHull,                   //  a single convex hull bounding shape.
    ePolygons                      //  tight polygon fitting shapes.
  };

  //  Color of the layer.  Will be multiplied with the texture color.
  public Color         color = Color.white;

  //  Project folder used as basis for this game object.
  public string        projectFolder = "";    //  Must not be changed after initialization.

  //  Collection of clip names that will be used in animation.
  public string[]      clipNames;             //  Must not be changed after initialization.

  //  Sheet resolution used at rendering.
  public string        sheetResolution = "default";

  //  Depth of layer.  Used to sort HarmonyRenderer layers against each others.
  public int           depth = 0;

  public int           discretizationStep = 20;

  //  Render target mode.
  public RenderTarget  renderTarget = RenderTarget.eScreen;

  //  Texture viewport used in eRenderTexture mode.
  public Rect          renderTextureViewport = new Rect(0, 0, 640, 480);

  //  Texture scale factor.
  public float         renderTextureScale = 1.0f;

  //  Synchronize collider mode.
  public SyncCollider  syncCollider = SyncCollider.eNever;

  //  Collider shape.
  public ColliderShape colliderShape = ColliderShape.eBox;

  //  Collider trigger.  Will create new collider with "Is Trigger" set to specified value.
  public bool colliderTrigger = false;

  public Dictionary<string, HarmonyMeta> metadata = new Dictionary<string, HarmonyMeta>();

  public string currentProjectFolder
  {
    get
    {
      return liveProjectFolder;
    }
  }

  public string currentClipName
  {
    get
    {
      if (clipIdx < clipNames.Length)
        return clipNames[clipIdx];

      return null;
    }
  }

  public int currentClipIndex
  {
    get
    {
      return clipIdx;
    }
  }

  public float currentFrame
  {
    get
    {
      return lastFrame;
    }
  }

  public HarmonyMeta currentMetadata
  {
    get
    {
      if ( metadata.ContainsKey(currentClipName) )
        return metadata[currentClipName];

      return null;
    }
  }

  [SerializeField] private List<string> metadataKeys;
  [SerializeField] private List<HarmonyMeta> metadataLists;

  /*!
   * @struct ClipData
   */
  private struct ClipData
  {
    public float nFrames;
  }

  private Dictionary<string, ClipData> loadedClips = new Dictionary<string, ClipData>();
  private static Dictionary< string, Dictionary< string, int > > g_clipNamesRefCount = new Dictionary< string, Dictionary< string, int > >();

  //  Resolved project folder
  private string           liveProjectFolder = "";

  //  Current clip index.
  private int              clipIdx = 0; //  Must not be changed after initialization.  Should be handled through animation module.

  private Rect             liveTextureViewport = new Rect(0, 0, 0, 0);

  private bool             isColliderSynced = false;

  /*!
   * @struct ColliderKey
   */
  private struct ColliderKey
  {
    public override string ToString()
    {
      return colliderShape.ToString() + clipName + Math.Floor(frame).ToString();
    }

    public string          clipName;
    public float           frame;
    public ColliderShape   colliderShape;
  }

  [SerializeField] private List<string> colliderKeys = new List<string>();
  [SerializeField] private List<Collider2D> colliderLinks = new List<Collider2D>();

  private Collider2D       currentCollider = null;

  private Dictionary<int, HarmonyProp> harmonyProps = new Dictionary<int, HarmonyProp>();
  private bool             harmonyPropsDirty = true;

  //  As much as I could gather while scavenging through Unity
  //  documentation, Unity multithreaded pipeline will not 
  //  precalculate frame before current rendering is finished.
  //  Therefore, there is no need for multiple scripts.
  private int              scriptId = -1;
  private int              scriptIdToRender = -1;

  private float            lastFrame = 0.0f;
  private uint             lastColor = 0;
  #if UNITY_EDITOR
  private int              lastDiscretizationStep = 0;
  #endif

  private bool             renderingWithDirectX = false;

  //  Renderer Texture Id/Ptr backup for efficiency.
  private IntPtr           rendererTexturePtr = new IntPtr(0);

    public bool readyToRender = true;

#if UNITY_EDITOR
  private float[]          interactiveBox = new float[4];
#endif

  private class Internal
  {
#if (UNITY_IPHONE || UNITY_XBOX360 || UNITY_WEBGL) && !UNITY_EDITOR

    [DllImport ("__Internal")]
    public static extern float LoadStageClip( string projectFolder, string clipName, string sheetResolution );

    [DllImport ("__Internal")]
    public static extern void UnloadStageClip( string projectFolder, string clipName );

    [DllImport ("__Internal")]
    public static extern void UnloadProject( string projectFolder );

    [DllImport ("__Internal")]
    public static extern int UpdateRenderScript( int scriptId, string projectFolder, string clipName, string sheetResolution, float frame, uint color, int discretizationStep );

    [DllImport ("__Internal")]
    public static extern int CreateRenderScript( string projectFolder, string clipName );

    [DllImport ("__Internal")]
    public static extern void UnloadRenderScript( int scriptId );

    [DllImport ("__Internal")]
    public static extern void CreateRenderEvent( int unityFrame, int depth, int scriptId, int layerMask, float[] model );

    [DllImport ("__Internal")]
    public static extern void CreateRenderTextureEvent( int scriptId, float offsetx, float offsety, float width, float height, float scale, System.IntPtr texture );

    [DllImport ("__Internal")]
    public static extern bool CalculateBoundingBox( int scriptId, [In, Out] float[] box );

    [DllImport ("__Internal")]
    public static extern bool CalculateConvexHull( int scriptId, ref IntPtr convexHullArray, ref int convexHullSize );

    [DllImport ("__Internal")]
    public static extern void DeallocateConvexHull( IntPtr convexHullArray );

    [DllImport ("__Internal")]
    public static extern bool CalculatePolygons( int scriptId, ref IntPtr polygonArray, ref int polygonsSize, ref IntPtr subPolygonArray, ref int subPolygonSize );

    [DllImport ("__Internal")]
    public static extern void DeallocatePolygons( IntPtr polygonArray, IntPtr subPolygonArray );

#else

    [DllImport ("HarmonyRenderer")]
    public static extern float LoadStageClip( string projectFolder, string clipName, string sheetResolution );

    [DllImport ("HarmonyRenderer")]
    public static extern void UnloadStageClip( string projectFolder, string clipName );

    [DllImport ("HarmonyRenderer")]
    public static extern void UnloadProject( string projectFolder );

    [DllImport ("HarmonyRenderer")]
    public static extern int UpdateRenderScript( int scriptId, string projectFolder, string clipName, string sheetResolution, float frame, uint color, int discretizationStep );

    [DllImport ("HarmonyRenderer")]
    public static extern int CreateRenderScript( string projectFolder, string clipName );

    [DllImport ("HarmonyRenderer")]
    public static extern void UnloadRenderScript( int scriptId );

    [DllImport ("HarmonyRenderer")]
    public static extern void CreateRenderEvent( int unityFrame, int depth, int scriptId, int layerMask, float[] model );

    [DllImport ("HarmonyRenderer")]
    public static extern void CreateRenderTextureEvent( int scriptId, float offsetx, float offsety, float width, float height, float scale, System.IntPtr texture );

    [DllImport ("HarmonyRenderer")]
    public static extern bool CalculateBoundingBox( int scriptId, [In, Out] float[] box );

    [DllImport ("HarmonyRenderer")]
    public static extern bool CalculateConvexHull( int scriptId, ref IntPtr convexHullArray, ref int convexHullSize );

    [DllImport ("HarmonyRenderer")]
    public static extern void DeallocateConvexHull( IntPtr convexHullArray );

    [DllImport ("HarmonyRenderer")]
    public static extern bool CalculatePolygons( int scriptId, ref IntPtr polygonArray, ref int polygonsSize, ref IntPtr subPolygonArray, ref int subPolygonSize );

    [DllImport ("HarmonyRenderer")]
    public static extern void DeallocatePolygons( IntPtr polygonArray, IntPtr subPolygonArray );

#endif
  }

  void Awake()
  {
    //  Create render texture.
    Renderer renderer = GetComponent<Renderer>();
    if ( (renderer != null) && (renderer.sharedMaterial != null) && (renderTextureViewport.width != 0) && (renderTextureViewport.height != 0) )
    {
      //  Using RenderTexture instead of Texture2D as it resides in default memory pool in directx and allows
      //  direct rendering to texture.
      RenderTexture rendererTexture = new RenderTexture((int)renderTextureViewport.width, (int)renderTextureViewport.height, 16, RenderTextureFormat.ARGB32);
      rendererTexture.filterMode = FilterMode.Bilinear;
      rendererTexture.Create();

#if UNITY_EDITOR
      Graphics.SetRenderTarget(rendererTexture);
      GL.Clear(true, true, Color.clear);
      Graphics.SetRenderTarget(null);
#endif

      // Set texture onto our material
      if (Application.isPlaying)
      {
        UnityEngine.Object.Destroy(renderer.sharedMaterial.mainTexture);
      }
      else
      {
        UnityEngine.Object.DestroyImmediate(renderer.sharedMaterial.mainTexture);
      }

      renderer.sharedMaterial.mainTexture = rendererTexture;
    }

    //  Disable all linked colliders.  They will be re-enabled through rendering.
    if (Application.isPlaying)
    {
      foreach ( Collider2D collider in colliderLinks )
      {
        collider.enabled = false;
      }
    }

    //Message.Log( "Graphics Device Version: " + SystemInfo.graphicsDeviceVersion );
    if ( SystemInfo.graphicsDeviceVersion.Contains( "Direct3D" ) )
      renderingWithDirectX = true;

    LoadClipIndex(clipIdx);
  }

	public void OnBeforeSerialize()
	{
    //  Serialize Metadata Dictionary. 
    metadataKeys = new List<string>();
    metadataLists = new List<HarmonyMeta>();

		foreach(KeyValuePair<string, HarmonyMeta> kvp in metadata)
		{
			metadataKeys.Add(kvp.Key);
			metadataLists.Add(kvp.Value);
		}
  }

  public void OnAfterDeserialize()
	{
    //  Deserialize Metadata Dictionary.
    if (metadataKeys == null)
      return;
    if (metadataLists == null)
      return;

    metadata = new Dictionary<string, HarmonyMeta>();
		for (int i=0 ; i!= Math.Min(metadataKeys.Count,metadataLists.Count) ; ++i)
    {
			metadata.Add(metadataKeys[i],metadataLists[i]);
    }
  }

  private bool ResolveProjectFolder()
  {
    if ( string.IsNullOrEmpty(projectFolder) )
      return false;

#if (UNITY_WEBGL) && !UNITY_EDITOR
        if (WebGL_FileSystem.ResourceReadyCheck(projectFolder))
        {
            liveProjectFolder = projectFolder;
            return true;
        }else{
            return false;
        }
#endif

    if ( new DirectoryInfo(projectFolder).Exists )
    {
      //this should be activated on webgl as well
      liveProjectFolder = projectFolder;
      return true;
    }

    //  Try to find a valid directory.  If projectFolder does not exist, append
    //  persistent data path to it and test again.
    if ( !new DirectoryInfo(projectFolder).Exists )
    {
      string altProjectFolder = Application.persistentDataPath + "/" + projectFolder;
      if ( new DirectoryInfo(altProjectFolder).Exists )
      {
        liveProjectFolder = altProjectFolder;
        return true;
      }
    }

    //  Try to find a valid directory.  If projectFolder does not exist, append
    //  streaming asset path to it and test again.
    if ( !new DirectoryInfo(projectFolder).Exists )
    {
      string altProjectFolder = Application.streamingAssetsPath + "/" + projectFolder;

#if UNITY_ANDROID
      //  streamingAssetsPath is a url (android), download assets locally.
      if ( ZIP_AndroidAssets.isUrl( altProjectFolder ) )
      {
        string tmpProjectFolder;
        if ( ZIP_AndroidAssets.synchronizeProjectFolder( altProjectFolder, out tmpProjectFolder ) )
        {
          liveProjectFolder = tmpProjectFolder;
          return true;
        }
      }
      //  otherwise, a direct path on disk.
      else
#endif
      {
        if ( new DirectoryInfo(altProjectFolder).Exists )
        {
          liveProjectFolder = altProjectFolder;
          return true;
        }
      }
    }

    return false;
  }

  public void SetClipName( string clipName )
  {
    if ( clipNames == null )
      return;

    int index = Array.IndexOf( clipNames, clipName );
    if ( index < 0 )
      return;

    SetClipIndex(index);
  }

  public void SetClipIndex( int index )
  {
    if ( index == clipIdx )
      return;

    if ( LoadClipIndex(index) )
    {
      clipIdx = index;

      //  Reset cached parameters.
      lastFrame = 0.0f;
      lastColor = 0;
    }
  }

  public bool LoadClipName( string clipName )
  {
    if ( string.IsNullOrEmpty(clipName) )
    {
      Message.LogWarning( "Invalid clip name!" );
      return false;
    }

    //  If clip has already been loaded, bail out.
    if ( loadedClips.ContainsKey( clipName ) )
      return true;

    if ( !ResolveProjectFolder() )
    {
      Message.LogWarning( "Could not find '" + projectFolder + "' on disk!" );
      return false;
    }

    float nFrames = Internal.LoadStageClip(liveProjectFolder, clipName, sheetResolution );
    if ( nFrames == 0.0f )
    {
      Message.LogWarning( "Clip '" + clipName + "' has no animation data.\n" +
                        "Is your asset folder missing information ?\n" +
                        "Possible reasons for this are:\n" +
                        "  - Clip name does not exist.  Look in the stage.xml file for valid clip names.\n" +
                        "  - Binary file version mismatch.  Make sure the output from Xml2Bin and the HarmonyRenderer plugin come from the same package.\n" +
                        "  - Incomplete XML Data.  If you are using the xml data, make sure skeleton.xml, animation.xml and drawingAnimation.xml files are in the folder.\n"
                        );
            //return false;
    }

    ClipData clipData = new ClipData();
    clipData.nFrames = nFrames;

    loadedClips[ clipName ] = clipData;

    if ( !g_clipNamesRefCount.ContainsKey(liveProjectFolder) )
      g_clipNamesRefCount[liveProjectFolder] = new Dictionary<string, int>();

    if ( g_clipNamesRefCount[liveProjectFolder].ContainsKey(clipName) )
      ++g_clipNamesRefCount[liveProjectFolder][clipName];
    else
      g_clipNamesRefCount[liveProjectFolder][clipName] = 1;

    //  Load audio clips used by current clip.
    HarmonyAudio audio = GetComponent<HarmonyAudio>();
    if (audio != null)
    {
      audio.LoadAudio( liveProjectFolder, clipName );
    }

    return true;
  }

  public bool LoadClipIndex( int index )
  {
    if ( clipNames == null )
      return false;

    if ( index >= clipNames.Length )
      return false;

    string clipName = clipNames[index];
    return LoadClipName(clipName);
  }

  public Bounds CalculateCumulativeBoundingBox()
  {
    //  Reset Props.  Props refer to HarmonyRenderer scriptId and we
    //  must create a new scriptId to compute bounding box.
    ResetProps();

    Bounds cumulBox = new Bounds(Vector3.zero, Vector3.zero);
    bool emptyBox = true;

    uint dummyColor = 0xFFFFFFFF;
    int dummyPrecision = 20;

    foreach( string clipName in clipNames )
    {
      if ( LoadClipName(clipName) )
      {
        float nFrames = GetClipFrameCount(clipName);
        int scriptId = Internal.CreateRenderScript( liveProjectFolder, clipName );
        if (scriptId != -1 )
        {
          for ( float frame=1.0f ; frame<=nFrames ; frame+=1.0f )
          {
            UpdateProps(scriptId);

            Internal.UpdateRenderScript( scriptId, liveProjectFolder, clipName, sheetResolution, frame, dummyColor, dummyPrecision );

            //  Update collider bounding box if available.
            float[] box = new float[4];
            if ( Internal.CalculateBoundingBox( scriptId, box ) )
            {
              if (emptyBox)
              {
                cumulBox.SetMinMax( new Vector3(box[0], box[1], 0.0f), new Vector3(box[2], box[3], 0.0f) );
                emptyBox = false;
              }
              else
              {
                cumulBox.Encapsulate( new Vector3(box[0], box[1], 0.0f) );
                cumulBox.Encapsulate( new Vector3(box[2], box[3], 0.0f) );
              }
            }
          }

          Internal.UnloadRenderScript(scriptId);
        }
      }
    }

    //  Reset props.  scriptId is not used anymore.
    ResetProps();

    return cumulBox;
  }

  public Bounds CalculateCurrentBoundingBox()
  {
    //  Reset Props.  Props refer to HarmonyRenderer scriptId and we
    //  must create a new scriptId to compute bounding box.
    ResetProps();

    Bounds cumulBox = new Bounds(Vector3.zero, Vector3.zero);
    bool emptyBox = true;

    if ( (clipIdx >= 0) && (clipIdx < clipNames.Length) )
    {
      uint dummyColor = 0xFFFFFFFF;
      int dummyPrecision = 20;

      if ( LoadClipIndex(clipIdx) )
      {
        string clipName = clipNames[clipIdx];

        float nFrames = GetClipFrameCount(clipName);
        int scriptId = Internal.CreateRenderScript( liveProjectFolder, clipName );
        if (scriptId != -1 )
        {
          for ( float frame=1.0f ; frame<=nFrames ; frame+=1.0f )
          {
            UpdateProps(scriptId);

            Internal.UpdateRenderScript( scriptId, liveProjectFolder, clipName, sheetResolution, frame, dummyColor, dummyPrecision );

            //  Update collider bounding box if available.
            float[] box = new float[4];
            if ( Internal.CalculateBoundingBox( scriptId, box ) )
            {
              if (emptyBox)
              {
                cumulBox.SetMinMax( new Vector3(box[0], box[1], 0.0f), new Vector3(box[2], box[3], 0.0f) );
                emptyBox = false;
              }
              else
              {
                cumulBox.Encapsulate( new Vector3(box[0], box[1], 0.0f) );
                cumulBox.Encapsulate( new Vector3(box[2], box[3], 0.0f) );
              }
            }
          }

          Internal.UnloadRenderScript(scriptId);
        }
      }
    }

    //  Reset props.  scriptId is not used anymore.
    ResetProps();

    return cumulBox;
  }

#if UNITY_EDITOR
  public void PreCalculateColliders()
  {
    //  Reset Props.  Props refer to HarmonyRenderer scriptId and we
    //  must create a new scriptId to compute bounding box.
    ResetProps();

    //  Clear existing colliders
    ClearColliders();

    uint dummyColor = 0xFFFFFFFF;
    int dummyPrecision = 20;

    foreach( string clipName in clipNames )
    {
      if ( LoadClipName(clipName) )
      {
        float nFrames = GetClipFrameCount(clipName);
        int scriptId = Internal.CreateRenderScript( liveProjectFolder, clipName );
        if (scriptId != -1 )
        {
          for ( float frame=1.0f ; frame<=nFrames ; frame+=1.0f )
          {
            UpdateProps(scriptId);

            Internal.UpdateRenderScript( scriptId, liveProjectFolder, clipName, sheetResolution, frame, dummyColor, dummyPrecision );

            ColliderKey key;
            key.clipName = clipName;
            key.frame = frame;
            key.colliderShape = colliderShape;

            /*Collider2D collider = */CreateCollider(scriptId, key);
          }

          Internal.UnloadRenderScript(scriptId);
        }

      }
    }

    //  Only leave first collider enabled (i.e. first clip, first frame).
    if ( colliderLinks.Count > 0 )
    {
      colliderLinks[0].enabled = true;
      for ( int i=1 ; i<colliderLinks.Count ; ++i )
      {
        colliderLinks[i].enabled = false;
      }
    }

    //  Reset props.  scriptId is not used anymore.
    ResetProps();
  }

  public void ClearColliders()
  {
    //  Clean up existing colliders
    for ( int i=0 ; i<colliderLinks.Count ; ++i )
    {
      Collider2D collider = colliderLinks[i];
      collider.hideFlags = HideFlags.HideInInspector;
      DestroyImmediate(collider);
    }

    colliderKeys.Clear();
    colliderLinks.Clear();
  }
#endif // UNITY_EDITOR

  private Collider2D CreateCollider( int scriptId, ColliderKey key )
  {
    Collider2D newCollider = null;

    //  Using bounding box shape as collider.
    if ( key.colliderShape == ColliderShape.eBox )
    {
      //  Update collider bounding box if available.
      float[] box = new float[4];
      if ( Internal.CalculateBoundingBox( scriptId, box ) )
      {
        BoxCollider2D boxCollider = gameObject.AddComponent<BoxCollider2D>();
        boxCollider.enabled = true;
        boxCollider.isTrigger = colliderTrigger;

#if UNITY_4_3 || UNITY_4_5 || UNITY_4_6
        boxCollider.center = new Vector2( (box[0]+box[2]) * 0.5f, (box[1]+box[3]) * 0.5f );
#else // UNITY_5_0
        boxCollider.offset = new Vector2( (box[0]+box[2]) * 0.5f, (box[1]+box[3]) * 0.5f );
#endif
        boxCollider.size = new Vector2( box[2]-box[0], box[3]-box[1] );

        colliderKeys.Add( key.ToString() );
        colliderLinks.Add( boxCollider );

        newCollider = boxCollider;
      }
    }
    //  Calculate convex hull shape as collider.
    else if ( colliderShape == ColliderShape.eConvexHull )
    {
      IntPtr convexHullArrayRaw  = IntPtr.Zero;
      int    convexHullSize      = 0;

      if ( Internal.CalculateConvexHull( scriptId, ref convexHullArrayRaw, ref convexHullSize ) )
      {
        float[] convexHullArray = new float[ convexHullSize * 2 ];
        Marshal.Copy( convexHullArrayRaw,
            convexHullArray,
            0,
            convexHullSize * 2 );

        Internal.DeallocateConvexHull( convexHullArrayRaw );

        PolygonCollider2D polygonCollider = gameObject.AddComponent<PolygonCollider2D>();
        polygonCollider.enabled = true;
        polygonCollider.isTrigger = colliderTrigger;
        polygonCollider.pathCount = 1;

        Vector2[] path = new Vector2[ convexHullSize ];
        for( int idx=0 ; idx < convexHullSize ; ++idx )
        {
          path[idx] = new Vector2( convexHullArray[idx*2], convexHullArray[idx*2 + 1] );
        }

        polygonCollider.SetPath( 0, path );

        colliderKeys.Add( key.ToString() );
        colliderLinks.Add( polygonCollider );

        newCollider = polygonCollider;
      }
    }
    //  Calculate tight polygons as collider.
    else // if ( colliderShape == ColliderShape.ePolygons )
    {
      IntPtr polygonArrayRaw     = IntPtr.Zero;
      int    polygonSize         = 0;

      IntPtr subPolygonArrayRaw  = IntPtr.Zero;
      int    subPolygonSize      = 0;

      if ( Internal.CalculatePolygons( scriptId, ref polygonArrayRaw, ref polygonSize, ref subPolygonArrayRaw, ref subPolygonSize ) )
      {
        float[] polygonArray = new float[ polygonSize * 2 ];
        Marshal.Copy( polygonArrayRaw,
            polygonArray,
            0,
            polygonSize * 2 );

        int[] subPolygonArray = new int[ subPolygonSize ];
        Marshal.Copy( subPolygonArrayRaw,
            subPolygonArray,
            0,
            subPolygonSize );

        Internal.DeallocatePolygons( polygonArrayRaw, subPolygonArrayRaw );

        PolygonCollider2D polygonCollider = gameObject.AddComponent<PolygonCollider2D>();
        polygonCollider.enabled = true;
        polygonCollider.isTrigger = colliderTrigger;
        polygonCollider.pathCount = subPolygonSize;

        int baseIndex = 0;
        for ( int polygonIndex=0 ; polygonIndex < subPolygonSize ; ++polygonIndex )
        {
          int numberOfPoints = subPolygonArray[polygonIndex];
          int lastIndex = baseIndex + numberOfPoints;

          Vector2[] path = new Vector2[numberOfPoints];
          for ( int idx=0 ; idx<numberOfPoints ; ++idx )
          {
            int ptIndex = baseIndex + idx;
            path[idx] = new Vector2( polygonArray[ptIndex*2], polygonArray[ptIndex*2 + 1] );
          }

          polygonCollider.SetPath(polygonIndex, path);

          baseIndex = lastIndex;
        }

        colliderKeys.Add( key.ToString() );
        colliderLinks.Add( polygonCollider );

        newCollider = polygonCollider;
      }
    }

    return newCollider;
  }

  private bool UpdateProps( int scriptId )
  {
    if (harmonyPropsDirty)
    {
      //  Build cached prop dictionary from child component list.
      HarmonyProp[] propComponents = GetComponentsInChildren<HarmonyProp>(true /*includeInactive*/);
      foreach( HarmonyProp propComponent in propComponents )
      {
        LoadClipName( propComponent.clipName );
        int propId = propComponent.CreateProp( scriptId, liveProjectFolder );

        if (propId >= 0)
          harmonyProps[propId] = propComponent;
      }

      harmonyPropsDirty = false;
    }

    List<int> keysToRemove = null;
    bool isDirty = false;

    foreach(KeyValuePair<int, HarmonyProp> harmonyProp in harmonyProps)
    {
      //  HarmonyProp component does not exists anymore, disable prop rendering
      //  and schedule to remove from dictionary.
      if ( harmonyProp.Value == null )
      {
        HarmonyProp.DisableProp( scriptId, harmonyProp.Key );

        keysToRemove = new List<int>();
        keysToRemove.Add(harmonyProp.Key);

        isDirty = true;
      }
      //  Update HarmonyProp component.
      else
      {
        LoadClipName( harmonyProp.Value.clipName );
        bool propIsDirty = harmonyProp.Value.UpdateProp( scriptId );

        isDirty = isDirty || propIsDirty;
      }
    }

    //  Clean null objects from dictionary.
    if (keysToRemove != null)
    {
      foreach(int key in keysToRemove)
      {
        harmonyProps.Remove(key);
      }
    }

    return isDirty;
  }

  public void ResetProps()
  {
    foreach(KeyValuePair<int, HarmonyProp> harmonyProp in harmonyProps)
    {
      if ( harmonyProp.Value != null )
      {
        harmonyProp.Value.ResetProp();
      }
    }

    harmonyProps.Clear();
    harmonyPropsDirty = true;
  }

  public void InvalidateProps()
  {
    harmonyPropsDirty = true;
  }

  public float GetClipFrameCount( string clipName )
  {
    //  Load clip if not already loaded.
    if ( !loadedClips.ContainsKey( clipName ) )
    {
      LoadClipName(clipName);
    }

    //  Retrieve duration from loadedClipNames.
    if ( loadedClips.ContainsKey( clipName ) )
    {
      return loadedClips[clipName].nFrames;
    }

    return 0.0f;
  }

  public float GetClipFrameCount( int index )
  {
    if ( clipNames == null )
      return 0.0f;

    if ( index >= clipNames.Length )
      return 0.0f;

    string clipName = clipNames[index];
    return GetClipFrameCount(clipName);
  }

  private void OnDisable()
  {
    bool needsUpdate = false;

    //  Free render scripts.  They'll be recreated if the game
    //  object is reactivated.  Clips and sprite sheet still
    //  remain loaded in memory.
    if ( scriptId != -1 )
    {
      ResetProps();

      Internal.UnloadRenderScript(scriptId);
      scriptId = -1;

      needsUpdate = true;
    }

    //  Cleanup will free texture buffers, vertex buffers and frame buffers.
    if ( needsUpdate )
    {
      HarmonyCamera.PluginCleanup();
    }
  }

  private void OnDestroy()
  {
    bool needsUpdate = false;

    //  Free render scripts.
    if ( scriptId != -1 )
    {
      Internal.UnloadRenderScript(scriptId);
      needsUpdate = true;
    }

    if ( g_clipNamesRefCount.ContainsKey(liveProjectFolder) )
    {
      Dictionary< string, int > clipNamesRefCount = g_clipNamesRefCount[ liveProjectFolder ];

      foreach( KeyValuePair<string, ClipData> clipEntry in loadedClips )
      {
        string clipName = clipEntry.Key;

        int clipRefCount = --clipNamesRefCount[clipName];

        //  If refCount is at zero, free resources in plugin.
        if ( clipRefCount == 0 )
        {
          clipNamesRefCount.Remove( clipName );

          //Message.Log( "UnloadStageClip: '" + liveProjectFolder + "':'" + clipName + "'" );
          Internal.UnloadStageClip( liveProjectFolder, clipName );
          needsUpdate = true;
        }
      }
      loadedClips.Clear();

      //  No more clips in specific project, unload project as well.
      if ( clipNamesRefCount.Count == 0 )
      {
        g_clipNamesRefCount.Remove( liveProjectFolder );

        //Message.Log( "UnloadProject: '" + liveProjectFolder + "'" );
        Internal.UnloadProject( liveProjectFolder );
        needsUpdate = true;
      }
    }

    //  Cleanup will free texture buffers, vertex buffers and frame buffers.
    if ( needsUpdate )
    {
      HarmonyCamera.PluginCleanup();
    }
  }

  private void Update()
  {
    if ( clipNames == null )
      return;
#if (UNITY_WEBGL) && !UNITY_EDITOR
    if (!readyToRender)
    {
      return;
    }
#endif

    float frame = 1.0f;
    HarmonyAnimation animation = GetComponent<HarmonyAnimation>();
    if ( animation != null )
    {
      animation.Step( Time.deltaTime );
      frame = animation.frame;
      frame = (float)Math.Floor(frame);

      SetClipName( animation.clipName );
    }

    bool animationIsDirty = (lastFrame != frame);

    if ( (clipIdx < 0) || (clipIdx >= clipNames.Length) )
      return;

    string clipName = clipNames[clipIdx];

    uint intColor = renderingWithDirectX ?
      ((uint)(color.a * 255) << 24) + ((uint)(color.r * color.a * 255) << 16) + ((uint)(color.g * color.a * 255) << 8) + ((uint)(color.b * color.a * 255) << 0) :
      ((uint)(color.a * 255) << 24) + ((uint)(color.b * color.a * 255) << 16) + ((uint)(color.g * color.a * 255) << 8) + ((uint)(color.r * color.a * 255) << 0) ;

    if ( scriptId == -1 )
      scriptId = Internal.CreateRenderScript(liveProjectFolder, clipName);

    if ( scriptId != -1 )
    {
      bool scriptIsDirty = animationIsDirty || (lastColor != intColor);
      scriptIsDirty = scriptIsDirty || UpdateProps(scriptId);

#if UNITY_EDITOR
      scriptIsDirty = scriptIsDirty || (lastDiscretizationStep != discretizationStep);
      lastDiscretizationStep = discretizationStep;
#endif

      if ( scriptIsDirty )
      {
        Internal.UpdateRenderScript( scriptId, liveProjectFolder, clipName, sheetResolution, frame, intColor, discretizationStep);
      }

      if ( renderTarget == RenderTarget.eRenderTexture )
      {
        //  renderable object attached to current game object.
        //  Attach texture to it on which we will render.
        Renderer renderer = GetComponent<Renderer>();
        if ( (renderer != null) && (renderer.sharedMaterial) )
        {
          bool textureIsDirty = scriptIsDirty || (liveTextureViewport != renderTextureViewport);

          //  No need to update texture at every frame if it did not change.
          if ( textureIsDirty )
          {
            // Pass texture pointer to the plugin
            RenderTexture rendererTexture = renderer.sharedMaterial.mainTexture as RenderTexture;

            if ( (rendererTexture == null) ||
                 (rendererTexture.width != renderTextureViewport.width) ||
                 (rendererTexture.height != renderTextureViewport.height) )
            {
              //  Using RenderTexture instead of Texture2D as it resides in default memory pool in directx and allows
              //  direct rendering to texture.
              rendererTexture = new RenderTexture((int)renderTextureViewport.width, (int)renderTextureViewport.height, 16, RenderTextureFormat.ARGB32);
              rendererTexture.filterMode = FilterMode.Bilinear;
              rendererTexture.Create();

#if UNITY_EDITOR
              //  Clear buffer to avoid getting garbage if update is delayed in scene view.
              Graphics.SetRenderTarget(rendererTexture);
              GL.Clear(true, true, Color.clear);
              Graphics.SetRenderTarget(null);
#endif

              // Set texture onto our material
              if (!Application.isPlaying)
              {
                Material materialCopy = Material.Instantiate(renderer.sharedMaterial) as Material;
                materialCopy.name = renderer.sharedMaterial.name;

                materialCopy.mainTexture = rendererTexture;

                UnityEngine.Object.DestroyImmediate(renderer.sharedMaterial.mainTexture);
                UnityEngine.Object.DestroyImmediate(renderer.sharedMaterial);

                renderer.material = materialCopy;
              }
              else
              {
                UnityEngine.Object.Destroy(renderer.sharedMaterial.mainTexture);
                UnityEngine.Object.Destroy(renderer.sharedMaterial);

                renderer.material.mainTexture = rendererTexture;
              }

              rendererTexturePtr = IntPtr.Zero;
            }

            //  Retrieve Texture Id/Ptr if not set already.  This is a costly operation so only do it once.
            if ( rendererTexturePtr == IntPtr.Zero )
            {
              rendererTexturePtr = rendererTexture.GetNativeTexturePtr();
            }

            // Pass texture pointer to the plugin
            Internal.CreateRenderTextureEvent( scriptId, renderTextureViewport.x, renderTextureViewport.y, renderTextureViewport.width, renderTextureViewport.height, renderTextureScale, rendererTexturePtr );

            liveTextureViewport = renderTextureViewport;
          }
        }
        else
        {
          //  Invalidate texture viewport...
          liveTextureViewport = new Rect(0, 0, 0, 0);
        }
      }
      else
      {
        //  Proceed with direct render in LateUpdate so that 
        //  all matrices are computed when render request is processed.
        scriptIdToRender = scriptId;
      }
    }

    //  Update attached components and child game objects.
    //  Only performed when application is playing to avoid modifying game object
    //  parameters and triggering scene changes.
    if ( Application.isPlaying && animationIsDirty )
    {
      bool doSync =
        ( syncCollider == SyncCollider.eAlways ) ||
        ( (syncCollider == SyncCollider.eOnce) && !isColliderSynced );

      if ( doSync )
      {
        ColliderKey key;
        key.clipName = clipName;
        key.frame = frame;
        key.colliderShape = colliderShape;

        if ( currentCollider != null )
        {
          currentCollider.enabled = false;
        }

        int colliderIndex = colliderKeys.IndexOf(key.ToString());
        if (colliderIndex != -1)
        {
          Collider2D collider = colliderLinks[colliderIndex];
          collider.enabled = true;
          collider.isTrigger = colliderTrigger;

          currentCollider = collider;
        }
        else
        {
          Collider2D collider = CreateCollider(scriptId, key);
          collider.enabled = true;
          collider.isTrigger = colliderTrigger;

          currentCollider = collider;
        }

        isColliderSynced = true;
      }
    }

    //  Calculate bounding box for gizmos rendering.
    //  Only used in editor.
#if UNITY_EDITOR
    if ( (scriptId != -1) && (renderTarget == RenderTarget.eScreen) )
    {
      Internal.CalculateBoundingBox( scriptId, interactiveBox );
    }
#endif

    lastFrame = frame;
    lastColor = intColor;
  }

  private void LateUpdate()
  {
    //  Render script after all matrix calculations so that it can inherit latest
    //  transform updates.
    if ( scriptIdToRender >= 0 )
    {
      int unityFrame = Time.frameCount;
      Matrix4x4 modelMatrix = transform.localToWorldMatrix;

      Internal.CreateRenderEvent( unityFrame, depth, scriptIdToRender, 1 << gameObject.layer, MatrixToArray(modelMatrix) );
      scriptIdToRender = -1; // Reset script id.
    }
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

#if UNITY_EDITOR
	private void OnDrawGizmos()
  {
    Gizmos.color = Color.yellow;

    if ( renderTarget == RenderTarget.eScreen )
    {
      //Message.Log( "Interactive Box: [" + interactiveBox[0] + "," + interactiveBox[1] + "] [" + interactiveBox[2] + "," + interactiveBox[3] + "]" );
      Vector3 center = new Vector3( (interactiveBox[0]+interactiveBox[2]) * 0.5f, (interactiveBox[1]+interactiveBox[3]) * 0.5f, 0.0f );
      Vector3 size = new Vector3( interactiveBox[2]-interactiveBox[0], interactiveBox[3]-interactiveBox[1], 0.0f );

      Gizmos.matrix = transform.localToWorldMatrix;
      Gizmos.DrawWireCube(center, size);

      /*
      Gizmos.color = Color.red;
      PolygonCollider2D[] colliders = GetComponents<PolygonCollider2D>();
      foreach( PolygonCollider2D collider in colliders )
      {
        if ( collider.enabled )
        {
          for( int i=0 ; i < collider.pathCount ; ++i )
          {
            Vector2[] path = collider.GetPath(i);
            for ( int j=0 ; j < path.Length ; ++j )
            {
              Gizmos.DrawLine( path[j], path[ (j+1)%path.Length ] );
            }
          }
        }
      }
      */
    }
    else if ( renderTarget == RenderTarget.eRenderTexture )
    {
      MeshRenderer meshRenderer = GetComponent<MeshRenderer>();
      if ( meshRenderer != null )
      {
        Gizmos.DrawWireCube(meshRenderer.bounds.center, meshRenderer.bounds.size);
      }
    }
	}
#endif
}
