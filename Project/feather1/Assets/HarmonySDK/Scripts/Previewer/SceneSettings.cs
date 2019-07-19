
using UnityEngine;

/*!
 *  @class SceneSettings
 *  Scene Settings for the Game Previewer.
 */
[AddComponentMenu("Harmony/Previewer/SceneSettings")]
public class SceneSettings : MonoBehaviour
{
  public string     projectFolder;
  public string[]   clipNames;
  public int        clipIdx;

  public GameObject viewerGroup;
  public GameObject browserGroup;
}
