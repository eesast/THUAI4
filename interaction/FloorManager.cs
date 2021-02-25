using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.PlayerLoop;
using Es.InkPainter;

public class FloorManager : MonoBehaviour{
    public const int MAP_SIZE = 19;
    public static string[,] FloorColor;
    public static bool[,] ColorChanged;
    public static Material[] Materials;
    public static Dictionary<string, int> Color2Index;
    
    void Start() {
        FloorColor = new string[MAP_SIZE,MAP_SIZE];
        //ColorChanged = new bool[MAP_SIZE, MAP_SIZE];
        Color2Index = new Dictionary<string, int>();
        
        CreateTerrain();
        
        Vector3 pos1 = new Vector3(3, 0, 3);
        Vector3 pos2 = new Vector3(4, 0, 4);
        GetComponent<SomewherePainter>().ScopePaint(pos1, pos2);
        /*Materials = Resources.LoadAll<Material>("Materials");
        */
    }

    // Update is called once per frame
    // 建立一个数组存server发的信息
    void Update() {
        //处理信息队列
        
        //UpdateColor();
    }
    
    //Onrecieve()

    void CreateTerrain(){
        //这个函数用来创建地形，包括地面、墙壁等
        
        //地面
        
        //墙壁
        
    }
}
