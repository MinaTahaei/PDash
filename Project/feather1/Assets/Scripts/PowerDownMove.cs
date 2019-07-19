using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PowerDownMove : MonoBehaviour {
    public GameObject GameObject;
    public GameObject Prefab;
    //List<GameObject> prefabList = new List<GameObject>();
    //public GameObject Prefab1;
    //public GameObject Prefab2;
    //public GameObject Prefab3;
    public float DownForce = -0.05f;
    private SpriteRenderer mySpriteRenderer;

    void DestroyGameObject()
    {
        Destroy(GameObject);
    }

    // This function is called just one time by Unity the moment the component loads
    private void Awake()
    {
        // get a reference to the SpriteRenderer component on this gameObject
        mySpriteRenderer = GetComponent<SpriteRenderer>();
    }

    // Use this for initialization
    void Start () {
        //prefabList.Add(Prefab);
        //prefabList.Add(Prefab1);
        //prefabList.Add(Prefab2);
        //prefabList.Add(Prefab3);

    }
	
	// Update is called once per frame
	void Update ()
    {
        GameObject.transform.Translate(new Vector3(0, DownForce));
        if (GameObject.transform.position.y < -10)
        {
            DestroyGameObject();
        }
    }
}
