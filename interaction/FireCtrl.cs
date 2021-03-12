using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FireCtrl : MonoBehaviour
{
    // Start is called before the first frame update
    
    public int lineLength = 1000;
    public GameObject obj;

    
    
    void Start()
    {
        
    }

// Update is called once per frame
    void Update()
    {

        if (Input.GetMouseButtonUp(0))
        {
            Instantiate(obj, transform.position, transform.rotation); //.GetComponent<Rigidbody>().velocity = speed * transform.forward;
        }
    }
}
