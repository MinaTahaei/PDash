
using UnityEngine;

using System.Collections.Generic;
using System.Xml;
using System.IO;
using System;

using XML_SaxParser;
using XML_StageSaxParser;
using XML_Types;

/*!
 *  @namespace XML_ProjectLoader
 *  XML Project Loader.
 */
namespace XML_ProjectLoader
{

  /*!
   *  @class XML_SaxParserDelegator
   */
  public class XML_SaxParserDelegator
  {
    public XML_SaxParserDelegator(XML_SaxParserComponent component)
    {
      _components = new List<XML_SaxParserComponent>();
      _components.Add(component);
    }

    ~XML_SaxParserDelegator()
    {
      _components.RemoveAt(_components.Count-1);
    }

    public void startElement( string nodeName, List<XML_Attribute> attributes)
    {
      XML_SaxParserComponent lastComponent = _components[_components.Count-1];
      XML_SaxParserComponent newComponent = lastComponent.startElement(nodeName, attributes);

      if (lastComponent != newComponent)
      {
        newComponent.startComponent();
      }

      _components.Add(newComponent);
    }

    public void endElement( string nodeName )
    {
      XML_SaxParserComponent lastComponent = _components[_components.Count-1];
      lastComponent.endElement(nodeName);

      _components.RemoveAt(_components.Count-1);

      if ((_components.Count == 0) || (lastComponent != _components[_components.Count-1]))
      {
        lastComponent.endComponent();
      }

    }

    private List<XML_SaxParserComponent> _components;
  }

  /*!
   *  @class XML_CommonLoader
   */
  public class XML_CommonLoader
  {
    public static void parseXmlFile( string filename, XML_SaxParserComponent mainComponent )
    {
      XML_SaxParserDelegator delegator = new XML_SaxParserDelegator(mainComponent);

      XmlTextReader reader = new XmlTextReader(filename);
      while (reader.Read())
      {
        switch (reader.NodeType)
        {
          case XmlNodeType.Element: // The node is an element.

           List<XML_Attribute> attributes = new List<XML_Attribute>();

           if (reader.HasAttributes)
           {
             for (int i = 0; i < reader.AttributeCount; i++)
             {
               reader.MoveToAttribute(i);

               XML_Attribute attribute = new XML_Attribute();
               attribute._name = reader.Name;
               attribute._value = reader.Value;

               attributes.Add(attribute);
             }
             reader.MoveToElement(); //Moves the reader back to the element node.
           }

           if (reader.IsEmptyElement)
            {
              delegator.startElement( reader.Name, attributes );
              delegator.endElement( reader.Name );
            }
            else
            {
              delegator.startElement( reader.Name, attributes );
            }

            break;
          case XmlNodeType.Text: //Display the text in each element.
            //Debug.LogWarning(reader.Value);
            break;
          case XmlNodeType.EndElement: //Display the end of the element.

            delegator.endElement( reader.Name );
            break;
        }
      }
    }
  }

  /*!
   *  @class XML_StageLoader
   *  Access to data contained in stage.xml.
   */
  public class XML_StageLoader
  {
    public static List<string> loadStageClipNames( string projectFolder )
    {
      XML_StageNamesSaxParserComponent mainComponent = new XML_StageNamesSaxParserComponent();

      string filename = projectFolder + "/stage.xml";
      if ( new FileInfo(filename).Exists )
      {
        XML_CommonLoader.parseXmlFile( filename, mainComponent );

        List<string> clipNames = mainComponent.clipNames;
        return clipNames;
      }

      return new List<string>();  //  empty list.
    }

    public static List<XML_Types.XML_SoundSequence> loadSoundSequences( string projectFolder, string clipName )
    {
      XML_AudioSaxParserComponent mainComponent = new XML_AudioSaxParserComponent(clipName);
      string filename = projectFolder + "/stage.xml";

      if ( new FileInfo(filename).Exists )
      {
        XML_CommonLoader.parseXmlFile( filename, mainComponent );

        List<XML_Types.XML_SoundSequence> soundSequences = mainComponent.soundSequences;
        return soundSequences;
      }

      return new List<XML_Types.XML_SoundSequence>();  // empty list.
    }

    public static List<XML_Types.XML_PropMeta> loadPropMeta( string projectFolder )
    {
      XML_MetaSaxParserComponent mainComponent = new XML_MetaSaxParserComponent();
      string filename = projectFolder + "/stage.xml";

      if ( new FileInfo(filename).Exists )
      {
        XML_CommonLoader.parseXmlFile( filename, mainComponent );

        List<XML_Types.XML_PropMeta> props = mainComponent.props;
        return props;
      }

      return new List<XML_Types.XML_PropMeta>();  // empty list.
    }

    public static List<XML_Types.XML_AnchorMeta> loadAnchorMeta( string projectFolder )
    {
      XML_MetaSaxParserComponent mainComponent = new XML_MetaSaxParserComponent();
      string filename = projectFolder + "/stage.xml";

      if ( new FileInfo(filename).Exists )
      {
        XML_CommonLoader.parseXmlFile( filename, mainComponent );

        List<XML_Types.XML_AnchorMeta> anchors = mainComponent.anchors;
        return anchors;
      }

      return new List<XML_Types.XML_AnchorMeta>();  // empty list.
    }

    public static List<XML_Types.XML_GenericMeta> loadGenericMeta( string projectFolder )
    {
      XML_MetaSaxParserComponent mainComponent = new XML_MetaSaxParserComponent();
      string filename = projectFolder + "/stage.xml";

      if ( new FileInfo(filename).Exists )
      {
        XML_CommonLoader.parseXmlFile( filename, mainComponent );

        List<XML_Types.XML_GenericMeta> metas = mainComponent.metas;
        return metas;
      }

      return new List<XML_Types.XML_GenericMeta>();  // empty list.
    }
  }
}
