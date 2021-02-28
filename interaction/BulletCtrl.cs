using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BulletCtrl : MonoBehaviour
{
    // Start is called before the first frame update

    public String destName = "Plane";
    public float speed;
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        transform.Translate(speed*Time.deltaTime, 0, 0);
    }

    private void OnCollisionStay(Collision other)
    {
        Debug.Log(other.gameObject.name);
        
        if(other.gameObject.name == destName)
            Destroy(gameObject);
    }
}
