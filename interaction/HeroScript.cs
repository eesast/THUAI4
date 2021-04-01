using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Communication.Proto;
using Google.Protobuf;
using Communication.CSharpClient;
using System.Net;

public class HeroScript : MonoBehaviour
{
    public int moveSpeed;
    public float turnSpeed = 20f;
    private float moveThreshold = 5;
    private Vector3 direction;
    private bool isMoving;
    private Animator animator;
    private Vector3 position;
    private bool isDying;

    void Start()
    {
        animator = GetComponentInParent<Animator>();
        position = new Vector3();
        position.y = 0f;
        direction = new Vector3();
        direction.y = 0f;
        direction.z = 0f;
        animator.SetBool("IsWalking", isMoving);
    }

    void Update()
    {
        if(!isDying)
        {
            Move();
            Rotate();
            SetAnimator();
        }
        else
        {
            Vector3 diePos = transform.position;
            diePos.y = 50f;
            transform.position = diePos;
        }
    }

    void Move()
    {
        if ((position - transform.position).magnitude > moveThreshold)
        {
            transform.position = position;
        }
        else
        {
            Vector3 pos = transform.position;
            pos.x = Mathf.Lerp(pos.x, position.x, moveSpeed * Time.deltaTime);
            pos.y = Mathf.Lerp(pos.y, position.y, moveSpeed * Time.deltaTime);
            pos.z = Mathf.Lerp(pos.z, position.z, moveSpeed * Time.deltaTime);
            transform.position = pos;
        }
    }

    void Rotate()
    {
        transform.rotation = Quaternion.Euler(Vector3.up * direction.x);
    }

    void SetAnimator()
    {
        animator.SetBool("IsWalking", isMoving);
    }

    public void Renew(GameObjInfo obj)
    {
        position.x = (float)obj.X/500;
        position.z = (float)obj.Y/500;
        direction.x = (float)(90 - obj.FacingDirection * 180 / 3.14);
        Debug.Log(obj.FacingDirection.ToString());
        isMoving = obj.IsMoving;
        moveSpeed = obj.MoveSpeed;
        isDying = obj.IsDying;
    }
}