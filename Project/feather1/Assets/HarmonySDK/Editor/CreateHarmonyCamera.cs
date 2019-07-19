using UnityEditor;
using UnityEngine;

using System.Collections;
using System.Collections.Generic;

public class CreateHarmonyCamera : EditorWindow
{
  private Dictionary<Camera, bool> cameraToggles = new Dictionary<Camera, bool>();

  static public void ShowWindow()
  {
    float width = 200.0f;
    float height = 200.0f;
    Rect rect = new Rect( (Screen.width + width) * 0.5f, (Screen.height + height) * 0.5f, width, height );
    EditorWindow.GetWindowWithRect(typeof(CreateHarmonyCamera), rect, true, "Create Harmony Camera");
  }

  void Awake()
  {
    //  Retrieve list of cameras in current scene.
    Camera[] cameras = FindObjectsOfType<Camera>();
    foreach( Camera camera in cameras )
    {
      cameraToggles[ camera ] = true;
    }
  }

  void OnGUI()
  {
    EditorGUILayout.LabelField("Available Cameras:");

    //  List of camera toggles.
    EditorGUILayout.BeginVertical( "Box" );

    Dictionary<Camera, bool> toToggle = new Dictionary<Camera, bool>();
    foreach( KeyValuePair<Camera, bool> cameraToggle in cameraToggles )
    {
      GameObject gameObject = cameraToggle.Key.gameObject;
      //HarmonyCamera harmonyCamera = gameObject.GetComponent<HarmonyCamera>();

      bool ret = EditorGUILayout.ToggleLeft( gameObject.name, cameraToggle.Value );

      if (ret != cameraToggle.Value)
      {
        toToggle[ cameraToggle.Key ] = ret;
      }
    }

    EditorGUILayout.EndVertical();

    //  Update camera toggles outside of main loop.
    foreach( KeyValuePair<Camera, bool> toggle in toToggle )
    {
      cameraToggles[ toggle.Key ] = toggle.Value;
    }

    //  Weak way of adding spacing, but couldn't find a better way of doing it.
    int nSpaces = 8 - cameraToggles.Count;
    for( int i = 0 ; i<nSpaces ; ++i )
    {
      EditorGUILayout.LabelField("");
    }

    //  Ok/Cancel buttons.
    EditorGUILayout.BeginHorizontal();

    EditorGUILayout.Space();

    if (GUILayout.Button("Create"))
    {
      foreach( KeyValuePair<Camera, bool> cameraToggle in cameraToggles )
      {
        if ( cameraToggle.Value == false )
          continue;

        GameObject gameObject = cameraToggle.Key.gameObject;

        if ( gameObject.GetComponent<HarmonyCamera>() == null )
        {
          gameObject.AddComponent<HarmonyCamera>();
        }
      }

      Close();
    }

    if (GUILayout.Button("Cancel"))
    {
      Close();
    }

    EditorGUILayout.EndHorizontal();
  }

}
