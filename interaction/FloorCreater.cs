using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FloorCreater : MonoBehaviour
{
    public GameObject funit;
    public Object[,] f = new Object[19,19];
    public int[,] color = new int[19,19];
    void Start()
    {
        //var *f = new funit[19][19];
        for(int i = 0; i < 19; i++)
        {
            for(int j = 0; j < 19; j++)
            {
                f[i,j] = Instantiate(funit, new Vector3((i-9)*1.0f, 0f, (j-9)*1.0f), Quaternion.identity);
                Debug.Log(f[i,j].ToString());
            }
        }
        for(int i = 0; i < 19; i++)
        {
            for(int j = 0; j < 19; j++)
            {
                color[i,j] = 0;
            }
        }
        color[9,9] = 1;
    }

    // Update is called once per frame
    void Update()
    {
        for(int i = 0; i < 19; i++)
        {
            for(int j = 0; j < 19; j++)
            {
                GameObject tmp = f[i, j] as GameObject;
                Debug.Log(tmp);
                if(color[i,j] == 1) tmp.GetComponent<FloorUnit>().unitColor();
            }
        }
    }    
}