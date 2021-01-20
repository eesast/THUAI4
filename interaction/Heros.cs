using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Heros : MonoBehaviour
{
    // Start is called before the first frame update
    public string color;
    
    void Start(){
        
    }

    // Update is called once per frame
    void Update(){
        
    }

    public void Coloring(int xPos, int yPos){
        FloorManager.SetColor(xPos, yPos, color);
    }

    
}
