using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;

[CustomEditor(typeof(HarmonyRenderer))]
[CanEditMultipleObjects]
public class HarmonyRendererEditor : Editor
{
  static int TAB_SIZE = 20;

  SerializedProperty targetProjectFolder;
  SerializedProperty targetClipNames;

  SerializedProperty targetSheetResolution;
  SerializedProperty targetColor;
  SerializedProperty targetDepth;
  SerializedProperty targetDiscretizationStep;

  SerializedProperty targetRenderTarget;
  SerializedProperty targetRenderTextureScale;
  SerializedProperty targetSyncCollider;
  SerializedProperty targetColliderShape;
  SerializedProperty targetColliderTrigger;

  SerializedProperty targetMetadataKeys;
  SerializedProperty targetMetadataLists;

  static bool foldRenderTarget = false;
  static bool foldColliders = false;
  static bool foldMetadata = false;

  void OnEnable()
  {
    //  Project properties
    targetProjectFolder = serializedObject.FindProperty("projectFolder");
    targetClipNames = serializedObject.FindProperty("clipNames");

    //  Dynamic properties
    targetSheetResolution = serializedObject.FindProperty("sheetResolution");
    targetColor = serializedObject.FindProperty("color");
    targetDepth = serializedObject.FindProperty("depth");
    targetDiscretizationStep = serializedObject.FindProperty("discretizationStep");

    //  Render target
    targetRenderTarget = serializedObject.FindProperty("renderTarget");
    targetRenderTextureScale = serializedObject.FindProperty("renderTextureScale");

    //  Colliders
    targetSyncCollider = serializedObject.FindProperty("syncCollider");
    targetColliderShape = serializedObject.FindProperty("colliderShape");
    targetColliderTrigger = serializedObject.FindProperty("colliderTrigger");

    //  Metadata
    targetMetadataKeys = serializedObject.FindProperty("metadataKeys");
    targetMetadataLists = serializedObject.FindProperty("metadataLists");
  }

  public override void OnInspectorGUI()
  {
    Object[] targetObjects = serializedObject.targetObjects;

    EditorGUILayout.PropertyField(targetProjectFolder, new GUIContent("Project Folder"));
    EditorGUILayout.PropertyField(targetClipNames, new GUIContent("Clip Names"), true);

    EditorGUILayout.PropertyField(targetSheetResolution, new GUIContent("Sheet Resolution"));
    EditorGUILayout.PropertyField(targetColor, new GUIContent("Color"));
    EditorGUILayout.PropertyField(targetDepth, new GUIContent("Depth"));

    EditorGUILayout.BeginHorizontal();
    EditorGUILayout.PrefixLabel( "Discretization Step" );
    targetDiscretizationStep.intValue = (int)EditorGUILayout.Slider(targetDiscretizationStep.intValue, 1, 30);
    EditorGUILayout.EndHorizontal();

    foldRenderTarget = EditorGUILayout.Foldout(foldRenderTarget, "Render Target");
    if(foldRenderTarget)
    {
      ++EditorGUI.indentLevel;

      int valueIndex1 = targetRenderTarget.enumValueIndex;
      bool isSameValue1 = !targetRenderTarget.hasMultipleDifferentValues;

      EditorGUILayout.PropertyField(targetRenderTarget, new GUIContent("Target"));

      int valueIndex2 = targetRenderTarget.enumValueIndex;
      bool isSameValue2 = !targetRenderTarget.hasMultipleDifferentValues;

      if ( (valueIndex1 != valueIndex2) ||
           !isSameValue1 && isSameValue2 )
      {
        if (valueIndex2 == (int)HarmonyRenderer.RenderTarget.eScreen)
        {
          foreach(HarmonyRenderer harmonyRenderer in targetObjects)
          {
            GameObject rendererObject = harmonyRenderer.gameObject;
            GenerateHarmonyMesh.ClearTextureObjectMesh(rendererObject);
          }

          //  Components were removed from ui.  Bail out now.
          serializedObject.ApplyModifiedProperties();
          EditorGUIUtility.ExitGUI();
        }
        else if (valueIndex2 == (int)HarmonyRenderer.RenderTarget.eRenderTexture)
        {
          foreach(HarmonyRenderer harmonyRenderer in targetObjects)
          {
            GameObject rendererObject = harmonyRenderer.gameObject;
            GenerateHarmonyMesh.CreateOrUpdateTextureObjectMesh(rendererObject, true);
          }

          //  Components were removed from ui.  Bail out now.
          serializedObject.ApplyModifiedProperties();
          EditorGUIUtility.ExitGUI();
        }
      }

      GUI.enabled = ( targetRenderTarget.enumValueIndex == (int)HarmonyRenderer.RenderTarget.eRenderTexture );
      {
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.PrefixLabel( "Texture Resolution" );

        bool disabled = true;
        if (targetObjects.Length == 1)
        {
          HarmonyRenderer harmonyRenderer = targetObjects[0] as HarmonyRenderer;
          GameObject rendererObject = harmonyRenderer.gameObject;

          MeshFilter meshFilter = harmonyRenderer.gameObject.GetComponent<MeshFilter>();
          if ((meshFilter != null) && (meshFilter.sharedMesh != null))
          {
            Bounds bounds = meshFilter.sharedMesh.bounds;
            float meshResolution = Mathf.Max( bounds.size.x, bounds.size.y );
            if ( meshResolution > 0.0f )
            {
              float currentTextureScale = targetRenderTextureScale.floatValue;
              float currentResolution = meshResolution * currentTextureScale;
              float newResolution = EditorGUILayout.Slider(currentResolution, 32, 8192);

              if (newResolution != currentResolution)
              {
                float newTextureScale = newResolution / meshResolution;
                harmonyRenderer.renderTextureScale = newTextureScale;

                GenerateHarmonyMesh.CreateOrUpdateTextureObjectMesh(rendererObject);

                serializedObject.Update();
              }

              disabled = false;
            }
          }
        }

        if (disabled)
        {
          bool wasEnabled = GUI.enabled;
          GUI.enabled = false;
          EditorGUILayout.Slider(32, 32, 8192);
          GUI.enabled = wasEnabled;
        }

        EditorGUILayout.EndHorizontal();

        //EditorGUILayout.PropertyField(targetRenderTextureViewport, new GUIContent("Texture Viewport"));
        //EditorGUILayout.PropertyField(targetRenderTextureScale, new GUIContent("Texture Scale"));

        if ( !Application.isPlaying )
        {
          //  Regenerate Mesh
          EditorGUILayout.BeginHorizontal();
          GUILayout.Space(EditorGUI.indentLevel * TAB_SIZE);

          if(GUILayout.Button("Generate Mesh", GUILayout.Width(120) ))
          {
            foreach(HarmonyRenderer harmonyRenderer in targetObjects)
            {
              GameObject rendererObject = harmonyRenderer.gameObject;
              GenerateHarmonyMesh.CreateOrUpdateTextureObjectMesh(rendererObject, true);
            }
          }

          if (GUILayout.Button("Clear Mesh", GUILayout.Width(120) ))
          {
            foreach(HarmonyRenderer harmonyRenderer in targetObjects)
            {
              GameObject rendererObject = harmonyRenderer.gameObject;
              GenerateHarmonyMesh.ClearTextureObjectMesh(rendererObject);
            }

            //  Components were removed from ui.  Bail out now.
            EditorGUIUtility.ExitGUI();
          }

          EditorGUILayout.EndHorizontal();
        }
      }
      GUI.enabled = true;

      --EditorGUI.indentLevel;
    }

    foldColliders = EditorGUILayout.Foldout(foldColliders, "Colliders");
    if(foldColliders)
    {
      ++EditorGUI.indentLevel;

      EditorGUILayout.PropertyField(targetSyncCollider, new GUIContent("Sync Collider"));
      EditorGUILayout.PropertyField(targetColliderShape, new GUIContent("Collider Shape"));
      EditorGUILayout.PropertyField(targetColliderTrigger, new GUIContent("Collider Trigger"));

      if ( !Application.isPlaying )
      {
        //  Update colliders
        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(EditorGUI.indentLevel * TAB_SIZE);

        if(GUILayout.Button("Generate Colliders", GUILayout.Width(120) ))
        {
          foreach(HarmonyRenderer harmonyRenderer in targetObjects)
          {
            harmonyRenderer.PreCalculateColliders();
          }

          //  Components were removed from ui.  Bail out now.
          EditorGUIUtility.ExitGUI();
        }

        if(GUILayout.Button("Clear Colliders", GUILayout.Width(120)))
        {
          foreach(HarmonyRenderer harmonyRenderer in targetObjects)
          {
            harmonyRenderer.ClearColliders();
          }

          //  Components were removed from ui.  Bail out now.
          EditorGUIUtility.ExitGUI();
        }

        EditorGUILayout.EndHorizontal();
      }

      --EditorGUI.indentLevel;
    }

    foldMetadata = EditorGUILayout.Foldout(foldMetadata, "Metadata");
    if(foldMetadata)
    {
      ++EditorGUI.indentLevel;

      //  Override label width to 1 pixel...
      EditorGUIUtility.labelWidth = 1f;

      for(int i = 0; i < System.Math.Min(targetMetadataKeys.arraySize,targetMetadataLists.arraySize); ++i)
      {
        SerializedProperty targetMetadataKey = targetMetadataKeys.GetArrayElementAtIndex(i);
        SerializedProperty targetMetadataContainer = targetMetadataLists.GetArrayElementAtIndex(i);

        SerializedProperty targetMetadataList = targetMetadataContainer.FindPropertyRelative("metas");

        EditorGUILayout.LabelField(targetMetadataKey.stringValue);

        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(EditorGUI.indentLevel * TAB_SIZE);
        EditorGUILayout.BeginVertical();
        EditorGUILayout.BeginHorizontal("box");

        GUILayout.Label( "name " );
        GUILayout.Label( "node" );
        GUILayout.Label( "value" );

        if(GUILayout.Button("+", GUILayout.Width(16), GUILayout.Height(16) ))
        {
          ++targetMetadataList.arraySize;
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
      }

      if ( !Application.isPlaying )
      {
        //  Update props and anchors
        EditorGUILayout.BeginHorizontal();
        GUILayout.Space(EditorGUI.indentLevel * TAB_SIZE);

        if(GUILayout.Button("Update All Metadata", GUILayout.Width(120) ))
        {
          foreach(HarmonyRenderer harmonyRenderer in targetObjects)
          {
            GameObject rendererObject = harmonyRenderer.gameObject;

            //  Create Metadata.
            GenerateHarmonyMeta.CreateOrUpdatePropsFromMetadata(rendererObject);
            GenerateHarmonyMeta.CreateOrUpdateAnchorsFromMetadata(rendererObject);
            GenerateHarmonyMeta.CreateOrUpdateGenericMetadata(rendererObject);
          }

          serializedObject.Update();
        }

        EditorGUILayout.EndHorizontal();
      }

      --EditorGUI.indentLevel;

      //  Restore label width...
      EditorGUIUtility.labelWidth = 0f;
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
