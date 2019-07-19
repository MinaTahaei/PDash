
using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;


/*!
 *  @class HarmonyAnimation
 *  Animation scheduler to be used in conjunction with the
 *  HarmonyRenderer game object component.
 */
[AddComponentMenu("Harmony/Core/HarmonyAnimation")]
public class HarmonyAnimation : MonoBehaviour
{
  public delegate void CallbackEventDelegate( GameObject sender );

  /*!
   *  @class CallbackEvent
   *  Base Callback Event.
   */
  public abstract class CallbackEvent
  {
    public CallbackEventDelegate callback;
  }

  /*!
   *  @class CallbackEvents
   *  Callback collection for an animation clip.
   */
  public class CallbackEvents : List<CallbackEvent>
  {
    public void AddCallbackAtEnd( CallbackEventDelegate callback )
    {
      Add( new FrameCallbackEvent( -1.0f /*last frame*/, callback ) );
    }
    public void AddCallbackAtFrame( float frame, CallbackEventDelegate callback )
    {
      Add( new FrameCallbackEvent( frame, callback ) );
    }
    public void AddCallbackAtDuration( float duration, CallbackEventDelegate callback )
    {
      Add( new DurationCallbackEvent( duration, callback ) );
    }

    //public List<CallbackEvent> callbacks = new List<CallbackEvent>();
  }

  /*!
   *  @class FrameCallbackEvent
   *  Callback Event triggered at specified frame.
   */
  private class FrameCallbackEvent : CallbackEvent
  {
    public FrameCallbackEvent( float frame, CallbackEventDelegate callback )
    {
      this.callback = callback;
      this.frame = frame;
    }

    public float frame;
  }

  /*!
   *  @class DurationCallbackEvent
   *  Callback Event triggered at specified duration.
   */
  private class DurationCallbackEvent : CallbackEvent
  {
    public DurationCallbackEvent( float duration, CallbackEventDelegate callback )
    {
      this.callback = callback;
      this.duration = duration;
    }

    public float duration;
  }

  /*!
   *  @class AnimationClip
   *  Single Animation Clip
   */
  protected class AnimationClip
  {
    public GameObject         gameObject;

    public float              frameRate;
    public string             clipName;
    public float              startFrame;
    public float              duration;
    public int                nTimes;
    public bool               reverse;

    public  float             currentDuration;

    public  float             currentFrame
    {
      get
      {
        if (reverse)
          return startFrame + (duration-currentDuration) * frameRate;

        return startFrame + currentDuration * frameRate;
      }
    }

    public  float             nFrames
    {
      get { return duration * frameRate; }
    }

    private HarmonyAudio      audio;
    private List<int>         audioEvents = new List<int>();

    /*!
     *  @class DelayCallbackEvent
     *  Internal callback event.
     */
    private class DelayCallbackEvent
    {
      public DelayCallbackEvent( float delay, CallbackEventDelegate callback )
      {
        this.callback = callback;
        this.delay = delay;
      }

      public CallbackEventDelegate     callback;
      public float                     delay;
    }

    private List<DelayCallbackEvent> callbacks;

    public AnimationClip( GameObject gameObject, float frameRate, string clipName, float startFrame, float duration, int nTimes, bool reverse, CallbackEvents callbacks)
    {
      this.gameObject      = gameObject;

      this.frameRate       = frameRate;
      this.clipName        = clipName;
      this.startFrame      = startFrame;
      this.duration        = duration;
      this.nTimes          = nTimes;
      this.reverse         = reverse;

      if ( callbacks != null )
      {
        float startDuration = (startFrame - 1.0f) / frameRate;

        this.callbacks = new List<DelayCallbackEvent>();
        foreach ( CallbackEvent callbackEvent in callbacks )
        {
          DurationCallbackEvent durationCallbackEvent = callbackEvent as DurationCallbackEvent;
          if ( durationCallbackEvent != null )
          {
            float delay = durationCallbackEvent.duration - startDuration;
            if ( delay <= duration )
            {
              this.callbacks.Add( new DelayCallbackEvent( delay, durationCallbackEvent.callback ) );
            }
          }
          else
          {
            FrameCallbackEvent frameCallbackEvent = callbackEvent as FrameCallbackEvent;
            if ( frameCallbackEvent != null )
            {
              float delay = (frameCallbackEvent.frame < 0) ? duration : (frameCallbackEvent.frame - 1) / frameRate;
              if ( delay <= duration )
              {
                this.callbacks.Add( new DelayCallbackEvent( delay, frameCallbackEvent.callback ) );
              }
            }
          }
        }
      }

      this.currentDuration = 0.0f;
    }

    ~AnimationClip()
    {
    }

    public void InitializeAudio( HarmonyAudio audio )
    {
      this.audio = audio;
    }

    public void DeinitializeAudio()
    {
      if (audio != null)
      {
        foreach( int audioEventId in audioEvents )
        {
          //  Stop audio immediately.
          //audio.StopAudioClips( audioEventId );

          //  Wait for audio to end.
          audio.StartCoroutine(audio.WaitForAudioClips(audioEventId));
        }
      }

      audioEvents.Clear();
    }

    public void ScheduleAudioEvent( float delay )
    {
      if (audio != null)
      {
        int audioEventId = audio.ScheduleAudioClips( clipName, delay, startFrame, frameRate );
        if ( audioEventId >= 0 )
        {
          PushAudioEvent( audioEventId );
        }
      }
    }

    public void PushAudioEvent( int id )
    {
      audioEvents.Add(id);
    }

    public void PopAudioEvent()
    {
      if ((audio != null) && (audioEvents.Count > 0))
      {
        //  Stop audio immediately.
        //audio.StopAudioClips( audioEvents[0] );

        //  Wait for audio to end.
        audio.StartCoroutine(audio.WaitForAudioClips(audioEvents[0]));

        audioEvents.RemoveAt(0);
      }
    }

    public void ClearAudioEvents()
    {
      if (audio == null)
        return;

      while (audioEvents.Count > 0)
      {
        audio.StopAudioClips( audioEvents[0] );
        audioEvents.RemoveAt(0);
      }
    }

    public void TriggerCallbacks( float duration0, float duration1 )
    {
      if ( callbacks != null )
      {
        foreach( DelayCallbackEvent callback in callbacks )
        {
          if ( (callback.delay >= duration0) && (callback.delay < duration1) )
          {
            //  trigger callback
            callback.callback( gameObject );
          }
        }
      }
    }

    public float Step( float dt )
    {
      //  Static frame display.
      if ( duration == 0.0f )
        return (nTimes < 0) ? 0.0f : dt;

      TriggerCallbacks( currentDuration, currentDuration + dt );

      currentDuration += dt;

      //  Push new audio event for next loop
      if ( (audioEvents.Count < 2) && ((nTimes > 1) || (nTimes < 0)) )
      {
        float delay = (duration - currentDuration);
        ScheduleAudioEvent(delay);
      }

      //  Handle animation looping.
      while ( currentDuration > duration )
      {
        PopAudioEvent();
        TriggerCallbacks( 0, currentDuration - duration );

        if ( nTimes == 1 )
        {
          break;
        }
        else if ( nTimes > 1 )
        {
          --nTimes;
        }

        currentDuration -= duration;
      }

      float ret_dt = 0.0f;
      if ( currentDuration > duration )
      {
        ret_dt = currentDuration - duration;
        currentDuration = duration;
      }

      return ret_dt;
    }
  }

  private List<AnimationClip> animationClips = new List<AnimationClip>();

  private  float  framePriv = 1.0f;
  private  string clipNamePriv;

  public   bool   paused = false;

  public   int    scheduledCount
  {
    get { return animationClips.Count; }
  }

  public   float  frame
  {
    get { return framePriv; }
  }

  public   string clipName
  {
    get { return clipNamePriv; }
  }

  public   bool isPlaying
  {
    get { return ((animationClips.Count > 0) && !paused); }
  }

  public   bool isStopped
  {
    get { return (animationClips.Count == 0); }
  }

  public   bool isPaused
  {
    get { return paused; }
  }

  public void Step( float dt )
  {
    while (!paused && (dt > 0.0f) && (animationClips.Count > 0) )
    {
      AnimationClip currentClip = animationClips[0];
      dt = currentClip.Step(dt);

      clipNamePriv = currentClip.clipName;
      framePriv = currentClip.currentFrame;

      if ( framePriv < 1.0f )
        framePriv = 1.0f;
      else if ( framePriv >= (currentClip.startFrame + currentClip.nFrames) )
        framePriv = (currentClip.startFrame + currentClip.nFrames - 0.01f);

      if ( dt > 0.0f )
      {
        currentClip.DeinitializeAudio();
        animationClips.Remove(currentClip);
      }
    }
  }

  public void PlayAnimation( float frameRate, string clipName, float startFrame = 1.0f, int nTimes = 1, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipName, startFrame, -1.0f /*full length*/, nTimes, reverse, callbacks );
  }

  public void PlayFrames( float frameRate, string clipName, float startFrame, float nFrames, int nTimes = 1, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipName, startFrame, (frameRate > 0.0f) ? (nFrames / frameRate) : 0.0f, nTimes, reverse, callbacks );
  }

  public void PlayDuration( float frameRate, string clipName, float startFrame, float duration, int nTimes = 1, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipName, startFrame, duration, nTimes, reverse, callbacks );
  }

  public void PlayStaticFrame( float staticFrame, string clipName, float duration, CallbackEvents callbacks = null )
  {
    PlayStaticFramePriv( staticFrame, clipName, duration, callbacks );
  }

  public void LoopAnimation( float frameRate, string clipName, float startFrame = 1.0f, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipName, startFrame, -1.0f /*full length*/, -1, reverse, callbacks );
  }

  public void LoopFrames( float frameRate, string clipName, float startFrame, float nFrames, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipName, startFrame, (frameRate > 0.0f) ? (nFrames / frameRate) : 0.0f, -1, reverse, callbacks );
  }

  public void LoopDuration( float frameRate, string clipName, float startFrame, float duration, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipName, startFrame, duration, -1, reverse, callbacks );
  }

  public void LoopStaticFrame( float staticFrame, string clipName )
  {
    PlayStaticFramePriv( staticFrame, clipName, -1.0f /*forever*/, null /*no callbacks available*/ );
  }

  public void SetStaticFrame( float staticFrame, string clipName )
  {
    PlayStaticFramePriv( staticFrame, clipName, 0.01f /*fake time*/, null /*no callbacks available*/ );
  }

  public void PlayAnimation( float frameRate, int clipIdx, float startFrame = 1.0f, int nTimes = 1, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipIdx, startFrame, -1.0f /*full length*/, nTimes, reverse, callbacks );
  }

  public void PlayFrames( float frameRate, int clipIdx, float startFrame, float nFrames, int nTimes = 1, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipIdx, startFrame, (frameRate > 0.0f) ? (nFrames / frameRate) : 0.0f, nTimes, reverse, callbacks );
  }

  public void PlayDuration( float frameRate, int clipIdx, float startFrame, float duration, int nTimes = 1, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipIdx, startFrame, duration, nTimes, reverse, callbacks );
  }

  public void PlayStaticFrame( float staticFrame, int clipIdx, float duration, CallbackEvents callbacks = null )
  {
    PlayStaticFramePriv( staticFrame, clipIdx, duration, callbacks );
  }

  public void LoopAnimation( float frameRate, int clipIdx, float startFrame = 1.0f, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipIdx, startFrame, -1.0f /*full length*/, -1, reverse, callbacks );
  }

  public void LoopFrames( float frameRate, int clipIdx, float startFrame, float nFrames, bool reverse = false,  CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipIdx, startFrame, (frameRate > 0.0f) ? (nFrames / frameRate) : 0.0f, -1, reverse, callbacks );
  }

  public void LoopDuration( float frameRate, int clipIdx, float startFrame, float duration, bool reverse = false, CallbackEvents callbacks = null )
  {
    PlayAnimationPriv( frameRate, clipIdx, startFrame, duration, -1, reverse, callbacks );
  }

  public void LoopStaticFrame( float staticFrame, int clipIdx )
  {
    PlayStaticFramePriv( staticFrame, clipName, -1.0f /*forever*/, null /*no callbacks available*/ );
  }

  public void SetStaticFrame( float staticFrame, int clipIdx )
  {
    PlayStaticFramePriv( staticFrame, clipName, 0.01f /*fake time*/, null /*no callbacks available*/ );
  }

  public void StopAnimation()
  {
    //  Stop scheduled audio clips.
    foreach( AnimationClip clip in animationClips )
    {
      clip.ClearAudioEvents();
    }

    //  Remove all animation clips from scheduled list.
    animationClips.Clear();
  }

  public void ResetAnimation()
  {
    //  Stop animation and set frame back to 1.0f
    StopAnimation();
    framePriv = 1.0f;
  }

  public void CancelAnimation()
  {
    //  Let last animation clip play to its end, and
    //  cancel rest of scheduled animation.
    if ( animationClips.Count > 0 )
    {
      //  Reschedule clip to be on its last lap.
      AnimationClip currentClip = animationClips[0];
      currentClip.nTimes = 1;

      while( animationClips.Count > 1 )
      {
        //  Stop scheduled audio clips.
        AnimationClip scheduledClip = animationClips[1];
        scheduledClip.ClearAudioEvents();

        animationClips.RemoveAt(1);
      }
    }
  }

  public void PauseAnimation()
  {
    paused = true;
  }

  public void ResumeAnimation()
  {
    paused = false;
  }

  public void PauseResumeAnimation()
  {
    paused = !paused;
  }

  private void PlayAnimationPriv( float frameRate, string clipName, float startFrame, float duration, int nTimes, bool reverse, CallbackEvents callbacks )
  {
    HarmonyRenderer renderer = GetComponent<HarmonyRenderer>();
    if (renderer != null)
    {
      //  Load clip at index if not already loaded.
      renderer.LoadClipName(clipName);

      //  Negative duration, replace with clip full duration.
      if ( duration < 0.0f )
      {
        float nFrames = renderer.GetClipFrameCount(clipName);
        duration =  (frameRate > 0.0f) ? ((nFrames + 1.0f - startFrame) / frameRate) : 0.0f;
      }
    }

    if ( duration > 0.0f )
    {
      AnimationClip clip = new AnimationClip( gameObject, frameRate, clipName, startFrame, duration, nTimes, reverse, callbacks );
      PlayPriv(clip);
    }
  }

  private void PlayStaticFramePriv( float staticFrame, string clipName, float duration, CallbackEvents callbacks )
  {
    HarmonyRenderer renderer = GetComponent<HarmonyRenderer>();
    if (renderer != null)
    {
      //  Load clip at index if not already loaded.
      renderer.LoadClipName(clipName);
    }

    if ( duration < 0.0f )
    {
      AnimationClip clip = new AnimationClip( gameObject, 0.0f, clipName, staticFrame, 1.0f /*don't care*/, -1, false /*forward*/, callbacks );
      PlayPriv(clip);
    }
    else
    {
      AnimationClip clip = new AnimationClip( gameObject, 0.0f, clipName, staticFrame, duration, 1, false /*forward*/, callbacks );
      PlayPriv(clip);
    }
  }

  private void PlayAnimationPriv( float frameRate, int clipIdx, float startFrame, float duration, int nTimes, bool reverse, CallbackEvents callbacks )
  {
    HarmonyRenderer renderer = GetComponent<HarmonyRenderer>();
    if (renderer != null && renderer.clipNames != null)
    {
      if ( clipIdx < renderer.clipNames.Length )
      {
        string clipName = renderer.clipNames[clipIdx];
        PlayAnimationPriv( frameRate, clipName, startFrame, duration, nTimes, reverse, callbacks );
      }
    }
  }

  private void PlayStaticFramePriv( float staticFrame, int clipIdx, float duration, CallbackEvents callbacks )
  {
    HarmonyRenderer renderer = GetComponent<HarmonyRenderer>();
    if (renderer != null && renderer.clipNames != null)
    {
      if ( clipIdx < renderer.clipNames.Length )
      {
        string clipName = renderer.clipNames[clipIdx];
        PlayStaticFramePriv( staticFrame, clipName, duration, callbacks );
      }
    }
  }

  private void PlayPriv( AnimationClip newClip )
  {
    HarmonyAudio audio = GetComponent<HarmonyAudio>();
    if ( audio != null )
    {
      newClip.InitializeAudio(audio);

      float delay = 0.0f;
      foreach( AnimationClip clip in animationClips )
      {
        //  infinite loop .. will not be able to play any clips afterwards unless it's stopped.
        if ( clip.nTimes < 0 )
        {
          delay = -1.0f; // invalid delay
          break;
        }

        delay += (clip.duration - clip.currentDuration) + (clip.nTimes-1) * clip.duration;
      }

      if ( delay >= 0.0f )
      {
        newClip.ScheduleAudioEvent(delay);
      }
    }

    animationClips.Add(newClip);
  }
}
