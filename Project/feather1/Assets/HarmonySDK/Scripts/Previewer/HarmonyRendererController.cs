
using UnityEngine;
using System.Collections;

/*!
 *  @class HarmonyRendererController
 *  Helper class to create game objects in the Game Previewer.
 */
[AddComponentMenu("Harmony/Previewer/HarmonyRendererController")]
public class HarmonyRendererController : MonoBehaviour {

  private GameObject rendererObject;
  public float       frameRate = 24.0f;

  IEnumerator Start()
  {
    SceneSettings settings = FindObjectOfType(typeof(SceneSettings)) as SceneSettings;
    if ( settings != null )
    {
      if ( settings.clipNames.Length > 0 )
      {
        rendererObject = new GameObject( "RendererObject" );
        rendererObject.transform.parent = settings.viewerGroup.transform;

        HarmonyRenderer renderer = rendererObject.AddComponent<HarmonyRenderer>();
        HarmonyAnimation animation = rendererObject.AddComponent<HarmonyAnimation>();
        HarmonyAudio audio = rendererObject.AddComponent<HarmonyAudio>();
        AudioSource templateAudioSource = rendererObject.AddComponent<AudioSource>();

        //  Linear rolloff.  Easier for sound to be heard.
        templateAudioSource.rolloffMode = AudioRolloffMode.Linear;

        //  Set up clip collection in renderer.
        renderer.projectFolder = settings.projectFolder;
        renderer.clipNames = settings.clipNames;

        renderer.LoadClipIndex(settings.clipIdx);

        //  Adjust renderer object size to fit in camera.
        Bounds box = renderer.CalculateCurrentBoundingBox();

        float scaleFactor = 5.0f / Mathf.Max( box.size.x, box.size.y );
        rendererObject.transform.localScale = new Vector3( scaleFactor, scaleFactor, 1.0f );

        //  Make sure sound is all downloaded before playing animation.
        yield return StartCoroutine(audio.WaitForDownloads());

        //  Loop animation indefinitely.
        animation.ResetAnimation();
        animation.LoopAnimation( frameRate, settings.clipIdx );
      }
    }
  }

  public IEnumerator RefreshRendererObject()
  {
    if ( rendererObject == null )
      yield break;

    SceneSettings settings = FindObjectOfType(typeof(SceneSettings)) as SceneSettings;
    if ( settings == null )
      yield break;

    HarmonyRenderer renderer = rendererObject.GetComponent<HarmonyRenderer>();
    HarmonyAnimation animation = rendererObject.GetComponent<HarmonyAnimation>();
    HarmonyAudio audio = rendererObject.GetComponent<HarmonyAudio>();
    if ( (renderer == null) || (animation == null) || (audio == null) )
      yield break;

    renderer.LoadClipIndex(settings.clipIdx);

    //  Make sure sound is all downloaded before playing animation.
    yield return StartCoroutine(audio.WaitForDownloads());

    //  Loop animation indefinitely.
    animation.ResetAnimation();
    animation.LoopAnimation( frameRate, settings.clipIdx );
    animation.ResumeAnimation();  // resume if paused.
  }

  public void OnDestroy()
  {
    Destroy(rendererObject);
  }
}
