
using UnityEngine;
using System.Collections;

/*!
 *  @class LoopAll
 *  Loop animation in all game objects in the scene.
 */
[AddComponentMenu("Harmony/Utils/LoopAll")]
public class LoopAll : MonoBehaviour {

  public float frameRate = 24.0f;

  IEnumerator Start()
  {
    //  Preemptively load clips.
    HarmonyRenderer renderer = GetComponent<HarmonyRenderer>();
    if ( (renderer == null) || (renderer.clipNames == null) )
      yield break;

    //yield return StartCoroutine(renderer.WaitForDownloads());

    foreach( string clipName in renderer.clipNames )
    {
      renderer.LoadClipName(clipName);
    }

    //  Wait for audio if necessary.
    HarmonyAudio audio = GetComponent<HarmonyAudio>();
    if ( audio != null )
    {
      yield return StartCoroutine(audio.WaitForDownloads());
    }
  }

  void Update()
  {
    HarmonyAudio audio = GetComponent<HarmonyAudio>();
    if ( (audio != null) && !audio.isReady )
      return;

    HarmonyAnimation animation = GetComponent<HarmonyAnimation>();
    if (animation == null)
      return;

    HarmonyRenderer renderer = GetComponent<HarmonyRenderer>();
    if ( (renderer == null) || (renderer.clipNames == null) )
      return;

    //if ( !renderer.isReady )
    //  return;

    //  Almost at end of our scheduled list, reschedule a new sequence.
    if (renderer.clipNames.Length > animation.scheduledCount)
    {
      //  Play all clips
      foreach( string clipName in renderer.clipNames )
      {
        animation.PlayAnimation( frameRate, clipName );
      }
    }
  }
}