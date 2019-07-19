
using System.Collections.Generic;
using System.Globalization;
using System;

using XML_Types;
using XML_SaxParser;
using XML_ProjectLoader;

namespace XML_StageSaxParser
{
  /*!
   *  @class XML_StageNamesSaxParserComponent
   *  Retrieve list of clip names available in project
   */
  public class XML_StageNamesSaxParserComponent : XML_SaxParserComponent
  {
    enum State
    {
      eRoot,
      eStages,
      eStage
    };

    public XML_StageNamesSaxParserComponent()
    {
      _clipNames = new List<string>();
    }

    public override XML_SaxParserComponent startElement( string nodeName, List<XML_Attribute> attributes )
    {
      if ( _state == State.eRoot )
      {
        if ( nodeName.Equals( XML_Constants.kStageColTag ) )
        {
          _state = State.eStages;
        }
      }
      else if ( _state == State.eStages )
      {
        if ( nodeName.Equals( XML_Constants.kStageTag ) )
        {
          //  Only enter stage mode if we encounter the xml tag we search for.
          string clipName = findAttribute( attributes, XML_Constants.kNameTag );
          if ( !string.IsNullOrEmpty(clipName) )
          {
            _clipNames.Add(clipName);
            _state = State.eStage;
          }
        }
      }

      return this;
    }

    public override void endElement( string nodeName )
    {
      if ( nodeName.Equals( XML_Constants.kStageTag ) )
      {
        _state = State.eStages;
      }
      else if ( nodeName.Equals( XML_Constants.kStageColTag ) )
      {
        _state = State.eRoot;
      }
    }

    public List<string> clipNames
    {
      get { return _clipNames; }
    }

    private State        _state;
    private List<string> _clipNames;
  }

  /*!
   *  @class XML_AudioSaxParserComponent
   *  Retrieve list of sound play sequences in clip
   */
  public class XML_AudioSaxParserComponent : XML_SaxParserComponent
  {
    enum State
    {
      eRoot,
      eStages,
      eStage
    };

    public XML_AudioSaxParserComponent( string clipName )
    {
      _clipName = clipName;
      _state = State.eRoot;

      _soundSequences = new List< XML_Types.XML_SoundSequence >();
    }

    public override XML_SaxParserComponent startElement( string nodeName, List<XML_Attribute> attributes )
    {
      if ( _state == State.eRoot )
      {
        if ( nodeName.Equals( XML_Constants.kStageColTag ) )
        {
          _state = State.eStages;
        }
      }
      else if ( _state == State.eStages )
      {
        if ( nodeName.Equals( XML_Constants.kStageTag ) )
        {
          //  Only enter stage mode if we encounter the xml tag we search for.
          string value = findAttribute(attributes, XML_Constants.kNameTag);
          if ( !string.IsNullOrEmpty(value) && value.Equals(_clipName) )
          {
            _state = State.eStage;
          }
        }
      }
      else // if ( _state == eStage )
      {
        if ( nodeName.Equals( XML_Constants.kSoundTag ) )
        {
          XML_Types.XML_SoundSequence seq = new XML_Types.XML_SoundSequence();
          seq._startFrame = 1.0f;

          foreach (XML_Attribute attr in attributes)
          {
            if ( attr._name.Equals( XML_Constants.kNameTag ) )
              seq._name = attr._value;
            else if ( attr._name.Equals( XML_Constants.kTimeTag ) )
              seq._startFrame = float.Parse( attr._value, CultureInfo.InvariantCulture.NumberFormat);
          }

          if ( !string.IsNullOrEmpty(seq._name) )
          {
            _soundSequences.Add(seq);
          }
        }
      }

      return this;

    }

    public override void endElement( string nodeName )
    {
      if ( nodeName.Equals( XML_Constants.kStageTag ) )
      {
        _state = State.eStages;
      }
      else if ( nodeName.Equals( XML_Constants.kStageColTag ) )
      {
        _state = State.eRoot;
      }
    }

    public List< XML_Types.XML_SoundSequence > soundSequences
    {
      get { return _soundSequences; }
    }

    private string                 _clipName;
    private State                  _state;

    private List< XML_Types.XML_SoundSequence >  _soundSequences;
  }

  /*!
   *  @class XML_MetaSaxParserComponent
   *  Retrieve list of meta in clip
   */
  public class XML_MetaSaxParserComponent : XML_SaxParserComponent
  {
    enum State
    {
      eRoot,
      eStages,
      eStage
    };

    public XML_MetaSaxParserComponent( )
    {
      _state     = State.eRoot;

      _props     = new List<XML_PropMeta>();
      _anchors   = new List<XML_AnchorMeta>();
      _metas     = new List<XML_GenericMeta>();
    }

    public override XML_SaxParserComponent startElement( string nodeName, List<XML_Attribute> attributes )
    {
      if ( _state == State.eRoot )
      {
        if ( nodeName.Equals( XML_Constants.kStageColTag ) )
        {
          _state = State.eStages;
        }
      }
      else if ( _state == State.eStages )
      {
        if ( nodeName.Equals( XML_Constants.kStageTag ) ||
             nodeName.Equals( XML_Constants.kPropColTag ) )
        {
          _clipName = findAttribute( attributes, XML_Constants.kNameTag );
          if ( !string.IsNullOrEmpty(_clipName) )
          {
            _state = State.eStage;
          }
        }
      }
      else // if ( _state == eStage )
      {
        if ( nodeName.Equals( XML_Constants.kPropTag ) )
        {
          XML_PropMeta meta;
          meta._playName = findAttribute( attributes, XML_Constants.kNameTag );
          if ( !string.IsNullOrEmpty(meta._playName) )
          {
            meta._clipName = _clipName;

            _props.Add(meta);
          }
        }
        else if ( nodeName.Equals( XML_Constants.kAnchorTag ) )
        {
          XML_AnchorMeta meta;
          meta._nodeName = findAttribute( attributes, XML_Constants.kNodeTag );
          meta._playName = findAttribute( attributes, XML_Constants.kPlayTag );
          if ( !string.IsNullOrEmpty(meta._nodeName) && !string.IsNullOrEmpty(meta._playName) )
          {
            if ( _anchors.IndexOf(meta) < 0 )
            {
              _anchors.Add(meta);
            }
          }
        }
        else if ( nodeName.Equals( XML_Constants.kMetaTag ) )
        {
          XML_GenericMeta meta;
          meta._name = findAttribute( attributes, XML_Constants.kNameTag );
          if ( !string.IsNullOrEmpty(meta._name) )
          {
            meta._clipName = _clipName;
            meta._value = findAttribute( attributes, XML_Constants.kValueTag );
            meta._nodeName = findAttribute( attributes, XML_Constants.kNodeTag );
            meta._playName = findAttribute( attributes, XML_Constants.kPlayTag );

            _metas.Add(meta);
          }
        }
      }

      return this;
    }

    public override void endElement( string nodeName )
    {
      if ( nodeName.Equals( XML_Constants.kStageTag ) )
      {
        _state = State.eStages;
        _clipName = string.Empty;
      }
      else if ( nodeName.Equals( XML_Constants.kStageColTag ) )
      {
        _state = State.eRoot;
      }
    }

    public List<XML_PropMeta> props
    {
      get { return _props; }
    }

    public List<XML_AnchorMeta> anchors
    {
      get { return _anchors; }
    }

    public List<XML_GenericMeta> metas
    {
      get { return _metas; }
    }

    private State                       _state;
    private string                      _clipName;

    private List<XML_PropMeta>          _props;
    private List<XML_AnchorMeta>        _anchors;
    private List<XML_GenericMeta>       _metas;
  }

}
