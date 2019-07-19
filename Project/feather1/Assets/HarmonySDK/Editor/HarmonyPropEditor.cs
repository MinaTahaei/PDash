using UnityEngine;
using System.Collections;
using UnityEditor;

[CustomEditor(typeof(HarmonyProp))]
[CanEditMultipleObjects]
public class HarmonyPropEditor : Editor
{
  static int TAB_SIZE = 20;

  SerializedProperty targetClipName;
  SerializedProperty targetPlayName;
  SerializedProperty targetFrame;

  SerializedProperty targetMetadataContainer;

  static bool foldMetadata = false;

  void OnEnable()
  {
    //  Prop reference in project.
    targetClipName = serializedObject.FindProperty("clipName");
    targetPlayName = serializedObject.FindProperty("playName");

    //  Prop animation data.
    targetFrame = serializedObject.FindProperty("frame");

    //  Metadata
    targetMetadataContainer = serializedObject.FindProperty("metadata");
  }

  public override void OnInspectorGUI()
  {
    Object[] targetObjects = serializedObject.targetObjects;

    EditorGUILayout.PropertyField(targetClipName, new GUIContent("Clip Name"));
    EditorGUILayout.PropertyField(targetPlayName, new GUIContent("Play Name"));

    EditorGUILayout.PropertyField(targetFrame, new GUIContent("Frame"));

    SerializedProperty targetMetadataList = targetMetadataContainer.FindPropertyRelative("metas");
    //if (targetMetadataList.arraySize > 0)
    {
      foldMetadata = EditorGUILayout.Foldout(foldMetadata, "Metadata");
      if(foldMetadata)
      {
        EditorGUIUtility.labelWidth = 1f;

        ++EditorGUI.indentLevel;

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(EditorGUI.indentLevel * TAB_SIZE);
        EditorGUILayout.BeginVertical();
        EditorGUILayout.BeginHorizontal("box");
        GUILayout.Label( "name " );
        GUILayout.Label( "node" );
        GUILayout.Label( "value" );

        if(GUILayout.Button("+", GUILayout.Width(16), GUILayout.Height(16) ))
        {
          targetMetadataList.arraySize++;
        }

        EditorGUILayout.EndHorizontal();
        EditorGUILayout.EndVertical();
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(EditorGUI.indentLevel * TAB_SIZE);
        EditorGUILayout.BeginVertical();
        for(int j=0 ; j<targetMetadataList.arraySize ; ++j)
        {
          SerializedProperty targetMetadata = targetMetadataList.GetArrayElementAtIndex(j);

          EditorGUILayout.BeginHorizontal("box");

          SerializedProperty targetMetadataName = targetMetadata.FindPropertyRelative("metaName");
          SerializedProperty targetMetadataNodeName = targetMetadata.FindPropertyRelative("nodeName");
          SerializedProperty targetMetadataValue = targetMetadata.FindPropertyRelative("metaValue");

          EditorGUILayout.PropertyField(targetMetadataName, new GUIContent(" ") );
          EditorGUILayout.PropertyField(targetMetadataNodeName, new GUIContent(" ") );
          EditorGUILayout.PropertyField(targetMetadataValue, new GUIContent(" ") );

          if(GUILayout.Button("-", GUILayout.Width(16), GUILayout.Height(16) ))
          {
            targetMetadataList.DeleteArrayElementAtIndex(j);
          }

          EditorGUILayout.EndHorizontal();
        }
        EditorGUILayout.EndVertical();
        EditorGUILayout.EndHorizontal();

        --EditorGUI.indentLevel;

        EditorGUIUtility.labelWidth = 0f;
      }
    }

    serializedObject.ApplyModifiedProperties();

    if (GUI.changed) {
      foreach (Object target in targetObjects)
      {
        EditorUtility.SetDirty(target);
      }
    }
  }
}
