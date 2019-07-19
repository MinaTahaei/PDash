
using UnityEngine;

/*!
 *  @class Message
 *  Display messages to Unity console. 
 */
public class Message
{
  public static void Log( object message, Object context )
  {
    if ( Debug.isDebugBuild )
      Debug.Log(message, context);
  }

  public static void Log( object message )
  {
    if ( Debug.isDebugBuild )
      Debug.Log(message);
  }

  public static void LogWarning( object message, Object context )
  {
    if ( Debug.isDebugBuild )
      Debug.LogWarning(message, context);
  }

  public static void LogWarning( object message )
  {
    if ( Debug.isDebugBuild )
      Debug.LogWarning(message);
  }

  public static void LogError( object message, Object context )
  {
    if ( Debug.isDebugBuild )
      Debug.LogError(message, context);
  }

  public static void LogError( object message )
  {
    if ( Debug.isDebugBuild )
      Debug.LogError(message);
  }
}
