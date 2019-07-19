using UnityEngine;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;

/*!
 *  @class HarmonyProp
 */
[ExecuteInEditMode]
[AddComponentMenu("Harmony/Core/HarmonyProp")]
public class HarmonyProp : MonoBehaviour
{
  private class Internal
  {
#if (UNITY_IPHONE || UNITY_XBOX360 || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal")]
    public static extern int CreateProp( int scriptId, string projectFolder, string clipName, string playName );

    [DllImport ("__Internal")]
    public static extern int UpdateProp( int scriptId, int propId, float frame );

    [DllImport ("__Internal")]
    public static extern void AnchorProp( int scriptId, int propId, string playName, string nodeName );

    [DllImport ("__Internal")]
    public static extern void UnanchorProp( int scriptId, int propId );
#else
    [DllImport ("HarmonyRenderer")]
    public static extern int CreateProp( int scriptId, string projectFolder, string clipName, string playName );

    [DllImport ("HarmonyRenderer")]
    public static extern int UpdateProp( int scriptId, int propId, float frame );

    [DllImport ("HarmonyRenderer")]
    public static extern void AnchorProp( int scriptId, int propId, string playName, string nodeName );

    [DllImport ("HarmonyRenderer")]
    public static extern void UnanchorProp( int scriptId, int propId );
    #endif
  }

  public string           clipName = "";
  public string           playName = "";
  public float            frame = 1.0f;

  public HarmonyMeta      metadata = new HarmonyMeta();

  private HarmonyAnchor   cachedAnchor;
  private float           cachedFrame = 1.0f;

  private Transform       cachedParentTransform = null;
  private HarmonyRenderer cachedHarmonyRenderer = null;

  private int             propId = -1;

  private void Awake()
  {
    cachedParentTransform = gameObject.transform.parent;
    cachedHarmonyRenderer = GetComponentInParent<HarmonyRenderer>();
  }

  private void OnTransformParentChanged()
  {
    //  Update HarmonyRenderer component if prop has been reparented
    //  in a new hierarchy.
    Transform parentTransform = gameObject.transform.parent;
    if (parentTransform != cachedParentTransform)
    {
      cachedParentTransform = parentTransform;

      HarmonyRenderer harmonyRenderer = GetComponentInParent<HarmonyRenderer>();
      if (harmonyRenderer != cachedHarmonyRenderer)
      {
        if (cachedHarmonyRenderer != null)
          cachedHarmonyRenderer.InvalidateProps();

        cachedHarmonyRenderer = harmonyRenderer;

        if (cachedHarmonyRenderer != null)
          cachedHarmonyRenderer.InvalidateProps();
      }
    }
  }

  public int CreateProp( int scriptId, string projectFolder )
  {
    if ( scriptId == -1 )
      return -1;

    if ( propId == -1 )
      propId = Internal.CreateProp( scriptId, projectFolder, clipName, playName );

    return propId;
  }

  public bool UpdateProp( int scriptId )
  {
    bool ret = false;

    if ( propId != -1 )
    {
      if ( gameObject.activeInHierarchy )
      {
        if ( frame != cachedFrame )
        {
          Internal.UpdateProp( scriptId, propId, frame );
          cachedFrame = frame;
          ret = true;
        }

        //  Prop is only visible if parented to a visible anchor.
        HarmonyAnchor anchor = GetComponentInParent<HarmonyAnchor>();
        if (anchor != cachedAnchor)
        {
          if (anchor != null)
          {
            //Message.Log( "Anchoring prop " + playName + " in script " + scriptId );
            Internal.AnchorProp( scriptId, propId, anchor.playName, anchor.nodeName );
          }
          else
          {
            //Message.Log( "Unanchoring prop " + playName + " in script " + scriptId );
            Internal.UnanchorProp( scriptId, propId );
          }

          cachedAnchor = anchor;
          ret = true;
        }
      }
      else
      {
        //  Current game object is deactivated.  Disable prop rendering.
        if (cachedAnchor != null)
        {
          //Message.Log( "Unanchoring prop " + playName + " in script " + scriptId );
          Internal.UnanchorProp( scriptId, propId );
          cachedAnchor = null;
          ret = true;
        }
      }
    }

    return ret;
  }

  static public void DisableProp( int scriptId, int propId )
  {
    //Message.Log( "Unanchoring prop " + playName + " in script " + scriptId );
    Internal.UnanchorProp( scriptId, propId );
  }

  public void ResetProp()
  {
    cachedAnchor = null;
    cachedFrame = 1.0f;

    propId = -1;
  }
}
