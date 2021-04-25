using System;
using System.Collections;
using System.Collections.Generic;
using Communication.Proto;
using UnityEngine;
using Communication.CSharpClient;
using System.Net;

public class HeroManager : MonoBehaviour
{
    // Start is called before the first frame update
    public string color;
    public GameObject[] hero = null;

    void Start(){
        
    }

    // Update is called once per frame
    void Update(){
        
    }

    public GameObject HeroMap(GameObjInfo obj)
    {
        GameObject retVal;
        switch(obj.JobType)
        {
            case JobType.Job0:
                retVal = hero[0]; // 平凡的角色
                break;
            case JobType.Job1:
                retVal = hero[1]; //投掷，hls
                break;
            case JobType.Job2:
                retVal = hero[2]; //lyy，跑得慢，子弹慢，伤害性坦克，伤害高
                break;
            case JobType.Job3:
                retVal = hero[3];
                break;
            case JobType.Job4:
                retVal = hero[4];
                break;
            case JobType.Job5:
                retVal = hero[5];
                break;
            case JobType.Job6:
                retVal = hero[6];
                break;
            default: 
                retVal = null;
                break;
        }
        return retVal;
    }

    public void Coloring(int xPos, int yPos){
        //FloorManager.SetColor(xPos, yPos, color);
    }

    
}
