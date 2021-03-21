using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Communication.Proto;
using Google.Protobuf;
using System.IO;
using Communication.CSharpClient;
using System.Net;

public class BulletScript : MonoBehaviour
{
    private Vector3 position;
    private float moveSpeed = 5;
    private int teamId;
    // Start is called before the first frame update
    void Start()
    {
        position = new Vector3();
    }

    // Update is called once per frame
    void Update()
    {
        Move();
    }

    void Move()
    {
            Vector3 pos = transform.position;
            pos.x = Mathf.Lerp(pos.x, position.x, moveSpeed * Time.deltaTime);
            pos.y = Mathf.Lerp(pos.y, position.y, moveSpeed * Time.deltaTime);
            pos.z = Mathf.Lerp(pos.z, position.z, moveSpeed * Time.deltaTime);
            transform.position = pos;
    }

    public void Renew(GameObjInfo obj)
    {
        position.x = obj.X / 500;
        position.y = 1f;
        position.z = obj.Y / 500;
        teamId = (int)obj.TeamID;
        moveSpeed = obj.MoveSpeed;
    }
}
