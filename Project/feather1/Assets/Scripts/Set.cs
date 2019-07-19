using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Set : MonoBehaviour {
    public GameObject obj;
    public GameObject Player;
    bool b = false;
    // Use this for initialization
    void OnTriggerEnter(Collider Object)
    {
        Debug.Log(Object.name);
        if (Object.gameObject.tag == "Lose")
        {
            b = true;
        }
    }

        void Start () {
        obj.SetActive(false);
	}
	
	// Update is called once per frame
	void Update () {
		if(b)
        {
            obj.SetActive(true);
        }
	}
}
