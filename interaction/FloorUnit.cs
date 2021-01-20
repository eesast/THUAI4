using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FloorUnit : MonoBehaviour
{
    //public int colorStatus = 0;
    public int xPos, yPos;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
    public void unitColor(Material material)
    {
        this.GetComponent<MeshRenderer>().material = material;
       // Debug.Log(this.GetComponent<MeshRenderer>().material);
       // this.colorStatus = 1;
    } 
}
