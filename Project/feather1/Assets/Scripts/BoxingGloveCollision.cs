using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BoxingGloveCollision : MonoBehaviour {
    public GameObject collisionDetector;
    public bool LeftSide;
    private bool h1 = true;
    private bool h2 = true;
	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        if (LeftSide == true)
        {
            if (collisionDetector.transform.position.x < -0.2 && h1 == true)
            {
                collisionDetector.transform.Translate(new Vector3(0.64f, 0));
            }
            if (collisionDetector.transform.position.x > -0.1)
                h1 = false;
            if (collisionDetector.transform.position.x > -3.5f && h1 == false)
            {
                collisionDetector.transform.Translate(new Vector3(-0.04f, 0));
                
            }
            if (collisionDetector.transform.position.x < -3.4)
                h1 = true;
        }
        else if (LeftSide == false)
        {
            if (collisionDetector.transform.position.x > 0 && h2 == true)
            {
                collisionDetector.transform.Translate(new Vector3(0.64f, 0));
            }
            if (collisionDetector.transform.position.x < 0.2)
                h2 = false;
            if (collisionDetector.transform.position.x < 3.7f && h2 == false)
            {
                collisionDetector.transform.Translate(new Vector3(-0.04f, 0));

            }
            if (collisionDetector.transform.position.x > 3.2)
                h2 = true;
        }
    }
}
