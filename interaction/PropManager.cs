using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Communication.Proto;
using System.IO;
using Communication.CSharpClient;
using System.Net;

public class PropManager : MonoBehaviour
{
    public GameObject bike; //单车
	public GameObject horn; //冲锋号
	public GameObject schoolBag; //书包
    public GameObject hotpot; //贺岁香锅
    public GameObject shield; //盾牌
    public GameObject clothes; //衣服
    public GameObject javelin; //标枪
    public GameObject puddle; //水坑
    public GameObject musicPlayer; //音乐播放器
    public GameObject mail; //钉子

    void Start()
    {

    }

    void Update()
    {

    }

    public GameObject PropMap(GameObjInfo objValue)
    {
        GameObject retVal;
        switch (objValue.PropType)
        {
            case PropType.Null:
                retVal = null;
                break;
            case PropType.Bike:
                retVal = bike;
                break;
            case PropType.Amplifier:
                retVal = horn;
                break;
            case PropType.JinKeLa:
                retVal = schoolBag;
                break;
            case PropType.Rice:
                retVal = hotpot; 
                break;
            case PropType.NegativeFeedback:
                retVal = shield;
                break;
            case PropType.Totem:
                retVal = clothes;
                break;
            case PropType.Phaser:
                retVal = javelin;
                break;
            case PropType.Dirt:
                retVal = puddle;
                break;
            case PropType.Attenuator:
                retVal = musicPlayer;
                break;
            case PropType.Divider:
                retVal = mail;
                break;
            default: retVal = null; break;
        }
        return retVal;
    }
}