using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Communication.Proto;
using Google.Protobuf;
using System.IO;
using Communication.CSharpClient;
using System.Net;

public class BulletManager : MonoBehaviour
{
    public GameObject[] bullet = null;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public GameObject BulletMap(GameObjInfo obj)
    {
        GameObject retVal;
        switch(obj.BulletType)
        {
            case BulletType.Bullet0:
                retVal = bullet[0];
                break;
            case BulletType.Bullet1:
                retVal = bullet[1];
                break;
            case BulletType.Bullet2:
                retVal = bullet[2];
                break;
            case BulletType.Bullet3:
                retVal = bullet[3];
                break;
            case BulletType.Bullet4:
                retVal = bullet[4];
                break;
            case BulletType.Bullet5:
                retVal = bullet[5];
                break;
            case BulletType.Bullet6:
                retVal = bullet[6];
                break;
            default: retVal = null;
                break;
        }
        return retVal;
    }
}
