using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PrefabSample : MonoBehaviour {
    public GameObject prefab;
    public Transform featherParent;
	// Use this for initialization
	void Start () {
        for (int i = 0; i < 10; i++)
        {

		 Instantiate(prefab,Vector3.zero + i*10 * Vector3.up,transform.rotation,featherParent);
        }
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}
