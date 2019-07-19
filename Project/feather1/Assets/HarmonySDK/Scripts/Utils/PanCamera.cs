
#if (UNITY_IPHONE || UNITY_ANDROID) && !UNITY_EDITOR
#define UNITY_TOUCH_ENABLED
#endif

using UnityEngine;
using System.Collections;

/*!
 *  @class PanCamera
 *  2D Manipulator for camera.
 */
[AddComponentMenu("Harmony/Utils/PanCamera")]
public class PanCamera : MonoBehaviour
{
  public Camera     attachedCamera;

  private float     zOffset;
  private Vector3   prevDevPos;

#if UNITY_TOUCH_ENABLED
  private bool      isPinching = false;
  private float     lastDelta;
#else
  private bool      isDragging = false;
  private float     zoomSpeed = 4.0f;
#endif

  // Use this for initialization
  void Start ()
  {
  }

  // Update is called once per frame
  void Update ()
  {
    if ( attachedCamera != null )
    {
      #if UNITY_TOUCH_ENABLED
      //  Pan.
      if (Input.touchCount == 1)
      {
        if ( Input.GetTouch(0).phase == TouchPhase.Began )
        {
          zOffset = attachedCamera.WorldToScreenPoint(gameObject.transform.position).z;
          prevDevPos = new Vector3(Input.GetTouch(0).position.x, Input.GetTouch(0).position.y, zOffset);
        }
        else if ( Input.GetTouch(0).phase == TouchPhase.Moved )
        {
          Vector3 devPos = new Vector3(Input.GetTouch(0).position.x, Input.GetTouch(0).position.y, zOffset);

          Vector3 prevWorldPos = attachedCamera.ScreenToWorldPoint(prevDevPos);
          Vector3 worldPos = attachedCamera.ScreenToWorldPoint(devPos);

          attachedCamera.transform.Translate( prevWorldPos - worldPos, Space.World );

          prevDevPos = devPos;
        }
      }
      //  Zoom in/out.
      else if (Input.touchCount == 2)
      {
        if ( (Input.GetTouch(0).phase == TouchPhase.Moved) && (Input.GetTouch(1).phase == TouchPhase.Moved) )
        {
          float delta = (Input.GetTouch(0).position - Input.GetTouch(1).position).magnitude;

          if ( isPinching )
          {
            float scale = delta / lastDelta;

            //  Scale is not directly proportional to field of view, but should be close enough.
            attachedCamera.fieldOfView = Mathf.Clamp(attachedCamera.fieldOfView * 1.0f/scale, 4, 120);
          }
          else
          {
            isPinching = true;
          }

          lastDelta = delta;
        }
        else
        {
          isPinching = false;
        }
      }
      #else
      //  Pan.
      if ( Input.GetMouseButton(0) )
      {
        if ( isDragging )
        {
          Vector3 devPos = new Vector3(Input.mousePosition.x, Input.mousePosition.y, zOffset);

          Vector3 prevWorldPos = attachedCamera.ScreenToWorldPoint(prevDevPos);
          Vector3 worldPos = attachedCamera.ScreenToWorldPoint(devPos);

          //  Translate camera in with inverse coordinates.
          attachedCamera.transform.Translate( prevWorldPos - worldPos, Space.World );

          prevDevPos = devPos;
        }
        else
        {
          isDragging = true;

          zOffset = attachedCamera.WorldToScreenPoint(gameObject.transform.position).z;
          prevDevPos = new Vector3(Input.mousePosition.x, Input.mousePosition.y, zOffset);
        }
      }
      else
      {
        isDragging = false;
      }

      //  Zoom in/out.
      if ( (Input.GetAxis("Mouse ScrollWheel") < 0) || Input.GetKeyDown( KeyCode.Plus ) || Input.GetKeyDown( KeyCode.KeypadPlus ) )
      {
        attachedCamera.fieldOfView = Mathf.Clamp(attachedCamera.fieldOfView + zoomSpeed, 4, 120);
      }
      else if ( (Input.GetAxis("Mouse ScrollWheel") > 0) || Input.GetKeyDown( KeyCode.Minus ) || Input.GetKeyDown( KeyCode.KeypadMinus ) )
      {
        attachedCamera.fieldOfView = Mathf.Clamp(attachedCamera.fieldOfView - zoomSpeed, 4, 120);
      }
      #endif
    }
  }

}
