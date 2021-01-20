using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WalkHero : Heros
{
    // Start is called before the first frame update
    
    void Start(){
        color = "green";
    }

    // Update is called once per frame
    void Update(){
        
    }

    private void OnCollisionEnter(Collision other){
        Debug.Log(other.collider.name);
        GameObject tmp = other.gameObject;
        Coloring(tmp.GetComponent<FloorUnit>().xPos, tmp.GetComponent<FloorUnit>().yPos);
    }
}
