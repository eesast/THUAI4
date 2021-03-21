using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf;
using Communication.Proto;
using System.IO;
using System.Net;
using Communication.CSharpClient;
using System.Collections.Concurrent;
using Es.InkPainter;
using System.Threading;
using System;

public class AIManager : MonoBehaviour
{
    public GameObject _powderBag;
    
    private bool isGameStarted;
    private ushort port = 7777;
    public int teamId = -1;
    public int playerId = -1;
    public int job = -1;
    private ConcurrentDictionary<long, bool> isCharactersExisted;
    private ConcurrentDictionary<long, bool> isPropsExisted;
    private ConcurrentDictionary<long, bool> isBulletsExisted;
    private ConcurrentDictionary<long, bool> laidList;
    private ConcurrentDictionary<long, PropScript> props; 
    private ConcurrentDictionary<long, HeroScript> heros; 
    private ConcurrentDictionary<long, BulletScript> bullets;
    private ConcurrentQueue<KeyValuePair<long, GameObjInfo>> TaskQueue;
    private ConcurrentQueue<GameObjInfo> LaidQueue;
    private bool isNewMessageToServer;
    private CSharpClient csharpClient;
    private MessageToServer messageToServer;
    public static long receiveCount = 0;
    public static int[,] mapColor = new int [50, 50];
    private static int[,] cellColor = new int [50, 50];
    private bool isReady;
    private double myAngle;
    private PropManager propManager;
    private HeroManager heroManager;
    private BulletManager bulletManager;
    public Color[] brushColors = null;
    public Texture brush;

    void Start()
    {
        Debug.Log("开始初始化参数");
        propManager = this.GetComponent<PropManager>();
        heroManager = this.GetComponent<HeroManager>();
        bulletManager = this.GetComponent<BulletManager>();
        // TO DO:初始化通信
        messageToServer = new MessageToServer();
        csharpClient = new CSharpClient();
        csharpClient.OnReceive += delegate ()
        {
            Debug.Log("Message Received.");
            //Debug.Log(csharpClient.ToString());
            //IMsg iMsg=csharpClient.Take();
            //Debug.Log(iMsg.ToString());
            //csharpClient.TryTake(out IMsg iMsg);
            //Debug.Log(iMsg.ToString());
            if (csharpClient.TryTake(out IMsg iMsg))
            {
                switch (iMsg.PacketType)
                {
                    case PacketType.MessageToClient:
                        {
                            MessageToClient msg = iMsg.Content as MessageToClient;
                            Debug.Log("msg received");
                            switch (msg.MessageType)
                            {
                                case MessageType.StartGame:
                                    GameStart(msg); break;
                                case MessageType.Gaming:
                                    Refresh(msg); break;
                                case MessageType.EndGame:
                                    GameOver(); break;
                                default: break;
                            }
                            break;
                        }
                    case PacketType.MessageToOneClient:
                        {
                            MessageToOneClient msg = iMsg.Content as MessageToOneClient;
                            Debug.Log("msg received");
                            switch (msg.MessageType)
                            {
                                case MessageType.ValidPlayer:
                                    Debug.Log("Info Valid");
                                    break;
                                case MessageType.InvalidPlayer:
                                    Debug.Log("Info Invalid");
                                    break;
                                case MessageType.Send:
                                    Debug.Log("Info Send");
                                    break;
                                default: break;
                            }
                            break;
                        }
                    default: break;
                }
            }
            else
            {
                Debug.Log("Failed to pop a message");
            }
        };
        if (csharpClient.Connect("127.0.0.1", port))
        {
            Debug.Log("成功连接Agent.");
        }
        else
        {
            Debug.Log("连接Agent失败.");
            Application.Quit();
        }
        teamId = 0;
        playerId = 0;
        messageToServer.MessageType = MessageType.AddPlayer;
        messageToServer.TeamID = teamId;
        messageToServer.PlayerID = playerId;
        messageToServer.JobType = JobType.Job0;

        csharpClient.SendMessage(messageToServer);
        //WebClient 
        heros = new ConcurrentDictionary<long, HeroScript>();
        props = new ConcurrentDictionary<long, PropScript>();
        bullets = new ConcurrentDictionary<long, BulletScript>();
        isCharactersExisted = new ConcurrentDictionary<long, bool>();
        isPropsExisted = new ConcurrentDictionary<long, bool>();
        laidList = new ConcurrentDictionary<long, bool>();
        TaskQueue = new ConcurrentQueue<KeyValuePair<long, GameObjInfo>>();
        LaidQueue = new ConcurrentQueue<GameObjInfo>();
        isReady = false;
        myAngle = 0;
        isNewMessageToServer = false;
        for (int i = 0; i < 50; i++)
        {
            for(int j = 0; j < 50; j++)
            {
                mapColor[i, j] = 0;
            }
        }
        Debug.Log("参数初始化完成");
    }

    void Update()
    {
        for (int i = 0; i < 50; i++)
        {
            for (int j = 0; j < 50; j++)
            {
                if(mapColor[i, j] != cellColor[i, j])
                {
                    mapColor[i, j] = cellColor[i, j];
                    if(mapColor[i, j] == 1)
                    {
                        GetComponent<SomewherePainter>().brush = new Brush(brush, 0.02f, brushColors[0]);
                    }
                    else
                    {
                        GetComponent<SomewherePainter>().brush = new Brush(brush, 0.02f, brushColors[1]);
                    }
                    Vector3 pos = new Vector3((float)(2 * i), 0f, (float)(2 * i));
                    GetComponent<SomewherePainter>().PointPaint(pos);
                }
            }
        }
        if(!TaskQueue.IsEmpty)
        {
            FlushTaskQueue();
        }
        if(!LaidQueue.IsEmpty)
        {
            FlushLaidQueue();
        }
    }

    void FixedUpdate()
    {
        if(isGameStarted)
        {
            GenerateMessage();
            if(isNewMessageToServer)
            {
                isNewMessageToServer = false;
                messageToServer.TeamID = teamId;
                messageToServer.PlayerID = playerId;
                csharpClient.SendMessage(messageToServer);
                Debug.Log("msg send");
            }
        }
    }

    private void GenerateMessage()
    {
        CheckKeyInput(out double angle, out bool isMoving, out bool isPPressed, out bool isLPressed, out bool attack);
        if (attack)
        {
            isNewMessageToServer = true;
            messageToServer.MessageType = MessageType.Attack;
            messageToServer.TimeInMilliseconds = 100;
            messageToServer.Angle = angle;
            Debug.Log("Attack!!!!!!!");
            attack = false;
        }
        else if (isMoving)
        {
            isNewMessageToServer = true;
            messageToServer.MessageType = MessageType.Move;
            messageToServer.TimeInMilliseconds = 400;
            messageToServer.Angle = angle;
            isMoving = false;
        }
        else if(isPPressed)
        {
            isNewMessageToServer = true;
            messageToServer.MessageType = MessageType.Pick;
            isPPressed = false;
            //messageToServer.PropType
        }
        else if(isLPressed)
        {
            isNewMessageToServer = true;
            messageToServer.MessageType = MessageType.Use;
            isLPressed = false;
        }
    }

    private void CheckKeyInput(out double angle, out bool isMoving, out bool isPPressed, out bool isLPressed, out bool attack)
    {
        angle = 0;
        isMoving = true;
        if (Input.GetKey(KeyCode.W))
        {
            angle = 3.1415926;
            Debug.Log("moving");
        }
        else if(Input.GetKey(KeyCode.S))
        {
            angle = 0;
            Debug.Log("moving");
        }
        else if(Input.GetKey(KeyCode.A))
        {
            angle = -0.5 * 3.1415926;
        }
        else if (Input.GetKey(KeyCode.D))
        {
            angle = 0.5 * 3.1415926;
        }
        else isMoving = false;
        isPPressed = Input.GetKey(KeyCode.P);
        isLPressed = Input.GetKey(KeyCode.L);
        attack = Input.GetKey(KeyCode.E);
    }

    private void GameOver()
    {
        //TO DO:显示结束画面，同时显示两方的得分
        UnityEngine.Debug.Log("退出游戏");
    }

    private void Refresh(MessageToClient message)
    {
        //根据地图信息更新地图
        //染色
        for (int i = 0; i < 50; i++)
        {
            for (int j = 0; j < 50; j++)
            {
                cellColor[i, j] = (int)message.CellColors[j].RowColors[i];
            }
        }
        foreach (var obj in message.GameObjs)
        {
            switch(obj.GameObjType)
            {
                case GameObjType.Character:
                {
                    if(heros.ContainsKey(obj.Guid))
                    {
                        heros[obj.Guid].Renew(obj); //存在就更新数据
                    }
                    else
                    {
                        if(!isCharactersExisted.ContainsKey(obj.Guid))
                        {
                            isCharactersExisted.TryAdd(obj.Guid, true);
                            TaskQueue.Enqueue(new KeyValuePair<long, GameObjInfo>(obj.Guid, obj)); //不存在就标记并创建，下面同理
                        }
                    }
                    break;
                }
                case GameObjType.Prop:
                {
                    if (props.ContainsKey(obj.Guid))
                    {
                        props[obj.Guid].Renew(obj);
                        if(obj.PropType == PropType.JinKeLa) //粉包（地雷）特殊处理
                        {
                            if(obj.IsLaid && !laidList[obj.Guid]) //地雷被放置
                            {
                                LaidQueue.Enqueue(obj);
                                laidList[obj.Guid] = true;
                            }
                        }
                    }
                    else
                    {
                        if(!isPropsExisted.ContainsKey(obj.Guid))
                        {
                            isPropsExisted.TryAdd(obj.Guid, true);
                            if (obj.PropType == PropType.JinKeLa) laidList.TryAdd(obj.Guid, false); //粉包（地雷）特殊处理
                            TaskQueue.Enqueue(new KeyValuePair<long, GameObjInfo>(obj.Guid, obj));
                        }
                    }
                    break;
                }
                case GameObjType.Bullet:
                {
                    if(bullets.ContainsKey(obj.Guid))
                    {
                        bullets[obj.Guid].Renew(obj); 
                    }
                    else
                    {
                        if(!isBulletsExisted.ContainsKey(obj.Guid))
                        {
                            isBulletsExisted.TryAdd(obj.Guid, true);
                            TaskQueue.Enqueue(new KeyValuePair<long, GameObjInfo>(obj.Guid, obj)); 
                        }
                    }
                    break;
                }
                default: break;
            }
        }
    }

    private void GameStart(MessageToClient message)
    {
        //根据agent发来的消息初始化各个角色
        foreach (var obj in message.GameObjs)
        {
            if(obj.GameObjType == GameObjType.Character)
            {
                if (heros.ContainsKey(obj.Guid))
                {
                    heros[obj.Guid].Renew(obj); //存在就更新数据
                }
                else
                {
                    if (!isCharactersExisted.ContainsKey(obj.Guid))
                    {
                        isCharactersExisted.TryAdd(obj.Guid, true);
                        TaskQueue.Enqueue(new KeyValuePair<long, GameObjInfo>(obj.Guid, obj)); //不存在就标记并创建，下面同理
                    }
                }
            }
        }
        isGameStarted = true;
    }

    private void FlushTaskQueue()
    {
        while(!TaskQueue.IsEmpty)
        {
            Debug.Log("FlushTask!!!!");
            bool isGood = TaskQueue.TryDequeue(out KeyValuePair<long, GameObjInfo> res);
            if(isGood)
            {
                MyInstantiate(res.Key, res.Value);
            }
        }
    }

    private void MyInstantiate(long objGuid, GameObjInfo objInfo)
    {
        switch(objInfo.GameObjType)
        {
            case GameObjType.Prop:
                var i = Instantiate(propManager.PropMap(objInfo));
                props.TryAdd(objGuid, i.GetComponent<PropScript>());
                props[objGuid].Renew(objInfo);
                Debug.Log("生成了道具");
                break;
            case GameObjType.Character:
                var j = Instantiate(heroManager.HeroMap(objInfo));
                heros.TryAdd(objGuid, j.GetComponent<HeroScript>());
                heros[objGuid].Renew(objInfo);
                Debug.Log("生成了角色");
                break;
            case GameObjType.Bullet:
                var z = Instantiate(bulletManager.BulletMap(objInfo));
                bullets.TryAdd(objGuid, z.GetComponent<BulletScript>());
                bullets[objGuid].Renew(objInfo);
                Debug.Log("生成了子弹");
                break;
            case GameObjType.BirthPoint:
                Debug.Log("birthpoint......");
                break;
            case GameObjType.Wall:
                Debug.Log("wall......");
                break;
            case GameObjType.OutOfBoundBlock:
                Debug.Log("outofboundblock......");
                break;
            default: Debug.Log("abaabaabaaba"); break;
        }
    }

    private void FlushLaidQueue()
    {
        while (!LaidQueue.IsEmpty)
        {
            bool isGood = LaidQueue.TryDequeue(out GameObjInfo res);
            if (isGood)
            {
                var i = Instantiate(_powderBag);
                //i.getComponent<PropScript>().teamId = 
                break;
            }
        }
    }
}