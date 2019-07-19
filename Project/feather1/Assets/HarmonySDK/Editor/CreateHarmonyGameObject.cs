using UnityEditor;
using UnityEngine;

using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.IO;
using System;

using XML_ProjectLoader;

public class CreateHarmonyGameObject : MonoBehaviour
{
  static string projectFolder = "";

  // Add a menu item named "Do Something" to MyMenu in the menu bar.
  [MenuItem ("GameObject/Harmony/Harmony Object", false, 10)]
  static void Run()
  {
    projectFolder = EditorUtility.OpenFolderPanel("Import Harmony Game Engine Project", projectFolder, "" );
    if ( !string.IsNullOrEmpty(projectFolder) &&
         new DirectoryInfo(projectFolder).Exists )
    {
      string[] clipNames = XML_StageLoader.loadStageClipNames(projectFolder).ToArray();

      if ( clipNames.Length > 0 )
      {
        string name = Path.GetFileName( projectFolder );

        GameObject rendererObject = new GameObject( name );
        rendererObject.transform.localScale = new Vector3( 1.0f, 1.0f, 1.0f );

        HarmonyRenderer renderer = rendererObject.AddComponent<HarmonyRenderer>();
        /*HarmonyAnimation animation = */rendererObject.AddComponent<HarmonyAnimation>();

        //  Do not create audio components if there is no audio sequences in specified project folder...
        bool needAudio = false;
        foreach ( string clipName in clipNames )
        {
          List<XML_Types.XML_SoundSequence> soundSequences = XML_StageLoader.loadSoundSequences(projectFolder, clipName);
          if (soundSequences.Count > 0)
          {
            needAudio = true;
            break;
          }
        }

        if (needAudio)
        {
          /*HarmonyAudio audio = */rendererObject.AddComponent<HarmonyAudio>();
          /*AudioSource audioSource = */rendererObject.AddComponent<AudioSource>();
        }

        //  Remove streaming assets path part of the specified project folder if
        //  applicable.  An absolute path will work on the user's machine but will
        //  likely not be found elsewhere.
        string streamingAssetsPath = Application.streamingAssetsPath;
        string rendererProjectFolder = projectFolder;
        if ( rendererProjectFolder.Contains( streamingAssetsPath ) )
        {
          rendererProjectFolder = rendererProjectFolder.Substring( streamingAssetsPath.Length + 1 );
        }

        renderer.projectFolder = rendererProjectFolder;
        renderer.clipNames = clipNames;

        //  Create Metadata.
        GenerateHarmonyMeta.CreateOrUpdatePropsFromMetadata(rendererObject);
        GenerateHarmonyMeta.CreateOrUpdateAnchorsFromMetadata(rendererObject);
        GenerateHarmonyMeta.CreateOrUpdateGenericMetadata(rendererObject);

        //  If no Harmony cameras available, ask user if he wants to create one.
        HarmonyCamera[] harmonyCameras = FindObjectsOfType<HarmonyCamera>();
        if (harmonyCameras.Length == 0)
        {
          string title = "Create HarmonyCamera components?";
          string body = "Only a camera with the HarmonyCamera component will render Harmony Game Objects.";

          if ( EditorUtility.DisplayDialog( title, body, "Create", "Do Not Create") )
          {
            //  Make sure there is at least one camera in the scene.
            Camera[] cameras = FindObjectsOfType<Camera>();
            if (cameras.Length == 0)
            {
              GameObject cameraObject = new GameObject( "Main Camera" );
              /*Camera camera = */cameraObject.AddComponent<Camera>();
            }

            CreateHarmonyCamera.ShowWindow();
          }
        }
      }
    }
  }
}
