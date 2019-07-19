#ifndef _TR_TYPES_H_
#define _TR_TYPES_H_

/*!
 * @namespace TR_Types
 * Common Data types and constants.
 */
namespace TR_Types
{
  /*!
   * @enum DataObjectId_t
   * Data object identifiers.
   */
  enum DataObjectId_t
  {
    eNullData       = 0,
    eNodeData,
    eChannelData,
    eBezierCurveData,
    eBezierPointData,
    eCatmullCurveData,
    eCatmullPointData,
    eLinearCurveData,
    eLinearPointData,
    eAnimatedPivotData,
    ePivotPointData,
    eDrawingAnimationData,
    eDrawingData,
    eFloatData,
    eStringData,
    eEffectData,

    MAX_IDS
  };

  /*!
   * @enum CurveChannel_t
   * Animation channel identifiers.
   */
  enum CurveChannel_t
  {
    eNullChannel    = 0,

    eSeparateX,
    eSeparateY,
    eSeparateZ,
    eXYZ,
    eVelocity,
    eScaleX,
    eScaleY,
    eScaleXY,
    eRotationZ,
    eSkew,
    ePivot,

    eRestOffsetX,
    eRestOffsetY,
    eRestLength,
    eRestRotation,

    eDeformOffsetX,
    eDeformOffsetY,
    eDeformLength,
    eDeformRotation,

    eOpacity,

    MAX_CURVE_CHANNELS
  };

  /*!
   * @enum EffectId_t
   * Effect identifiers.
   */
  enum EffectId_t
  {
    eNoop           = 0,
    eCutter         = 1<<0,
    eInverseCutter  = 1<<1,
    eDeformation    = 1<<2,

    eAllCutters     = eCutter | eInverseCutter
  };

  typedef unsigned NameId_t;

  typedef int DataOffset_t;
  typedef int DataRef_t;

  extern int g_nullOffset;
}

#endif /* _TR_TYPES_H_ */
