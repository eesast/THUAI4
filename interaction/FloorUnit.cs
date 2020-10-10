using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FloorUnit : MonoBehaviour
{
    public int colorStatus = 0;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
    public void unitColor()
    {
        this.GetComponent<MeshRenderer>().material.color = Color.blue;
    } 
}