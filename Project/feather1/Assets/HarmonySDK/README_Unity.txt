
Harmony Game Engine Integration in Unity
Copyright© 2016 Toon Boom Animation Inc.
========================================

The integration of the Harmony Game SDK in Unity requires the use
of a native c++ plugin that will handle all the rendering and management
of the Harmony assets.  A pro version of Unity 4 (4.5 or higher) or a
standard version of Unity 5 is required to use the native c++ plugin.

Disclaimer
----------

The Harmony Game SDK makes use of the following libraries:

- libpng (http://www.libpng.org/pub/png/src/libpng-LICENSE.txt)
- zlib (http://www.zlib.net/zlib_license.html)
- GLEW (http://glew.sourceforge.net/glew.txt,
        http://glew.sourceforge.net/mesa.txt,
        http://glew.sourceforge.net/khronos.txt)
- Clipper (http://www.boost.org/LICENSE_1_0.txt)

The Harmony Game SDK integration in Unity makes use of the
following libraries:

- DotNetZip (http://dotnetzip.codeplex.com/license)
- mono i18N (http://www.mono-project.com/docs/faq/licensing/)

Compatibility
-------------

- Mac OSX 32/64 (OpenGLCore)
- Windows 32/64 (DirectX9, DirectX11, OpenGLCore)
- iOS 32/64 (OpenGLES2.0, OpenGLES3.0, Metal)
- Android arm/x86 (OpenGLES2.0, OpenGLES3.0)

- Tested with Unity v.5.1.4, 5.2.4, 5.3.3

Project Setup
-------------

To communicate with the plugin and to schedule animation events, the
following subset of scripts must be present in the Unity project Assets
directory:

  * Core/HarmonyRenderer.cs     (Component)
  * Core/HarmonyCamera.cs       (Component)
  * Core/HarmonyAnimation.cs    (Component)
  * Core/HarmonyAudio.cs        (Component)
  * Core/HarmonyLocator.cs      (Component)
  * Core/HarmonyProp.cs         (Component)
  * Core/HarmonyAnchor.cs       (Component)
  * Xml/XML_ProjectLoader.cs
  * Xml/XML_SaxParser.cs
  * Xml/XML_StageSaxParser.cs
  * Xml/XML_Types.cs
  * Zip/ZIP_AndroidAssets.cs
  * Zip/ZIP_Operations.cs
  * Util/Message.cs

Furthermore, all projects folders exported from Toon Boom Harmony must be
copied in the StreamingAssets directory.  The reason for this is that Assets
managed by Unity won't be available on the file system once the game is published.
As the native plugin does not have access to Unity database, it will not be
able to retrieve the assets on its own.

All audio files that are contained in the project folder (sub-folder audio)
can be moved in a custom folder in the Unity Assets folder.  As sounds are
played directly by Unity, the native plugin will not need access to them.

Scene Setup
-----------

To create a single animated character, a game object must be added to the scene
with the script HarmonyRenderer attached to it as a new component.
This script main purpose is to schedule rendering events for the camera.
The script HarmonyAnimation can be appended to the game object if the character
requires to be animated and the script HarmonyAudio, if the character produces
sound events.

The script HarmonyCamera must be attached to a camera game object in
order to render an animated character.

In order to ease setup process, two wizard scripts are added to the Unity
interface.  The "Harmony Object" wizard will create a game object that
renders directly to screen while the "Harmony Texture Object" wizard will
create a game object that renders to a textured mesh in Unity.

Harmony Renderer
----------------

The HarmonyRenderer component is the main component required for rendering a
game object.  The properties 'projectFolder' and 'clipNames' must be set after
creation of the game object.  It is possible to change the resolution of the
sprite sheet dynamically by changing the property 'sheetResolution'.

The color of the layer can be changed using property 'color'.  The color
rgba values are directly multiplied with the rendered texture rgba values.

Using the 'depth' property will allow reordering of the HarmonyRenderer game
objects between themselves.  An object with a depth index that is lower to
another will render before.  Two objects with the same depth index will have
an undefined rendering order.

The property 'discretization step' can be used to refine triangulation
of the character.  This will only be visible for characters with bone
deformation.

The 'renderTarget' property will change how animation is rendered. The default
mode 'eScreen' renders directly to screen through the camera.  The alternate
mode 'eRenderTexture' will render on a 'Renderer' object (e.g. Plane, Cube)
if one is attached to the current game object.  The resulting texture
resolution can be modified through component editor.

The 'syncCollider' property is used if a collider shape must be computed
for the game object.  To be used, a valid BoxCollider2D or PolygonCollider2D
component must be attached to the game object.  Generated collider can either
be a bounding box, a convex hull, or a concave polygon shape.  The mode
'eNever' will not update collider, the mode 'eOnce' will only update the
collider once and finally the mode 'eAlways' will update it at every Update()
call.

Metadata defined in Toon Boom Harmony is also available in the HarmonyRenderer.
Please refer to the interface HarmonyMeta for more information.

Harmony Camera
--------------

The HarmonyCamera component has three rendering modes:

PostCamera (default):
Will render Harmony game object after rendering rest of the scene. The plugin
will be able to use depth information during rendering but will not render
behind semi-transparent polygons.

PreCamera:
Will render Harmony game object before rendering rest of the scene.  The
plugin can render before semi-transparent object in the Unity scene properly,
but will not write depth information.

EndOfFrame:
Will render Harmony game object after all cameras and ui have finished
rendering.  The plugin can still use depth information for rendering.

Harmony Animation
-----------------

The HarmonyAnimation component manages current and scheduled animation for
its sibling component HarmonyRenderer.  It is responsible for providing a
clip name and a current frame for each rendering.

Harmony Audio
-------------

The HarmonyAudio component is used by the HarmonyAnimation when a sound event
is triggered.  It manages a pool of AudioSource objects that are used and
reused with different sound events.

The property 'audioFolder' may be used to specify where in the Assets folder
are the sound clips located.  If the sound clip cannot be found in the Unity
Assets, the script will look for it in the StreamingAssets next to the project
folder.  Doing so will require Unity to download it into its own database and
may impact performances.

Most of the properties of the AudioSource objects created by the HarmonyAudio
component can be modified by creating an empty AudioSource component in the
current game object.  The script will use the properties available in the
AudioSource to build new ones based on it.

Harmony Prop and Anchor
-----------------------

The HarmonyProp and HarmonyAnchor components are defined through
metadata in Toon Boom Harmony and can be used to dynamically assign prop
objects to anchor points in the character's hierarchy.  A prop object
has its own displayable frame that is distinct from the character
animation.  It's also possible to set metadata to the prop object that
can be accessed through the HarmonyMeta interface.  The HarmonyAnchor
component can also be used to extract the transformation of a named bone
inside the hierarchy.  The transformation will reflect the current clip
in use and the frame of rendering.

Cleanup
-------

All instances of a single character are shared in the native c++ plugin memory.
Once the last game object referencing a project has been destroyed, the c++
plugin will also remove references to this project from its memory.

WebGL Asset Preloading
----------------------

After Unity3D's WebGL build is complete, the developer is required to run the 
platform specific PrePackager executable, which will read the flagged Project 
Folders and make the necessary final steps. These PrePackager executables can 
be deleted after you made sure you've ran one of them and you also made sure 
that all the necessary Project Folders are flagged for Preload.

This allows the developer to set certain Project Folders to be ready to be used
when Unity WebGL is initialised. 

1) In Unity Assets, under StreamingAssets there is a file called: "PreLoadAssets.txt"ù
2) In this file, the developer can then set the project 
folders (multiple project folders can be set by separated by the ';'ù character).

For example, if the user requires the 
HarmonyResources/SpaceDuck and HarmonyResources/Wishling projects to be preloaded, 
the developer would have to set the contents of "PreLoadAssets.txt"ù as follows:
"HarmonyResources/SpaceDuck;HarmonyResources/Wishling"ù

3) Run the Prepackager tool located in StreamingAssets/RunExecutable/<PLATFORM>/Prepackager

Sample Uses
-----------

>> GameObject gameObject = new GameObject( "MyGameObject" );
>>
>> HarmonyRenderer renderer = gameObject.AddComponent<HarmonyRenderer>();
>> HarmonyAnimation animation = gameObject.AddComponent<HarmonyAnimation>();
>> HarmonyAudio audio = gameObject.AddComponent<HarmonyAudio>();
>>
>> //  Set up clip collection in renderer.
>> renderer.projectFolder = "MyProjectFolder";
>> renderer.clipNames = new string[] { "MyClip1", "MyClip2", "MyClip3" };
>>
>> //  Play sequence by clip name.  Since this clip is first scheduled,
>> //  will be played right away.
>> animation.PlayAnimation( 24.0f,      // frame rate
>>                          "MyClip1"   // clip name
>>                        );
>>
>> //  Play sequence by clip index.  Start at first frame and loop twice.
>> animation.PlayAnimation( 24.0f,      // frame rate
>>                          1,          // clip index
>>                          1.0f,       // start frame
>>                          2           // number of loops
>>                        );
>>
>> //  Loop sequence indefinitely until stopped manually.
>> animation.LoopAnimation( 24.0f,      // frame rate
>>                          2           // clip index
>>                        );

More examples are available in the Unity project provided with the game
engine package.

