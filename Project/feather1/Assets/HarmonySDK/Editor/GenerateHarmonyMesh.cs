using UnityEngine;

using System.Collections;
using System.Collections.Generic;
using System.IO;
using System;

using XML_ProjectLoader;

/*!
 *  @class GenerateMeta
 */
public class GenerateHarmonyMesh
{
  public static void CreateOrUpdateTextureObjectMesh(GameObject rootObject, bool regenerateMesh = false)
  {
    HarmonyRenderer renderer = rootObject.GetComponent<HarmonyRenderer>();
    if (renderer == null)
      return;

    //if (renderer.renderTarget != HarmonyRenderer.RenderTarget.eRenderTexture)
    //  return;

    MeshFilter meshFilter = rootObject.GetComponent<MeshFilter>();
    if (meshFilter == null)
      meshFilter = rootObject.AddComponent<MeshFilter>();

    MeshRenderer meshRenderer = rootObject.GetComponent<MeshRenderer>();
    if (meshRenderer == null)
      meshRenderer = rootObject.AddComponent<MeshRenderer>();

    Bounds box = new Bounds(Vector3.zero, Vector3.zero);
    if ( (meshFilter.sharedMesh == null) || regenerateMesh )
    {
      box = renderer.CalculateCumulativeBoundingBox();
    }
    else
    {
      box = meshFilter.sharedMesh.bounds;
    }

    if (box.size.sqrMagnitude > 0.0f)
    {
      float textureScale = renderer.renderTextureScale;

      float textureWidth = Mathf.NextPowerOfTwo((int)(box.size.x * textureScale));
      float textureHeight = Mathf.NextPowerOfTwo((int)(box.size.y * textureScale));
      float textureX = box.min.x * textureScale;
      float textureY = box.min.y * textureScale;

      float x = box.min.x;
      float y = box.min.y;
      float width = box.size.x;
      float height = box.size.y;

      float scaledWidth = width * textureScale;
      float scaledHeight = height * textureScale;

      float u0 = 0.0f;
      float u1 = scaledWidth / textureWidth;
      float v0 = 1.0f - (scaledHeight / textureHeight);
      float v1 = 1.0f;

      //Message.Log( "uvs: (" + u0 + "," + v0 + ") (" + u1 + "," + v1 + ")" );

      renderer.renderTextureViewport = new Rect(textureX, textureY, textureWidth, textureHeight);

      //  Create basic quad to map texture unto.
      if ( (meshFilter.sharedMesh == null) || regenerateMesh )
      {
        Mesh mesh = new Mesh();
        mesh.name = rootObject.name + "_mesh";

        Vector3[] vertices = new Vector3[4]
        {
          new Vector3( x        , y         , 0),
          new Vector3( x + width, y         , 0),
          new Vector3( x + width, y + height, 0),
          new Vector3( x        , y + height, 0)
        };

        Vector3[] normals = new Vector3[4]
        {
          new Vector3( 0, 0, -1.0f ),
          new Vector3( 0, 0, -1.0f ),
          new Vector3( 0, 0, -1.0f ),
          new Vector3( 0, 0, -1.0f )
        };

        Vector2[] uvs = new Vector2[4]
        {
          new Vector2( u0, v1 ),
          new Vector2( u1, v1 ),
          new Vector2( u1, v0 ),
          new Vector2( u0, v0 )
        };

        int[] faces = new int[6]
        {
          2, 1, 0,
          3, 2, 0
        };

        mesh.vertices = vertices;
        mesh.normals = normals;
        mesh.uv = uvs;
        mesh.triangles = faces;

        UnityEngine.Object.DestroyImmediate(meshFilter.sharedMesh);
        meshFilter.mesh = mesh;
      }
      else
      {
        Mesh meshCopy = Mesh.Instantiate(meshFilter.sharedMesh) as Mesh;
        meshCopy.name = rootObject.name + "_mesh";

        Vector2[] uvs = new Vector2[4]
        {
          new Vector2( u0, v1 ),
          new Vector2( u1, v1 ),
          new Vector2( u1, v0 ),
          new Vector2( u0, v0 )
        };

        meshCopy.uv = uvs;

        UnityEngine.Object.DestroyImmediate(meshFilter.sharedMesh);
        meshFilter.mesh = meshCopy;
      }

      //  Create material for new mesh.
      if (meshRenderer.sharedMaterial == null)
      {
        Shader shader = Shader.Find("Harmony/UnlitTransparentPremult");
        if (shader != null)
        {
          Material mat = new Material(shader);
          mat.name = rootObject.name + "_mat";

          UnityEngine.Object.DestroyImmediate(meshRenderer.sharedMaterial);
          meshRenderer.material = mat;
        }
      }
    }
  }

  public static void ClearTextureObjectMesh(GameObject rootObject)
  {
    MeshFilter meshFilter = rootObject.GetComponent<MeshFilter>();
    if (meshFilter != null)
    {
      UnityEngine.Object.DestroyImmediate(meshFilter.sharedMesh);
      UnityEngine.Object.DestroyImmediate(meshFilter);
    }

    MeshRenderer meshRenderer = rootObject.GetComponent<MeshRenderer>();
    if (meshRenderer != null)
    {
      UnityEngine.Object.DestroyImmediate(meshRenderer.sharedMaterial.mainTexture);
      UnityEngine.Object.DestroyImmediate(meshRenderer.sharedMaterial);
      UnityEngine.Object.DestroyImmediate(meshRenderer);
    }
  }
}
