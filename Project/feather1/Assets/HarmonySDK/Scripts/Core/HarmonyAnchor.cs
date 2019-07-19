
using UnityEngine;
using System.Runtime.InteropServices;

/*!
 *  @class HarmonyAnchor
 *  Extract position from animation node.
 *  To be used in conjunction with HarmonyRenderer.
 */
[ExecuteInEditMode]
[AddComponentMenu("Harmony/Core/HarmonyAnchor")]
public class HarmonyAnchor : MonoBehaviour
{
  public string            playName;
  public string            nodeName;

  public bool              extractTransform = false;

  private HarmonyRenderer  harmonyRenderer = null;

  private float            activeFrame = 0.0f;

  private class Internal
  {
#if (UNITY_IPHONE || UNITY_XBOX360 || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal")]
    public static extern bool CalculateLocatorTransform( string projectFolder, string clipName, float frame, string locatorName, [In, Out] float[] position, [In, Out] float[] rotation, [In, Out] float[] scale );
#else
    [DllImport ("HarmonyRenderer")]
    public static extern bool CalculateLocatorTransform( string projectFolder, string clipName, float frame, string locatorName, [In, Out] float[] position, [In, Out] float[] rotation, [In, Out] float[] scale );
    #endif
  }

  private void Awake()
  {
    harmonyRenderer = GetComponentInParent<HarmonyRenderer>();
  }

  private void LateUpdate()
  {
    if (extractTransform == false)
      return;

    if (!Application.isPlaying)
      return;

    if (harmonyRenderer == null)
      return;


    string clipName = harmonyRenderer.currentClipName;
    if (string.IsNullOrEmpty(clipName))
      return;

    string projectFolder = harmonyRenderer.currentProjectFolder;
    if (string.IsNullOrEmpty(projectFolder))
      return;

    float frame = harmonyRenderer.currentFrame;
    if (frame != activeFrame)
    {
      //  Update HarmonyLocator position during LateUpdate to make sure that parent HarmonyRenderer component
      //  has finished updating the rendering script.  The locator must be a child of a Game Object with a
      //  HarmonyRenderer component.  It will inherit the game object transform and append specified bone transform.
      float[] position = new float[3];
      float[] rotation = new float[3];
      float[] scale = new float[3];

      if ( Internal.CalculateLocatorTransform( projectFolder, clipName, frame, nodeName, position, rotation, scale ) )
      {
        transform.localPosition = new Vector3( position[0], position[1], position[2] );
        transform.localRotation = Quaternion.Euler( rotation[0], rotation[1], rotation[2] );
        transform.localScale = new Vector3( scale[0], scale[1], scale[2] );
      }

      activeFrame = frame;
    }
  }
}
