using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class probability : MonoBehaviour {
    //private List<GameObject> prefabList = new List<GameObject>();
    //public GameObject Prefab1;
    //public GameObject Prefab2;
    //public GameObject Prefab3;
    //public GameObject[] prefabList = new GameObject[3];
    private int currentIndex = 0;
    public GameObject gameObject;

    //public void NewRandomObject()
    //{
    //    int newIndex = Random.Range(0, prefabList.Length);
    //    // Deactivate old gameobject
    //    prefabList[currentIndex].SetActive(false);
    //    // Activate new gameobject
    //    currentIndex = newIndex;
    //    prefabList[currentIndex].SetActive(true);
    //}

    // Use this for initialization
    void Start () {
        //Prefab1.SetActive(false);
        //Prefab2.SetActive(false);
        //Prefab3.SetActive(false);
        //prefabList.Add(Prefab1);
        //prefabList.Add(Prefab2);
        //prefabList.Add(Prefab3);
        //NewRandomObject();
        gameObject.SetActive(true);
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}
