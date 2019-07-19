#ifndef _TR_DATA_OBJECT_H_
#define _TR_DATA_OBJECT_H_

#include "TR_Types.h"

#include "MEM_Override.h"

#include <cstdlib>

/*!
 * @class TR_DataObject
 * Base data container object.
 */
class TR_DataObject
{
  MEM_OVERRIDE

public:

  typedef TR_Types::DataOffset_t DataOffset_t;

  TR_DataObject(TR_Types::DataObjectId_t id);

  static size_t g_dataObjectSizeTable[];

public:

  TR_Types::DataObjectId_t  _id;
};

/*!
 * @class TR_NodeDataObject
 * Node data container.
 */
class TR_NodeDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:

  TR_NodeDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  DataOffset_t              _nameOffset;

  DataOffset_t              _channelDataOffset;
  DataOffset_t              _drawingDataOffset;

  DataOffset_t              _effectDataOffset;

  DataOffset_t              _brotherDataOffset;
  DataOffset_t              _childDataOffset;
};

/*!
 * @class TR_EffectDataObject
 * Effect data container.
 */
class TR_EffectDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:

  TR_EffectDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  TR_Types::EffectId_t      _effectId;
  DataOffset_t              _matteDataOffset;
};

/*!
 *  @class TR_ChannelDataObject
 *  Animated channel data container.
 */
class TR_ChannelDataObject : public TR_DataObject
{
public:

  TR_ChannelDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  TR_Types::CurveChannel_t  _channelType;

  DataOffset_t              _linkedDataOffset;
  DataOffset_t              _nextChannelDataOffset;
};

/*!
 * @class TR_BezierCurveDataObject
 * Bezier curve data container
 */
class TR_BezierCurveDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_BezierCurveDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  unsigned                  _nPoints;
};

/*!
 * @class TR_BezierPointDataObject
 * Bezier point data container.
 * @sa TR_BezierCurveDataObject
 */
class TR_BezierPointDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_BezierPointDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  float                     _x, _y,
                            _leftx, _lefty,
                            _rightx, _righty;

  bool                      _constSeg;
};

/*!
 * @class TR_CatmullCurveDataObject
 * Catmull curve data container.
 */
class TR_CatmullCurveDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_CatmullCurveDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  float                     _scaleX, _scaleY, _scaleZ;

  unsigned                  _nPoints;
};

/*!
 * @class TR_CatmullPointDataObject
 * Catmull point data container.
 * @sa TR_CatmullCurveDataObject
 */
class TR_CatmullPointDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_CatmullPointDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  float                     _frame;
  float                     _x, _y, _z,
                            _tension,
                            _continuity,
                            _bias,
                            _distance;

  float                     _d_in_x,
                            _d_in_y,
                            _d_in_z;

  float                     _d_out_x,
                            _d_out_y,
                            _d_out_z;
};

/*!
 * @class TR_LinearCurveDataObject
 * Linear curve data container.
 */
class TR_LinearCurveDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_LinearCurveDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  unsigned                  _nPoints;
};

/*!
 * @class TR_LinearPointDataObject
 * Linear point data container.
 * @sa TR_LinearCurveDataObject
 */
class TR_LinearPointDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_LinearPointDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  float                     _x, _y;
};

/*!
 * @class TR_AnimatedPivotDataObject
 * Pivot collection data container.
 */
class TR_AnimatedPivotDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_AnimatedPivotDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  unsigned                  _nPoints;
};

/*
 * @class TR_PivotPointDataObject
 */
class TR_PivotPointDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_PivotPointDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  float                     _frame,
                            _x, _y, _z;
};

/*!
 * @class TR_DrawingAnimationDataObject
 * Drawing animation data container.
 */
class TR_DrawingAnimationDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_DrawingAnimationDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  unsigned                  _nDrawings;
};

/*!
 * @class TR_DrawingDataObject
 * Drawing data container.
 */
class TR_DrawingDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_DrawingDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  float                     _frame;
  float                     _repeat;

  DataOffset_t              _drawingNameOffset;
};

/*!
 * @class TR_FloatDataObject
 * Float data container.
 */
class TR_FloatDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_FloatDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  float                     _value;
};

/*!
 * @class TR_StringDataObject
 * String data container.
 */
class TR_StringDataObject : public TR_DataObject
{
  MEM_OVERRIDE

public:
  TR_StringDataObject();

  static TR_Types::DataObjectId_t Id();

public:

  unsigned                  _nChars;
};

#endif /* _TR_DATA_OBJECT_H_ */
