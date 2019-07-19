using UnityEngine;
using UnityEditor;

using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

[CustomEditor(typeof(HarmonyAnchor))]
[CanEditMultipleObjects]
public class HarmonyAnchorEditor : Editor
{
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

#if UNITY_4_3 || UNITY_4_5 || UNITY_4_6 || UNITY_5_0
  //  GizmoType changed halfway through 5.0.  Type will be upgraded in Unity if necessary.
  [DrawGizmo(GizmoType.SelectedOrChild)]
#else // UNITY_5_1 or higher
  [DrawGizmo(GizmoType.InSelectionHierarchy)]
#endif
  static void RenderSelectedAnchorControls (Transform objectTransform, GizmoType gizmoType)
  {
    RenderAnchorControls(objectTransform, gizmoType, true);
  }

#if UNITY_4_3 || UNITY_4_5 || UNITY_4_6 || UNITY_5_0
  //  GizmoType changed halfway through 5.0.  Type will be upgraded in Unity if necessary.
  [DrawGizmo(GizmoType.NotSelected)]
#else // UNITY_5_1 or higher
  [DrawGizmo(GizmoType.NotInSelectionHierarchy)]
#endif
  static void RenderUnselectedAnchorControls (Transform objectTransform, GizmoType gizmoType)
  {
    RenderAnchorControls(objectTransform, gizmoType, false);
  }

  static void RenderAnchorControls (Transform objectTransform, GizmoType gizmoType, bool selected)
  {
    GameObject gameObject = objectTransform.gameObject;
    HarmonyAnchor harmonyAnchor = gameObject.GetComponent<HarmonyAnchor>();
    if (harmonyAnchor == null)
      return;

    HarmonyRenderer harmonyRenderer = harmonyAnchor.GetComponentInParent<HarmonyRenderer>();
    if (harmonyRenderer == null)
      return;

    string clipName = harmonyRenderer.currentClipName;
    if (string.IsNullOrEmpty(clipName))
      return;

    string projectFolder = harmonyRenderer.currentProjectFolder;
    if (string.IsNullOrEmpty(projectFolder))
      return;

    float[] position = new float[3];
    float[] rotation = new float[3];
    float[] scale = new float[3];

    if ( Internal.CalculateLocatorTransform( projectFolder, clipName, harmonyRenderer.currentFrame, harmonyAnchor.nodeName, position, rotation, scale ) )
    {
      Vector3 localPosition = new Vector3(position[0], position[1], position[2]);
      Quaternion localQuaternion = Quaternion.Euler(rotation[0], rotation[1], rotation[2]);
      Vector3 localScale = new Vector3(scale[0], scale[1], scale[2]);

      Matrix4x4 localMatrix = Matrix4x4.TRS(localPosition, localQuaternion, localScale);

      Vector3 globalPosition = localMatrix.MultiplyPoint(Vector3.zero);
      Vector3 globalRightDirection = localMatrix.MultiplyVector(Vector3.right);
      Vector3 globalUpDirection = localMatrix.MultiplyVector(Vector3.up);

      if (gameObject.transform.parent != null)
      {
        globalPosition = gameObject.transform.parent.localToWorldMatrix.MultiplyPoint(globalPosition);
        globalRightDirection = gameObject.transform.parent.localToWorldMatrix.MultiplyVector(globalRightDirection);
        globalUpDirection = gameObject.transform.parent.localToWorldMatrix.MultiplyVector(globalUpDirection);
      }

      globalRightDirection.Normalize();
      globalUpDirection.Normalize();

      Quaternion axisQuaternion = Quaternion.FromToRotation(Vector3.forward, Vector3.right);
      Quaternion globalRightQuaternion = Quaternion.FromToRotation(new Vector3(1,0,0), globalRightDirection);

      float arrowLength = HandleUtility.GetHandleSize(globalPosition) * 0.5f;
      float axisLength = arrowLength * 0.6f;
      float circleRadius = arrowLength * 0.5f;

      Handles.color = selected ? Color.cyan : Color.red;

      Handles.DrawWireDisc(globalPosition,
                           Vector3.forward,
                           circleRadius );

      Handles.DrawLine(globalPosition - globalRightDirection * axisLength,
                       globalPosition );

      Handles.DrawLine(globalPosition - globalUpDirection * axisLength,
                       globalPosition + globalUpDirection * axisLength );

      Handles.ArrowCap(0,
                       globalPosition,
                       globalRightQuaternion * axisQuaternion,
                       arrowLength );
    }
  }
}
