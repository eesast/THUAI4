using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.PlayerLoop;

public class FloorManager : MonoBehaviour{
    public const int MAP_SIZE = 19;
    public GameObject funit;
    public Object[,] FloorUnit;
    public static string[,] FloorColor;
    public static bool[,] ColorChanged;
    public static Material[] Materials;
    public static Dictionary<string, int> Color2Index;
    
    void Start() {
        FloorUnit = new Object[MAP_SIZE,MAP_SIZE];
        FloorColor = new string[MAP_SIZE,MAP_SIZE];
        ColorChanged = new bool[MAP_SIZE, MAP_SIZE];
        Color2Index = new Dictionary<string, int>();
        
        CreateTerrain();
        
        Materials = Resources.LoadAll<Material>("Materials");
        Color2Index.Add("brown", 0);
        Color2Index.Add("green", 1);
        Color2Index.Add("grey", 2);
        Color2Index.Add("yellow", 3);
        Color2Index.Add("blue", 4);

        for(int i = 0; i < MAP_SIZE; i++) 
            for(int j = 0; j < MAP_SIZE; j++) 
                SetColor(i, j, "yellow");
    }

    // Update is called once per frame
    // 建立一个数组存server发的信息
    void Update() {
        //处理信息队列
        
        UpdateColor();
    }
    
    void UpdateColor(){
        for(int i = 0; i < MAP_SIZE; i++) {
            for(int j = 0; j < MAP_SIZE; j++) {
                if (!ColorChanged[i, j])
                    continue;
                GameObject tmp = FloorUnit[i, j] as GameObject;
                tmp.GetComponent<FloorUnit>().unitColor(Materials[Color2Index[FloorColor[i, j]]]);
                ColorChanged[i, j] = false;
            }
        }
    }

    public static void SetColor(int xPos, int yPos, string unitColor){
        FloorColor[xPos, yPos] = unitColor;
        ColorChanged[xPos, yPos] = true;
    }
    //Onrecieve()

    void CreateTerrain(){
        //这个函数用来创建地形，包括地面、墙壁等
        
        //地面
        for(int i = 0; i < MAP_SIZE; i++) {
             for(int j = 0; j < MAP_SIZE; j++) {
                 FloorUnit[i, j] = Instantiate(funit, new Vector3((i-MAP_SIZE/2)*1.0f, 0f, (j-MAP_SIZE/2)*1.0f), Quaternion.identity);
                 GameObject tmp = FloorUnit[i, j] as GameObject;
                 tmp.GetComponent<FloorUnit>().xPos = i;
                 tmp.GetComponent<FloorUnit>().yPos = j;
             }
        }
        
        //墙壁
        
    }
}