using UnityEngine;

using System.Collections;
using System.Collections.Generic;
using System.IO;
using System;

using XML_ProjectLoader;

/*!
 *  @class GenerateHarmonyMeta
 */
public class GenerateHarmonyMeta
{
  public static void CreateOrUpdatePropsFromMetadata(GameObject rootObject)
  {
    HarmonyRenderer renderer = rootObject.GetComponent<HarmonyRenderer>();
    if (renderer == null)
      return;

    string projectFolder = renderer.projectFolder;
    if ( !new DirectoryInfo(projectFolder).Exists )
    {
      projectFolder = Application.streamingAssetsPath + "/" + projectFolder;
    }

    //  Load props metadata from XML
    XML_Types.XML_PropMeta[] xmlProps = XML_StageLoader.loadPropMeta(projectFolder).ToArray();
    if ( xmlProps.Length > 0 )
    {
      string propObjectName = "Props";
      GameObject propContainerObject = null;

      Transform[] childTransforms = rootObject.GetComponentsInChildren<Transform>(true /*includeInactive*/);
      foreach ( Transform childTransform in childTransforms )
      {
        GameObject childObject = childTransform.gameObject;

        //  Direct child to root object
        if (childObject.transform.parent == rootObject.transform)
        {
          //  Object matching preset Props game object name.
          if (childObject.name == propObjectName)
          {
            propContainerObject = childObject;
            break;
          }
        }
      }

      HarmonyProp[] propComponents = rootObject.GetComponentsInChildren<HarmonyProp>(true /*includeInactive*/);

      foreach ( XML_Types.XML_PropMeta xmlProp in xmlProps )
      {
        if ( !Array.Exists( propComponents, prop => (prop.clipName == xmlProp._clipName) && (prop.playName == xmlProp._playName) ) )
        {
          if (propContainerObject == null)
          {
            //  Create container game object from props child to renderer object.
            propContainerObject = new GameObject( propObjectName );
            propContainerObject.transform.parent = rootObject.transform;
          }

          GameObject propObject = new GameObject( xmlProp._playName );
          propObject.transform.parent = propContainerObject.transform;

          HarmonyProp propComponent = propObject.AddComponent<HarmonyProp>();

          propComponent.clipName = xmlProp._clipName;
          propComponent.playName = xmlProp._playName;
        }
      }
    }

  }

  public static void CreateOrUpdateAnchorsFromMetadata(GameObject rootObject)
  {
    HarmonyRenderer renderer = rootObject.GetComponent<HarmonyRenderer>();
    if (renderer == null)
      return;

    string projectFolder = renderer.projectFolder;
    if ( !new DirectoryInfo(projectFolder).Exists )
    {
      projectFolder = Application.streamingAssetsPath + "/" + projectFolder;
    }

    //  Load anchors metadata from XML
    XML_Types.XML_AnchorMeta[] xmlAnchors = XML_StageLoader.loadAnchorMeta(projectFolder).ToArray();
    if ( xmlAnchors.Length > 0 )
    {
      string anchorObjectName = "Anchors";
      GameObject anchorContainerObject = null;

      Transform[] childTransforms = rootObject.GetComponentsInChildren<Transform>(true /*includeInactive*/);
      foreach ( Transform childTransform in childTransforms )
      {
        GameObject childObject = childTransform.gameObject;

        //  Direct child to root object
        if (childObject.transform.parent == rootObject.transform)
        {
          //  Object matching preset Anchors game object name.
          if (childObject.name == anchorObjectName)
          {
            anchorContainerObject = childObject;
            break;
          }
        }
      }

      HarmonyAnchor[] anchorComponents = rootObject.GetComponentsInChildren<HarmonyAnchor>(true /*includeInactive*/);

      foreach ( XML_Types.XML_AnchorMeta xmlAnchor in xmlAnchors )
      {
        if ( !Array.Exists( anchorComponents, anchor => (anchor.playName == xmlAnchor._playName) && (anchor.nodeName == xmlAnchor._nodeName) ) )
        {
          if (anchorContainerObject == null)
          {
            //  Create container game object from anchors child to renderer object.
            anchorContainerObject = new GameObject( anchorObjectName );
            anchorContainerObject.transform.parent = rootObject.transform;
          }

          GameObject anchorObject = new GameObject( xmlAnchor._nodeName );
          anchorObject.transform.parent = anchorContainerObject.transform;

          HarmonyAnchor anchorComponent = anchorObject.AddComponent<HarmonyAnchor>();

          anchorComponent.playName = xmlAnchor._playName;
          anchorComponent.nodeName = xmlAnchor._nodeName;
        }
      }
    }
  }

  public static void CreateOrUpdateGenericMetadata(GameObject rootObject)
  {
    HarmonyRenderer renderer = rootObject.GetComponent<HarmonyRenderer>();
    if (renderer == null)
      return;

    string projectFolder = renderer.projectFolder;
    if ( !new DirectoryInfo(projectFolder).Exists )
    {
      projectFolder = Application.streamingAssetsPath + "/" + projectFolder;
    }

    //  Load custom metadata from XML
    XML_Types.XML_GenericMeta[] xmlMetas = XML_StageLoader.loadGenericMeta(projectFolder).ToArray();
    if ( xmlMetas.Length > 0 )
    {
      foreach ( XML_Types.XML_GenericMeta xmlMeta in xmlMetas )
      {
        HarmonyMeta.MetaEntry entry = new HarmonyMeta.MetaEntry();
        entry.metaName = xmlMeta._name;
        entry.nodeName = xmlMeta._nodeName;
        entry.metaValue = xmlMeta._value;

        if ( Array.Exists(renderer.clipNames, clipName => clipName == xmlMeta._clipName) )
        {
          if ( !renderer.metadata.ContainsKey( xmlMeta._clipName ) )
            renderer.metadata[ xmlMeta._clipName ] = new HarmonyMeta();

          if ( renderer.metadata[ xmlMeta._clipName ].GetMeta(entry.metaName, entry.nodeName) == null )
            renderer.metadata[ xmlMeta._clipName ].AddMeta(entry);
        }
        else
        {
          HarmonyProp[] propComponents = rootObject.GetComponentsInChildren<HarmonyProp>();
          foreach( HarmonyProp propComponent in propComponents )
          {
            if ( propComponent.playName == xmlMeta._playName )
            {
              if ( propComponent.metadata.GetMeta(entry.metaName, entry.nodeName) == null )
                propComponent.metadata.AddMeta(entry);
              break;
            }
          }
        }
      }
    }
  }
}

