
using System.Collections.Generic;
using System;

using XML_Types;

/*!
 *  @namespace XML_SaxParser
 */
namespace XML_SaxParser
{
  /*!
   *  @namespace XML_SaxParserComponent
   *  Component interface.
   */
  public class XML_SaxParserComponent
  {
    public virtual XML_SaxParserComponent startElement( string nodeName, List<XML_Attribute> attributes)
    {
      return this;
    }

    public virtual void endElement( string nodeName )
    {
    }

    public virtual void startComponent()
    {
    }

    public virtual void endComponent()
    {
    }

    public string findAttribute( List<XML_Attribute> attributes, string name )
    {
      foreach( XML_Attribute attr in attributes )
      {
        if ( name.Equals( attr._name ) )
        {
          return attr._value;
        }
      }

      return null;
    }
  }
}
