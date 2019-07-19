
#if (UNITY_IPHONE || UNITY_ANDROID) && !UNITY_EDITOR
#define UNITY_TOUCH_ENABLED
#endif

#define CLONE_ON_TOUCH

using UnityEngine;

using System.Collections;
using System.Runtime.InteropServices;
using System.IO;

using XML_ProjectLoader;

/*!
 *  @class DemoPhysics
 *  Demo that shows how colliders are generated.
 */
[AddComponentMenu("Harmony/Demo/DemoPhysics")]
public class DemoPhysics : MonoBehaviour
{
#if CLONE_ON_TOUCH
  public GameObject gameObjectToClone;
#else
  public string projectFolder = "HarmonyResources/SpaceDuck";
  private string liveProjectFolder;
#endif // CLONE_ON_TOUCH

  private System.Random rand       = new System.Random();
  private Vector3 posAtMouseDown;

#if !UNITY_TOUCH_ENABLED
  private bool isMouseDown         = false;
  private Vector3 posAtMouseMove;
#endif // !UNITY_TOUCH_ENABLED

  public HarmonyRenderer.ColliderShape colliderShape = HarmonyRenderer.ColliderShape.eBox;

  void Start()
  {
    #if !CLONE_ON_TOUCH
    if ( new DirectoryInfo(projectFolder).Exists )
    {
      liveProjectFolder = projectFolder;
    }

    if ( !new DirectoryInfo(projectFolder).Exists )
    {
      string altProjectFolder = Application.persistentDataPath + "/" + projectFolder;
      if ( new DirectoryInfo(altProjectFolder).Exists )
      {
        liveProjectFolder = altProjectFolder;
      }
      else
      {
        altProjectFolder = Application.streamingAssetsPath + "/" + projectFolder;
        if ( new DirectoryInfo(altProjectFolder).Exists )
        {
          liveProjectFolder = altProjectFolder;
        }
        else
        {
          if ( ZIP_AndroidAssets.isUrl(altProjectFolder) )
          {
            string tmpProjectFolder;
            if ( ZIP_AndroidAssets.synchronizeProjectFolder( altProjectFolder, out tmpProjectFolder ) )
            {
              liveProjectFolder = tmpProjectFolder;
            }
          }
        }
      }
    }
    #endif

    GUIText guiText = GetComponent<GUIText>();

    if( !guiText )
    {
      Debug.Log("DemoPhysics needs a GUIText component!");
      return;
    }

    float fontMult = 1.0f / 120.0f;
    int   fontSize = 30;

    guiText.fontSize = (Screen.dpi != 0) ? (int)(Screen.dpi * fontMult * fontSize) : fontSize;
  }

  void Update()
  {
    Camera mainCamera = Camera.main;
    if (mainCamera == null)
      return;

    #if UNITY_TOUCH_ENABLED
    if ( Input.touchCount == 1 )
    {
      if ( Input.GetTouch(0).phase == TouchPhase.Began )
      {
        float zOffset = mainCamera.WorldToScreenPoint(new Vector3(0,0,0)).z;
        posAtMouseDown = new Vector3(Input.GetTouch(0).position.x, Input.GetTouch(0).position.y, zOffset);
      }
      else if ( Input.GetTouch(0).phase == TouchPhase.Ended )
      {
        float zOffset = mainCamera.WorldToScreenPoint(new Vector3(0,0,0)).z;
        Vector3 currentPos = new Vector3(Input.GetTouch(0).position.x, Input.GetTouch(0).position.y, zOffset);

        //  Consider a move of 30 pixel a single click.  Let's be generous for tablet users.
        if ( (posAtMouseDown - currentPos).magnitude < 30.0 )
        {
          Vector3 worldPos = mainCamera.ScreenToWorldPoint(currentPos);
          CreateCharacterAt(worldPos);
        }
      }
    }
    #else
    if ( Input.GetMouseButton(0) )
    {
      if ( isMouseDown )
      {
        float zOffset = mainCamera.WorldToScreenPoint(new Vector3(0,0,0)).z;
        posAtMouseMove = new Vector3( Input.mousePosition.x, Input.mousePosition.y, zOffset );
      }
      else
      {
        float zOffset = mainCamera.WorldToScreenPoint(new Vector3(0,0,0)).z;
        posAtMouseDown = new Vector3( Input.mousePosition.x, Input.mousePosition.y, zOffset );
        posAtMouseMove = posAtMouseDown;
        isMouseDown = true;
      }

    }
    else
    {
      if ( isMouseDown )
      {
        //  Consider a move of 5 pixel a single click
        if ( (posAtMouseDown - posAtMouseMove).magnitude < 5.0 )
        {
          Vector3 worldPos = mainCamera.ScreenToWorldPoint(posAtMouseMove);
          CreateCharacterAt(worldPos);
        }
      }

      isMouseDown = false;
    }
    #endif

    HarmonyRenderer[] rendererObjects = FindObjectsOfType(typeof(HarmonyRenderer)) as HarmonyRenderer[];
    GUIText guiText = GetComponent<GUIText>();

    string format = System.String.Format("{0} Object{1}" , rendererObjects.Length, (rendererObjects.Length > 1) ? "s" : "" );
    guiText.text = format;
    guiText.material.color = Color.green;
  }

  void CreateCharacterAt( Vector3 pos )
  {
    //Message.Log( "Create Character at: " + pos );

    #if CLONE_ON_TOUCH
    GameObject rendererObject = GameObject.Instantiate( gameObjectToClone ) as GameObject;
    rendererObject.transform.localPosition = new Vector3( pos.x, pos.y, pos.z );
    rendererObject.transform.parent = gameObject.transform;

    HarmonyRenderer renderer = rendererObject.GetComponent<HarmonyRenderer>();
    HarmonyAnimation animation = rendererObject.GetComponent<HarmonyAnimation>();
    #else
    GameObject rendererObject = new GameObject( "Space Duck" );
    rendererObject.transform.localScale = new Vector3( 0.01f, 0.01f, 0.01f );
    rendererObject.transform.localPosition = new Vector3( pos.x, pos.y, pos.z );
    rendererObject.transform.parent = gameObject.transform;


    HarmonyRenderer renderer = rendererObject.AddComponent<HarmonyRenderer>();
    HarmonyAnimation animation = rendererObject.AddComponent<HarmonyAnimation>();

    Rigidbody2D rigidBody = rendererObject.AddComponent<Rigidbody2D>();
    PolygonCollider2D collider = rendererObject.AddComponent<PolygonCollider2D>();

    //  Set up clip collection in renderer.
    renderer.projectFolder = liveProjectFolder;
    renderer.clipNames = XML_StageLoader.loadStageClipNames(renderer.projectFolder).ToArray();

    renderer.syncCollider = HarmonyRenderer.SyncCollider.eAlways;
    renderer.colliderShape = colliderShape;
    #endif // CLONE_ON_TOUCH

    //  Randomly play a clip.
    if ( renderer.clipNames.Length > 0 )
      animation.LoopAnimation( 24.0f, rand.Next() % renderer.clipNames.Length );
  }

}
