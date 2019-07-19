
using UnityEngine;
using System.Collections;
using System.Collections.Generic;

using WebGL_FS;

/*!
 *  @class DemoCallback
 *  Demo that shows how to use callbacks with the HarmonyAnimation component.
 */
[AddComponentMenu("Harmony/Demo/DemoCallback")]
public class DemoCallback : MonoBehaviour {

  public float frameRate = 24.0f;
  public string locatorName;
#if (UNITY_WEBGL) && !UNITY_EDITOR
    private List<string> projectsBeingPrepared = new List<string>();

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
                projectsDone.Add(projectFolder);
            }else if (stateOfResource == 1){
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
                postStart();
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

   void postStart()
    {
      HarmonyRenderer renderer = GetComponent<HarmonyRenderer>();
      //  Preemptively load clip.
      renderer.LoadClipIndex(0 /* first clip */);

      HarmonyAnimation animation = gameObject.GetComponent<HarmonyAnimation>();
      if (animation != null)
      {
        HarmonyAnimation.CallbackEvents callbacks = new HarmonyAnimation.CallbackEvents();

        //  Trigger a callback at frame 10 of animation.
        callbacks.AddCallbackAtFrame(10.0f, CallbackMethod1);

        //  Trigget a callback at end of animation.
        callbacks.AddCallbackAtEnd(CallbackMethod2);

        //  Loop animation indefinitely.
        animation.LoopAnimation(frameRate, 0 /* first clip */, 1.0f /*first frame*/, false /*forward*/, callbacks);
      }
      renderer.readyToRender = true;
    }

    IEnumerator Start()
    {
        //  Preemptively load clips.
        HarmonyRenderer renderer = GetComponent<HarmonyRenderer>();

        if ((renderer == null) || (renderer.clipNames == null))
            yield break;

#if UNITY_WEBGL && !UNITY_EDITOR
            if (WebGL_FileSystem.GetPreparationStateOfResource(renderer.projectFolder)!=3)
            {
                renderer.readyToRender = false;
                PrepareProjectFolder(renderer.projectFolder);
            }else{
                postStart();
            }  
#else
        postStart();
#endif
    }

    void CallbackMethod1( GameObject sender )
    {
        HarmonyAnchor anchor = sender.GetComponentInChildren<HarmonyAnchor>();
        if ( anchor != null )
        {
            //  Create a projectile at locator location and following same direction.
            GameObject projectile = GameObject.CreatePrimitive(PrimitiveType.Sphere);
            projectile.AddComponent<Rigidbody>();

            projectile.transform.localPosition = anchor.gameObject.transform.TransformPoint( 0, 0, 0 );

            Vector3 direction = anchor.gameObject.transform.localToWorldMatrix.MultiplyVector( Vector3.right ).normalized;
            projectile.GetComponent<Rigidbody>().AddForce( direction * 1000 );

            //  Destroy projectile after 2 seconds.
            Destroy(projectile, 2);
        }
    }

    void CallbackMethod2( GameObject sender )
    {
        HarmonyAnchor anchor = sender.GetComponentInChildren<HarmonyAnchor>();
        if ( anchor != null )
        {
            //  Create a projectile at anchor location and following same direction.
            GameObject projectile = GameObject.CreatePrimitive(PrimitiveType.Cube);
            projectile.AddComponent<Rigidbody>();

            projectile.transform.localPosition = anchor.gameObject.transform.TransformPoint( 0, 0, 0 );

            Vector3 direction = anchor.gameObject.transform.localToWorldMatrix.MultiplyVector( Vector3.right ).normalized;
            projectile.GetComponent<Rigidbody>().AddForce( direction * 500 );

            //  Destroy projectile after 2 seconds.
            Destroy(projectile, 2);
        }
    }
}
