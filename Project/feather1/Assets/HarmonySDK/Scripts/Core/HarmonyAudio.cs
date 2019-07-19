
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System;

using XML_ProjectLoader;

/*!
 *  @class HarmonyAudio
 *  Audio scheduler to be used in conjunction with 
 *  the HarmonyRenderer and HarmonyAnimation game object
 *  components.
 */
[AddComponentMenu("Harmony/Core/HarmonyAudio")]
public class HarmonyAudio : MonoBehaviour {

  // Audio subfolder in Resources folder.
  public string audioFolder = "";

  private int audioIndex = 0;

  private class AudioSequence
  {
    public AudioClip  audioClip;
    public float      startFrame;
  }

  private Dictionary< string, List< AudioSequence > > audioSequences = new Dictionary< string, List< AudioSequence > >();

  public class AudioEvent
  {
    public List<AudioSource> audioSources = new List< AudioSource >();
  }

  private static int audioEventId = 0;
  private Dictionary< int, AudioEvent > audioEvents = new Dictionary< int, AudioEvent >();

  private List< AudioSource > audioSourcePool = new List< AudioSource >();

  private List< WWW > scheduledDownloads = new List< WWW >();

  public bool isReady
  {
    get
    {
      //  Remove all finished download request.
      scheduledDownloads.RemoveAll( www => www.isDone );

      return (scheduledDownloads.Count == 0);
    }
  }

  public void ClearAudio()
  {
    audioSequences.Clear();
  }

  public void LoadAudio( string projectFolder, string clipName )
  {
    //  Already loaded, bail out.
    if ( audioSequences.ContainsKey( clipName ) )
      return;

    //  Create entry.
    audioSequences[clipName] = new List< AudioSequence >();

    //ClearAudio();

    //  Load sound sequences from xml data.
    List<XML_Types.XML_SoundSequence> soundSequences = XML_StageLoader.loadSoundSequences(projectFolder, clipName);
    foreach ( XML_Types.XML_SoundSequence soundSequence in soundSequences )
    {
      //Message.Log( "sound sequence: " + soundSequence._name + " startFrame: " + soundSequence._startFrame );

      //  Retrieve audio clip from asset database.  If it's not already in asset database, copy clip
      //  from projectFolder to asset database.
      string soundName = Path.GetFileNameWithoutExtension(soundSequence._name);
      if ( !string.IsNullOrEmpty(audioFolder ) )
      {
        soundName = audioFolder + "/" + soundName;
      }

      AudioClip clip = Resources.Load<AudioClip>(soundName);
      if ( clip == null )
      {
        //  Download audio file into resources.  This is only useful to test Harmony Export in Unity.
        //  For final game, sound files should be copied in Resources.
        string soundFilename = projectFolder + "/audio/" + soundSequence._name;
        if ( File.Exists(soundFilename) )
        {
          Message.Log( "Copying from '" + soundFilename + "'" );

          WWW www = new WWW("file://" + System.Uri.EscapeUriString(soundFilename) );
          scheduledDownloads.Add(www);  //  Add download to list of scheduled downloads.

          clip = www.GetAudioClip(true /*threeD*/, false /*stream*/);
        }
      }

      //  Create new audio sequence with audio clip.
      //  Clip starts with a delay specified by startFrame when animation is initiated.
      if ( clip != null )
      {
        AudioSequence audioSequence = new AudioSequence();
        audioSequence.audioClip = clip;
        audioSequence.startFrame = soundSequence._startFrame;

        audioSequences[clipName].Add(audioSequence);
      }
    }
  }

  public int ScheduleAudioClips( string clipName, float delay, float startFrame, float frameRate )
  {
    //Message.Log( "ScheduleAudioClips delay: " + delay + " startFrame: " + startFrame + " frameRate: " + frameRate );

    if ( !audioSequences.ContainsKey( clipName ) )
      return -1;

    AudioEvent audioEvent = new AudioEvent();

    List< AudioSequence > clipAudioSequences = audioSequences[clipName];

    for ( int i=0 ; i<clipAudioSequences.Count ; ++i )
    {
      AudioSequence audioSequence = clipAudioSequences[i];
      if ( audioSequence.startFrame >= startFrame )
      {
        AudioSource audioSource = null;
        if ( audioSourcePool.Count > 0 )
        {
          audioSource = audioSourcePool[0];
          audioSourcePool.RemoveAt(0);
        }
        else
        {
          string audioName = "audioObject_" + string.Format( "{0:0,0}", ++audioIndex  );

          //  Parent audio game object to current game object.
          GameObject audioObject = new GameObject(audioName);
          audioObject.transform.parent = gameObject.transform;

          //  Reset transform in audioObject
          audioObject.transform.localPosition = Vector3.zero;
          audioObject.transform.localScale = Vector3.one;
          audioObject.transform.localRotation = Quaternion.identity;

          //  Create default AudioSource object.
          audioSource = audioObject.AddComponent<AudioSource>();

          audioSource.playOnAwake = false; // handled by us.
          audioSource.loop = false; // handled by us.

          AudioSource templateAudioSource = gameObject.GetComponent<AudioSource>();
          if ( templateAudioSource != null )
          {
            //  Could not find any better way to copy parameters from template AudioSource
            //  to target AudioSource.  Cannot only Instantiate a Component without its
            //  GameObject, and Type.GetFields returns empty collection for Unity Components.
            audioSource.mute = templateAudioSource.mute;
            audioSource.bypassEffects = templateAudioSource.bypassEffects;
            audioSource.priority = templateAudioSource.priority;
            audioSource.volume = templateAudioSource.volume;
            audioSource.pitch = templateAudioSource.pitch;

            //  3D sound settings.
            audioSource.dopplerLevel = templateAudioSource.dopplerLevel;
            audioSource.minDistance = templateAudioSource.minDistance;
            audioSource.maxDistance = templateAudioSource.maxDistance;
#if UNITY_4_3 || UNITY_4_5 || UNITY_4_6

            audioSource.panLevel = templateAudioSource.panLevel;
#else // UNITY_5_0
            audioSource.spatialBlend = templateAudioSource.spatialBlend;
#endif
            audioSource.spread = templateAudioSource.spread;
            audioSource.rolloffMode = templateAudioSource.rolloffMode;

            //  2D sound settings.
#if UNITY_4_3 || UNITY_4_5 || UNITY_4_6
            audioSource.pan = templateAudioSource.pan;
#else // UNITY_5_0 
            audioSource.panStereo = templateAudioSource.panStereo;
#endif
          }
        }

        audioSource.clip = clipAudioSequences[i].audioClip;

        float clipDelay = ((audioSequence.startFrame-startFrame) / frameRate) + delay;
        audioSource.PlayScheduled(AudioSettings.dspTime + clipDelay);

        audioEvent.audioSources.Add(audioSource);
      }
    }

    if ( audioEvent.audioSources.Count > 0 )
    {
      int id = ++audioEventId;
      audioEvents.Add( id, audioEvent );

      return id;

    }

    return -1;
  }

  public void StopAudioClips( int audioEventId )
  {
    if ( audioEvents.ContainsKey( audioEventId ) )
    {
      AudioEvent audioEvent = audioEvents[audioEventId];

      //  Stop all audio sources in audio event.
      foreach( AudioSource audioSource in audioEvent.audioSources )
      {
        audioSource.Stop();
        audioSource.clip = null;
      }

      //  Remove audio event from collection. 
      audioEvents.Remove(audioEventId);

      //  Reschedule audio source for new audio playback.
      audioSourcePool.AddRange( audioEvent.audioSources );
    }
  }

  public IEnumerator WaitForAudioClips( int audioEventId )
  {
    if ( audioEvents.ContainsKey( audioEventId ) )
    {
      AudioEvent audioEvent = audioEvents[audioEventId];

      //  Wait for all audio sources to finish playing sound.
      foreach( AudioSource audioSource in audioEvent.audioSources )
      {
        while ( audioSource.isPlaying )
        {
          yield return null;
        }

        audioSource.clip = null;
      }

      //  Remove audio event from collection. 
      audioEvents.Remove(audioEventId);

      //  Reschedule audio source for new audio playback.
      audioSourcePool.AddRange( audioEvent.audioSources );
    }
  }

  public IEnumerator WaitForDownloads()
  {
    //  Retrieve and return unfinished download for use in a
    //  coroutine yield statement.
    while ( scheduledDownloads.Count > 0 )
    {
      WWW www = scheduledDownloads[0];
      if ( www.isDone )
      {
        scheduledDownloads.RemoveAt(0);
        continue;
      }

      yield return www;
    }

    //  Make sure that all audio clips are ready to play as well.
    foreach( KeyValuePair<string, List< AudioSequence > > entry in audioSequences )
    {
      foreach( AudioSequence audioSequence in entry.Value )
      {
#if UNITY_4_3 || UNITY_4_5 || UNITY_4_6
        while ( !audioSequence.audioClip.isReadyToPlay )
        {
          yield return null;
        }
#else // UNITY_5_0
        while ( audioSequence.audioClip.loadState != AudioDataLoadState.Loaded &&
                audioSequence.audioClip.loadState != AudioDataLoadState.Failed )
        {
          yield return null;
        }
#endif
      }
    }
  }
}
