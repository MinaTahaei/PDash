#include "TR_Utils.h"

#include "STD_Containers.h"
#include "STD_Types.h"

#define kAttrScaleXTag               "scale.x"
#define kAttrScaleYTag               "scale.y"
#define kAttrScaleXYTag              "scale.xy"
#define kAttrRotationAngleZTag       "rotation.anglez"
#define kAttrRotationZTag            "rotation.z"
#define kAttrSkewTag                 "skew"
#define kAttrPositionXYZTag          "position.attr3dpath"
#define kAttrOffsetXYZTag            "offset.attr3dpath"
#define kAttrOffsetTag               "offset"
#define kAttrPositionXTag            "position.x"
#define kAttrOffsetXTag              "offset.x"
#define kAttrPositionYTag            "position.y"
#define kAttrOffsetYTag              "offset.y"
#define kAttrPositionZTag            "position.z"
#define kAttrOffsetZTag              "offset.z"
#define kAttrVelocityTag             "velocity"
#define kAttrPivotTag                "pivot"

#define kAttrRestOffsetXTag          "rest.offset.x"
#define kAttrRestOffsetYTag          "rest.offset.y"
#define kAttrRestLengthTag           "rest.length"
#define kAttrRestRotationAngleZTag   "rest.rotation.anglez"
#define kAttrRestRotationZTag        "rest.rotation.z"

#define kAttrDeformOffsetXTag        "deform.offset.x"
#define kAttrDeformOffsetYTag        "deform.offset.y"
#define kAttrDeformLengthTag         "deform.length"
#define kAttrDeformRotationAngleZTag "deform.rotation.anglez"
#define kAttrDeformRotationZTag      "deform.rotation.z"

#define kAttrOpacityTag              "opacity"

namespace TR_Utils
{

  typedef STD_Map< STD_String, TR_Types::CurveChannel_t > ChannelMap_t;

  static ChannelMap_t g_channels;
  static bool g_initialized = false;

  void initChannels()
  {
    if ( g_initialized == true )
      return;

    g_channels[ kAttrScaleXTag ]                = TR_Types::eScaleX;
    g_channels[ kAttrScaleYTag ]                = TR_Types::eScaleY;
    g_channels[ kAttrScaleXYTag ]               = TR_Types::eScaleXY;
    g_channels[ kAttrRotationAngleZTag ]        = TR_Types::eRotationZ;
    g_channels[ kAttrRotationZTag ]             = TR_Types::eRotationZ;
    g_channels[ kAttrSkewTag ]                  = TR_Types::eSkew;
    g_channels[ kAttrPositionXYZTag ]           = TR_Types::eXYZ;
    g_channels[ kAttrOffsetXYZTag ]             = TR_Types::eXYZ;
    g_channels[ kAttrOffsetTag ]                = TR_Types::eXYZ;
    g_channels[ kAttrVelocityTag ]              = TR_Types::eVelocity;
    g_channels[ kAttrPositionXTag ]             = TR_Types::eSeparateX;
    g_channels[ kAttrOffsetXTag ]               = TR_Types::eSeparateX;
    g_channels[ kAttrPositionYTag ]             = TR_Types::eSeparateY;
    g_channels[ kAttrOffsetYTag ]               = TR_Types::eSeparateY;
    g_channels[ kAttrPositionZTag ]             = TR_Types::eSeparateZ;
    g_channels[ kAttrOffsetZTag ]               = TR_Types::eSeparateZ;
    g_channels[ kAttrPivotTag ]                 = TR_Types::ePivot;

    g_channels[ kAttrRestOffsetXTag ]           = TR_Types::eRestOffsetX;
    g_channels[ kAttrRestOffsetYTag ]           = TR_Types::eRestOffsetY;
    g_channels[ kAttrRestLengthTag ]            = TR_Types::eRestLength;
    g_channels[ kAttrRestRotationAngleZTag ]    = TR_Types::eRestRotation;
    g_channels[ kAttrRestRotationZTag ]         = TR_Types::eRestRotation;

    g_channels[ kAttrDeformOffsetXTag ]         = TR_Types::eDeformOffsetX;
    g_channels[ kAttrDeformOffsetYTag ]         = TR_Types::eDeformOffsetY;
    g_channels[ kAttrDeformLengthTag ]          = TR_Types::eDeformLength;
    g_channels[ kAttrDeformRotationAngleZTag ]  = TR_Types::eDeformRotation;
    g_channels[ kAttrDeformRotationZTag ]       = TR_Types::eDeformRotation;

    g_channels[ kAttrOpacityTag ]               = TR_Types::eOpacity;

    g_initialized = true;
  }
}


bool TR_Utils::nameToChannel( const STD_String &name, TR_Types::CurveChannel_t &channel )
{
  initChannels();

  ChannelMap_t::const_iterator i = g_channels.find( name );
  if ( i != g_channels.end() )
  {
    channel = i->second;
    return true;
  }

  return false;
}

bool TR_Utils::channelToNames( TR_Types::CurveChannel_t channel, NameCol_t &names )
{
  initChannels();

  names.clear();
  for ( ChannelMap_t::const_iterator i = g_channels.begin(), iEnd = g_channels.end() ; i!=iEnd ; ++i )
  {
    if ( i->second == channel )
    {
      names.insert(i->first);
    }
  }

  return !names.empty();
}
