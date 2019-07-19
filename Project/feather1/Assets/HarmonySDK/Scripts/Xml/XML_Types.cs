
/*!
 *  @namespace XML_Types
 *  Common XML types.
 */
namespace XML_Types
{
  public struct XML_Attribute
  {
    public string _name;
    public string _value;
  }

  public struct XML_SoundSequence
  {
    public string _name;
    public float  _startFrame;
  }

  public struct XML_PropMeta
  {
    public string _clipName;
    public string _playName;
  }

  public struct XML_AnchorMeta
  {
    public string _playName;
    public string _nodeName;
  };

  public struct XML_GenericMeta
  {
    public string _name;
    public string _clipName;
    public string _playName;
    public string _nodeName;
    public string _value;
  };

  //  Common tags
  public class XML_Constants
  {
    static public string kNameTag                  = "name";
    static public string kFilenameTag              = "filename";
    static public string kRepeatTag                = "repeat";
    static public string kTimeTag                  = "time";

    //  Stage group main tags
    static public string kStageColTag              = "stages";
    static public string kStageTag                 = "stage";
    static public string kPropColTag               = "props";
    static public string kPlayTag                  = "play";
    static public string kMetaTag                  = "meta";
    static public string kPropTag                  = "prop";
    static public string kAnchorTag                = "anchor";
    static public string kSoundTag                 = "sound";

    //  Stage group variable tags
    static public string kParentSkeletonTag        = "parent";
    static public string kParentBoneTag            = "bone";
    static public string kValueTag                 = "value";

    //  Skeleton group main tags
    static public string kSkeletonColTag           = "skeletons";
    static public string kSkeletonTag              = "skeleton";
    static public string kNodeColTag               = "nodes";
    static public string kNodeTag                  = "node";
    static public string kPegTag                   = "peg";
    static public string kReadTag                  = "read";
    static public string kLinkColTag               = "links";
    static public string kLinkTag                  = "link";

    //  Skeleton group variable tags
    static public string kNodeInTag                = "in";
    static public string kNodeOutTag               = "out";

    //  Drawing animation group main tags
    static public string kDrawingAnimationColTag   = "drawingAnimations";
    static public string kDrawingAnimationTag      = "drawingAnimation";
    static public string kDrawingColTag            = "drawing";
    static public string kDrawingTag               = "drw";

    //  Drawing animation group variable tags
    static public string kFrameTag                 = "frame";
    static public string kDrawingOffsetXCoordTag   = "offsetX";
    static public string kDrawingOffsetYCoordTag   = "offsetY";

    //  Sprite sheet group main tags
    static public string kSpriteSheetColTag        = "spritesheets";
    static public string kSpriteSheetTag           = "spritesheet";
    static public string kSpriteTag                = "sprite";

    //  Sprite sheet group variable tags
    static public string kResolutionTag            = "resolution";
    static public string kRectTag                  = "rect";

    //  Channels
    static public string kAttrScaleXTag            = "scale.x";
    static public string kAttrScaleYTag            = "scale.y";
    static public string kAttrScaleXYTag           = "scale.xy";
    static public string kAttrRotationZTag         = "rotation.anglez";
    static public string kAttrSkewTag              = "skew";
    static public string kAttrPositionXYZTag       = "position.attr3dpath";
    static public string kAttrOffsetXYZTag         = "offset.attr3dpath";
    static public string kAttrOffsetTag            = "offset";
    static public string kAttrPositionXTag         = "position.x";
    static public string kAttrOffsetXTag           = "offset.x";
    static public string kAttrPositionYTag         = "position.y";
    static public string kAttrOffsetYTag           = "offset.y";
    static public string kAttrPositionZTag         = "position.z";
    static public string kAttrOffsetZTag           = "offset.z";
    static public string kAttrVelocityTag          = "velocity";
    static public string kAttrPivotTag             = "pivot";

    //  Animation group main tags
    static public string kAnimationColTag          = "animations";
    static public string kAnimationTag             = "animation";
    static public string kAttrLinkColTag           = "attrlinks";
    static public string kAttrLinkTag              = "attrlink";
    static public string kTimedValueColTag         = "timedvalues";
    static public string kTimedValueTag            = "timedvalue";
    static public string kConstantValueTag         = "value";
    static public string kBezierTag                = "bezier";
    static public string kBezierPtTag              = "pt";
    static public string kCatmullTag               = "catmull";
    static public string kCatmullPtTag             = "pt";
    static public string kLinearTag                = "linear";
    static public string kLinearPtTag              = "pt";
    static public string kPivotTag                 = "pivot";
    static public string kPivotPtTag               = "pt";

    //  Animation group variable tags
    static public string kAttrTag                  = "attr";
    static public string kBezierConstSegTag        = "constSeg";
    static public string kBezierXCoordTag          = "x";
    static public string kBezierYCoordTag          = "y";
    static public string kBezierRXCoordTag         = "rx";
    static public string kBezierRYCoordTag         = "ry";
    static public string kBezierLXCoordTag         = "lx";
    static public string kBezierLYCoordTag         = "ly";
    static public string kCatmullXCoordTag         = "x";
    static public string kCatmullYCoordTag         = "y";
    static public string kCatmullZCoordTag         = "z";
    static public string kCatmullTensionTag        = "tension";
    static public string kCatmullContinuityTag     = "continuity";
    static public string kCatmullBiasTag           = "bias";
    static public string kCatmullTimeTag           = "lockedInTime";
    static public string kCatmullScaleXTag         = "scaleX";
    static public string kCatmullScaleYTag         = "scaleY";
    static public string kCatmullScaleZTag         = "scaleZ";
    static public string kLinearXCoordTag          = "x";
    static public string kLinearYCoordTag          = "y";
    static public string kPivotXCoordTag           = "x";
    static public string kPivotYCoordTag           = "y";
    static public string kPivotZCoordTag           = "z";
    static public string kPivotStartTimeTag        = "start";

  }
}
