using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Communication.Proto;
using Google.Protobuf;
using System.IO;
using Communication.CSharpClient;
using System.Net;

public class PropScript : MonoBehaviour
{
    private Vector3 position;
    private float maxMoveSpeed = 5;
    public bool isPicked = false;
    private int teamId;

    void Start()
    {
        position = new Vector3();
    }

    void Update()
    {
        transform.position = position;
    }
    void LateUpdate()
    {
        MyDestroy();
    }

    public void Renew(GameObjInfo obj)
    {
        position.x = (float)(obj.X/1000);
        position.y = 0f;
        position.z = (float)(obj.Y/1000);
        teamId = (int)obj.TeamID;
    }
    void MyDestroy()
    {
        if (isPicked)
        {
            Destroy(gameObject);
        }
    }
}