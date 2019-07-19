using UnityEngine;
using System.Collections;

/*!
 *  @class PixelLine
 *  GL Line with pixel size.
 */
[AddComponentMenu("Harmony/Utils/PixelLine")]
public class PixelLine : MonoBehaviour
{
  public Camera mainCamera;
  public float  lineWidth = 10.0f;

  void Update()
  {
    if (mainCamera == null)
      return;

    LineRenderer lineRenderer = GetComponent<LineRenderer>();
    if (lineRenderer == null)
      return;

    //  Reproject pixel distance to world.
    float zOffset = mainCamera.WorldToScreenPoint(new Vector3(0.0f, 0.0f, 0.0f)).z;
    Vector3 devPoint = new Vector3( mainCamera.pixelWidth * 0.5f, mainCamera.pixelHeight * 0.5f, zOffset);
    Vector3 devPoint1 = new Vector3( devPoint.x+lineWidth, devPoint.y, devPoint.z );

    Vector3 worldPoint = mainCamera.ScreenToWorldPoint( devPoint );
    Vector3 worldPoint1 = mainCamera.ScreenToWorldPoint( devPoint1 );

    float worldWidth = Vector3.Distance( worldPoint, worldPoint1 );

    //  Update LineRenderer with new width.
    lineRenderer.SetWidth( worldWidth, worldWidth );
  }
}
