using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Movement : MonoBehaviour {

    public GameObject GameObject;
    public GameObject CollisionDetectorBottom;
    public GameObject CollisionDetectorTop;
    public GameObject AnimatedObject;
    public GameObject Bird1;
    public GameObject Bird2;
    public GameObject Bird3;
    public Rigidbody RigidBody;
    public float SidewayForce = 0.06f;
    public float UpForce = 0.07f;
    private float TimeLeft = 11f;
    private bool AnimationSwitch = false;
    private bool DoReactRight;
    private bool DoReactLeft;
    private bool Lost = false;
    private bool SpeedUp = false;
    private bool PowerUp = false;

    void OnTriggerEnter(Collider Object)
    {
        Debug.Log(Object.name);
       if(Object.gameObject.tag == "TriggerLeft")
       {
            DoReactLeft = true;
       }
       if (Object.gameObject.tag == "TriggerRight")
       {
            DoReactRight = true;
       }
       if (Object.gameObject.tag == "Saw" || Object.gameObject.tag == "Box" || Object.gameObject.tag == "scissors" || Object.gameObject.tag == "NearWater")
       {
            Lost = true;
       }
       if (Object.gameObject.tag == "Water")
       {
            SpeedUp = true;
       }
       if (Object.gameObject.tag == "Bird")
       {
           PowerUp = true;
       }
    }

    void OnTriggerExit(Collider Object)
    {
        if (Object.gameObject.tag == "TriggerLeft")
        {
            DoReactLeft = false;
        }
        if (Object.gameObject.tag == "TriggerRight")
        {
            DoReactRight = false;
        }
    }

    // Use this for initialization
    void Start ()
    {
        GameObject.transform.position = new Vector3(0, 5, -0.1f);
        RigidBody = GetComponent<Rigidbody>();
        Debug.Log("Hi!");
    }

    void DestroyGameObject()
    {
        Destroy(GameObject);
    }

    void BirdMovement(GameObject Bird1)
    {
        Bird1.transform.position = new Vector3(Bird1.transform.position.x, Bird1.transform.localPosition.y + 0.02f, transform.position.z);
        GameObject.transform.position = (new Vector3(-100, GameObject.transform.position.x));
        TimeLeft -= Time.deltaTime;
        if (TimeLeft < 1)
        {
            if (GameObject.tag == "Saw" || gameObject.tag == "Box" || gameObject.tag == "scissors" || gameObject.tag == "NearWater")
                TimeLeft++;
        }
        if (TimeLeft < 0)
        {
            Bird1.transform.position = new Vector3(Bird1.transform.position.x, Bird1.transform.localPosition.y + 0.1f, transform.position.z);
            GameObject.transform.position = (new Vector3(0, 4.1f));
        }
        if (TimeLeft < -1)
            Destroy(Bird1);
    }

    // Update is called once per frame
    void Update ()
    {
        if(GameObject.transform.position.x >= 5.2)
            GameObject.transform.position = new Vector3(5.2f, GameObject.transform.position.y);

        if (GameObject.transform.position.x <= -5.2)
            GameObject.transform.position = new Vector3(-5.2f, GameObject.transform.position.y);
        //rigidbody.AddForce(0, ForwardForce * Time.deltaTime);

        //if (CollisionDetectorBottom.transform.position.x <= -5.2 && (CollisionDetectorBottom.transform.position.y > 3.7 && CollisionDetectorBottom.transform.position.y < 6))
        //GameObject.transform.position =(new Vector3(GameObject.transform.position.x, -20));

        if (Lost)
        {
            //GameObject.transform.position = new Vector3(20, GameObject.transform.position.y);
            DestroyGameObject();
        }

        if (Input.GetKey(KeyCode.LeftArrow))
        {
            if (DoReactLeft)
                GameObject.transform.Translate(new Vector3(SidewayForce, UpForce));
        }

        if (Input.GetKey(KeyCode.RightArrow))
        {
            if(DoReactRight)
                GameObject.transform.Translate(new Vector3(SidewayForce * (-1), UpForce));
        }

        if(SpeedUp)
        {
            GameObject.transform.Translate(new Vector3(0, (-2) * UpForce));
        }

        if (PowerUp)
        {

            BirdMovement(Bird1);
            BirdMovement(Bird2);
            BirdMovement(Bird3);
        }


    }
}
