using UnityEngine;
using System.Collections;

using WebGL_FS;

/*!
 *  @class ViewerUi
 *  Ui for the Game Previewer.
 */
[AddComponentMenu("Harmony/Previewer/ViewerUi")]
public class ViewerUi : MonoBehaviour {

  private float    fontMult = 1.0f / 120.0f;
  private int      fontSize = 30;

  private GUIStyle buttonStyle;
  private GUIStyle labelStyle;

  private bool initialized = false;

  void OnEnable()
  {
    //  Make sure we create a new controller for current scene.
    HarmonyRendererController controller = FindObjectOfType(typeof(HarmonyRendererController)) as HarmonyRendererController;
    if (controller == null)
    {
      GameObject controllerObject = new GameObject( "RendererController" );
      controllerObject.transform.parent = transform.parent;

      controller = controllerObject.AddComponent<HarmonyRendererController>();
    }
  }

  void OnGUI ()
  {
    SceneSettings settings = FindObjectOfType(typeof(SceneSettings)) as SceneSettings;
    HarmonyAnimation animation = FindObjectOfType(typeof(HarmonyAnimation)) as HarmonyAnimation;
    HarmonyRendererController controller = FindObjectOfType(typeof(HarmonyRendererController)) as HarmonyRendererController;

    //  Update font size according to dpi.
    if ( !initialized )
    {
      int realFontSize = (Screen.dpi != 0) ? (int)(Screen.dpi * fontMult * fontSize) : fontSize;

      //  Setup styles.
      labelStyle = new GUIStyle(GUI.skin.GetStyle("label"));
      labelStyle.fontSize = realFontSize;
      labelStyle.wordWrap = false;

      buttonStyle = new GUIStyle(GUI.skin.GetStyle("button"));
      buttonStyle.fontSize = realFontSize;
      buttonStyle.stretchWidth = false;

      initialized = true;
    }

    float width = Screen.width;
    float height = Screen.height;

    //  Add button to go back to main browser scene.
    GUIContent buttonText = new GUIContent("Go Back");
    Rect buttonRect = GUILayoutUtility.GetRect(buttonText, buttonStyle);

    int buttonWidth = (int)(buttonRect.width + 1);
    int buttonHeight = (int)(buttonRect.height + 1);

    //  Lower right corner
    buttonRect = new Rect( width - buttonWidth - 5,
                           height - buttonHeight - 5,
                           buttonWidth,
                           buttonHeight );

    if (GUI.Button( buttonRect, buttonText, buttonStyle ))
    {
      //  Exit scene.  Delete controller object and enable
      //  browser group hierarchy.

      if (controller != null)
        Destroy(controller.gameObject);
#if UNITY_WEBGL && !UNITY_EDITOR
      WebGL_FileSystem.RemovePreparedResource(settings.projectFolder);
#endif
      settings.viewerGroup.SetActive(false);
      settings.browserGroup.SetActive(true);
    }

    //  If more than 1 clip, add browser for previous/next clip.
    if ( settings != null )
    {
      int nClips = settings.clipNames.Length;

      //  Add button for previous clip.
      GUI.enabled = ( settings.clipIdx >= 1 );
      {
        buttonText = new GUIContent("Prev");
        buttonRect = GUILayoutUtility.GetRect(buttonText, buttonStyle);

        buttonWidth = (int)(buttonRect.width + 1);
        buttonHeight = (int)(buttonRect.height + 1);

        //  Lower middle side
        buttonRect = new Rect( (width / 2) - buttonWidth - 5,
                               height - ((buttonHeight + 5) * 2),
                               buttonWidth,
                               buttonHeight );

        if (GUI.Button( buttonRect, buttonText, buttonStyle ))
        {
          --settings.clipIdx;

          //  Update clip index and restart animation.
          if ( controller != null )
            StartCoroutine(controller.RefreshRendererObject());
        }
      }

      //  Add button for next clip.
      GUI.enabled = ( settings.clipIdx < (nClips-1) );
      {
        buttonText = new GUIContent("Next");
        buttonRect = GUILayoutUtility.GetRect(buttonText, buttonStyle);

        buttonWidth = (int)(buttonRect.width + 1);
        buttonHeight = (int)(buttonRect.height + 1);

        //  Lower middle side
        buttonRect = new Rect( (width / 2) + 5,
                               height - ((buttonHeight + 5) * 2),
                               buttonWidth,
                               buttonHeight );

        if (GUI.Button( buttonRect, buttonText, buttonStyle ))
        {
          ++settings.clipIdx;

          //  Update clip index and restart animation.
          if ( controller != null )
            StartCoroutine(controller.RefreshRendererObject());
        }
      }

      GUI.enabled = true;
    }

    //  Add button for play/pause animation.
    GUI.enabled = ( animation != null );

    buttonText = new GUIContent( ( (animation == null) || animation.paused == true ) ? "Play" : "Pause" );
    buttonRect = GUILayoutUtility.GetRect(buttonText, buttonStyle);

    buttonWidth = (int)(buttonRect.width + 1);
    buttonHeight = (int)(buttonRect.height + 1);

    //  Lower middle side
    buttonRect = new Rect( (width - buttonWidth) / 2,
                           height - buttonHeight - 5,
                           buttonWidth,
                           buttonHeight );

    if (GUI.Button( buttonRect, buttonText, buttonStyle ))
    {
      if ( animation != null )
      {
        //  Pause or Resume animations.
        animation.PauseResumeAnimation();

        //  Change pause state of AudioListener as well.
        AudioListener.pause = animation.paused;
      }
    }

    GUI.enabled = true;

    //  Add label for current clip.
    if ( settings != null )
    {
      if ( settings.clipIdx >= 0 && settings.clipIdx < settings.clipNames.Length )
      {
        GUIContent labelText = new GUIContent( settings.clipNames[settings.clipIdx] );
        Rect labelRect = GUILayoutUtility.GetRect(labelText, labelStyle);

        int labelWidth = (int)labelRect.width;
        int labelHeight = (int)labelRect.height;

        //  Upper middle side.
        labelRect = new Rect( (width - labelWidth) / 2,
                              5,
                              labelWidth,
                              labelHeight );

        GUI.Label( labelRect, labelText, labelStyle );
      }
    }
  }
}
