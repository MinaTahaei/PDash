//using UnityEngine;
//using System.Collections;
//using System.Collections.Generic;
//
//public class BackgroundScroll : MonoBehaviour
//{
//
//    public float scrollSpeed;
//    private Vector2 savedOffset;
//
//    void Start()
//    {
//        savedOffset = renderer.sharedMaterial.GetTextureOffset("_MainTex");
//    }
//
//    void Update()
//    {
//        float y = Mathf.Repeat(Time.time * scrollSpeed, 1);
//        Vector2 offset = new Vector2(savedOffset.x, y);
//        renderer.sharedMaterial.SetTextureOffset("_MainTex", offset);
//    }
//
//    void OnDisable()
//    {
//        renderer.sharedMaterial.SetTextureOffset("_MainTex", savedOffset);
//    }
//}
using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class BackgroundScroll : MonoBehaviour
{

    public float scrollSpeed = 0.5f;
    private Renderer renderer;

    void Start()
    {
        renderer = GetComponent<Renderer>();
    }

    void Update()
    {
        Vector2 offset = new Vector2(0, Time.time * scrollSpeed);
        renderer.material.mainTextureOffset = offset;
    }
 
}