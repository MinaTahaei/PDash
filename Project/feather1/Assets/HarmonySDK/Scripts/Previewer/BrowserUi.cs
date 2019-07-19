
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

using XML_ProjectLoader;

using WebGL_FS;

/*!
 *  @class BrowserUi
 *  File browser for Game Previewer.
 */
[AddComponentMenu("Harmony/Previewer/BrowserUi")]
public class BrowserUi : MonoBehaviour {

  public string rootFolder;

  private int topSpacing = 40;
  private int bottomSpacing = 10;
  private int leftSpacing = 10;
  private int rightSpacing = 10;

  private int buttonSpacing = 10;

  private enum StateButton{ PRESSED, RELEASED, NONE };

  private StateButton buttonState;
  private Vector2 beginTouch;
  private float deltaTouch;
  private float deltaOld;
  private bool isFirstTouch;

  private List<string> projectFolders = new List<string>();

#if (UNITY_WEBGL) && !UNITY_EDITOR
  private List<string> projectsBeingPrepared = new List<string>();
  private int lastPreparedProjectFolderidx = -1;

  void PreparationChecker()
  {
    int numberOfProjectFoldersBeingPrepared = projectsBeingPrepared.Count;
    List<string> projectsDone = new List<string>();
    for (int i = 0; i < numberOfProjectFoldersBeingPrepared; i++)
    {
      string projectFolder = projectsBeingPrepared[i];
      int stateOfResource = WebGL_FileSystem.GetPreparationStateOfResource(projectFolder);
      if (stateOfResource == 3)
      {
        Debug.Log("ProjectFolder successfully prepared: "+projectFolder);
        projectsDone.Add(projectFolder);
      }
      else if (stateOfResource == 1)
      {
        Debug.Log("ProjectFolder Failed to Download: "+projectFolder);
        //restart download:
        WebGL_FileSystem.PrepareProjectFolderResource(projectFolder);
      }

    }
  
    for (int i = 0; i < projectsDone.Count; i++)
    {
      projectsBeingPrepared.Remove(projectsDone[i]);
    }
  
    if (projectsBeingPrepared.Count != numberOfProjectFoldersBeingPrepared)
    {
      if (projectsBeingPrepared.Count == 0)
      {
        CancelInvoke("PreparationChecker");
        //for (int i = 0; i < projectFolders.Count; i++)
        //{
        //    string cProject = projectFolders[i];
        //    if (!WebGL_FileSystem.ResourceReadyCheck(cProject))
        //    {
        //        PrepareProjectFolder(cProject);
        //        break;
        //    }
        //}
      }          
    }
  }


  void PrepareProjectFolder(string projectFolder)
  {
    projectsBeingPrepared.Add(projectFolder);
    WebGL_FileSystem.PrepareProjectFolderResource(projectFolder);
    InvokeRepeating("PreparationChecker", 1, 1);
  }
#endif

  void gatherProjectFolders()
  { 
    projectFolders.Clear();
#if (UNITY_WEBGL) && !UNITY_EDITOR
    projectFolders.AddRange(WebGL_FileSystem.GetDirectories(rootFolder));
#else
    if (new DirectoryInfo(rootFolder).Exists)
    {
      projectFolders.AddRange(Directory.GetDirectories(rootFolder));
    }

    if (!new DirectoryInfo(rootFolder).Exists)
    {
      string altProjectFolder = Application.persistentDataPath + "/" + rootFolder;
      if (new DirectoryInfo(altProjectFolder).Exists)
      {
        projectFolders.AddRange(Directory.GetDirectories(altProjectFolder));
      }

      altProjectFolder = Application.streamingAssetsPath + "/" + rootFolder;
      if (new DirectoryInfo(altProjectFolder).Exists)
      {
        projectFolders.AddRange(Directory.GetDirectories(altProjectFolder));
      }
      else
      {
#if UNITY_ANDROID
        if ( ZIP_AndroidAssets.isUrl(altProjectFolder) )
        {
          string[] dirList = ZIP_AndroidAssets.retrieveProjectFolders(altProjectFolder);
          foreach( string dir in dirList )
          {
            projectFolders.Add( altProjectFolder + "/" + dir );
          }
        }
#endif
      }
    }
#endif
  }

  private void Awake()
  {
    gatherProjectFolders();
#if (UNITY_WEBGL) && !UNITY_EDITOR
    for (int i = 0; i < projectFolders.Count; i++)
    {
      string cProject = projectFolders[i];
      if (!WebGL_FileSystem.ResourceReadyCheck(cProject))
      {
        lastPreparedProjectFolderidx = i;
        PrepareProjectFolder(cProject);
        break;
      }
    }
#endif
  }

  private void OnEnable()
  {
    isFirstTouch = true;
    buttonState = StateButton.NONE;
    deltaOld = topSpacing;
    deltaTouch = 0;
  }

  private bool ResolveProjectFolder( string inProjectFolder, out string outProjectFolder )
  {
    outProjectFolder = null;

#if (UNITY_WEBGL) && !UNITY_EDITOR
    if (WebGL_FileSystem.ResourceReadyCheck(inProjectFolder))
    {
      outProjectFolder = inProjectFolder;
      return true;
    }
    else
    {
      return false;
    }
#endif

    if ( string.IsNullOrEmpty(inProjectFolder) )
      return false;

    if ( new DirectoryInfo(inProjectFolder).Exists )
    {
      outProjectFolder = inProjectFolder;
      return true;
    }

#if UNITY_ANDROID
    if ( ZIP_AndroidAssets.isUrl(inProjectFolder) )
    {
      if ( ZIP_AndroidAssets.synchronizeProjectFolder( inProjectFolder, out outProjectFolder ) )
        return true;
    }
#endif

    return false;
  }

  private void Update()
  {
    if(!Input.mousePresent) {
      Touch[] touch = Input.touches;

      if(touch.Length <= 0) {
        if(!isFirstTouch) {
          deltaOld += deltaTouch;
          deltaTouch = 0;
        }
        isFirstTouch = true;
        return;
      }

      deltaTouch -= touch[0].deltaPosition.y;

      if( isFirstTouch ) {
        isFirstTouch = false;
      }
    } else {
      if(Input.GetMouseButtonDown(0))
      {
        buttonState = StateButton.PRESSED;
      }
      if(Input.GetMouseButtonUp(0))
      {
        buttonState = StateButton.RELEASED;
      }
      switch(buttonState)
      {
        case StateButton.PRESSED:
          if( isFirstTouch )
          {
            beginTouch = Input.mousePosition;
            isFirstTouch = false;
          }

          Vector2 pos = Input.mousePosition;
          deltaTouch = -pos.y + beginTouch.y;
          break;
        case StateButton.RELEASED:
          if(!isFirstTouch) {
            deltaOld += deltaTouch;
            deltaTouch = 0;
          }
          isFirstTouch = true;
          break;
        case StateButton.NONE:
          break;
      }
    }
  }

  private void OnGUI ()
  {
    Camera mainCamera = Camera.main;

    float width = mainCamera.pixelWidth;
    float height = mainCamera.pixelHeight;

    SceneSettings settings = FindObjectOfType(typeof(SceneSettings)) as SceneSettings;
    if ( settings == null )
    {
      // no settings available, let's create new game object.
      GameObject settingsObject = new GameObject( "Settings" );
      settings = settingsObject.AddComponent<SceneSettings>();
    }

    int nProjects = projectFolders.Count;

    float fontMult = 1.0f / 120.0f;
    int   fontSize = 15;

    GUIStyle buttonStyle = new GUIStyle(GUI.skin.GetStyle("button"));
    buttonStyle.fontSize = (Screen.dpi != 0) ? (int)(Screen.dpi * fontMult * fontSize) : fontSize;

    GUIStyle boxStyle = new GUIStyle(GUI.skin.GetStyle("box"));
    boxStyle.fontSize = (Screen.dpi != 0) ? (int)(Screen.dpi * fontMult * fontSize) : fontSize;

    int boxWidth = (int)(width / 3);

    int buttonWidth = boxWidth - leftSpacing - rightSpacing;
    int buttonHeight = buttonWidth / 5;

    int boxHeight = topSpacing + (buttonHeight * nProjects) + (buttonSpacing * nProjects-1) + bottomSpacing;

    // Top of the menu
    if( (deltaTouch + deltaOld) > topSpacing) {
      deltaOld = topSpacing;
      deltaTouch = 0;
    }

    //Bottom of the menu
    if( -(deltaTouch + deltaOld) > boxHeight+bottomSpacing-height ) {
      deltaOld = -(boxHeight+bottomSpacing-height);
      deltaTouch = 0;
    }

    Rect boxRect = new Rect( (width - boxWidth) / 2, deltaTouch+deltaOld , boxWidth, boxHeight );
    //Rect boxRect = new Rect( (width - boxWidth) / 2, (height - boxHeight) / 2, boxWidth, boxHeight );

    //  Make a background box.
    GUIContent boxText = new GUIContent("Loader Menu");
    GUI.Box(boxRect, boxText, boxStyle);

    for ( int i = 0 ; i < nProjects ; ++i )
    {
      Rect buttonRect = new Rect( boxRect.x + leftSpacing,
                                  boxRect.y + topSpacing + i * (buttonHeight + buttonSpacing),
                                  buttonWidth,
                                  buttonHeight );

      GUIContent buttonText = new GUIContent(new DirectoryInfo((projectFolders[i] as string)).Name);

      string temp;
      if (!ResolveProjectFolder(projectFolders[i], out temp))
      {
        continue;
      }

      if ( GUI.Button( buttonRect, buttonText, buttonStyle ) )
      {
        if ( ResolveProjectFolder( projectFolders[i], out settings.projectFolder ) )
        {
          settings.clipNames = XML_StageLoader.loadStageClipNames(settings.projectFolder).ToArray();
          settings.clipIdx = 0;
        }
#if UNITY_WEBGL && !UNITY_EDITOR
        lastPreparedProjectFolderidx ++;
        lastPreparedProjectFolderidx = lastPreparedProjectFolderidx % projectFolders.Count;
        PrepareProjectFolder(projectFolders[lastPreparedProjectFolderidx]);
#endif

        //  Load new scene.  Enable viewer group game object hierarchy.
        settings.viewerGroup.SetActive(true);
        settings.browserGroup.SetActive(false);
      }
    }
  }
}
