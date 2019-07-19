using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Movement : MonoBehaviour {

    
    public GameObject TriggerLeft;
    public GameObject TriggerRight;
    public GameObject GameObject;
    public Rigidbody2D rigidbody;
    public float SidewayForce = 50f;
    public float ForwardForce = 100f;

    public bool DoReact { get; private set; }

    private void OnTriggerLeftEnter(Collider Object)
    {
       if(Object.tag == "GameObject")
       {
            DoReact = true;
       }

    }
    private void OnTriggerRightEnter(Collider Object)
    {
        if(Object.tag == "GameObject")
        {
            DoReact = true;
        }

    }

    private void OnTriggerLeftExit(Collider Object)
    {
        if (Object.tag == "GameObject")
        {
            DoReact = false;
        }

    }

    private void OnTriggerRightExit(Collider Object)
    {
        if (Object.tag == "GameObject")
        {
            DoReact = false;
        }

    }

    // Use this for initialization
    void Start ()
    {
        Debug.Log("Hi!");
    }
    
    // Update is called once per frame
    void FixedUpdate ()
    {
        //rigidbody.AddForce(0, ForwardForce * Time.deltaTime);

        if (Input.GetKey(KeyCode.LeftArrow) && DoReact)
        {
            rigidbody.AddForce(Vector2.left*SidewayForce ,ForceMode2D.Impulse);
            
        }

        else if (Input.GetKey(KeyCode.RightArrow) && DoReact)
        {
            rigidbody.AddForce(Vector2.right * SidewayForce, ForceMode2D.Impulse);

        }
	}
}
