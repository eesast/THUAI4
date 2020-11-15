using System;
using System.Collections.Generic;
using System.Text;
using System.Collections.Concurrent;

namespace THUnity2D
{
    public enum Color{ //墙不可染色
    empty=0,team1=1,team2=2,team3=3,team4=4, wall=5
    }
    public class MapCell:GameObject{//在空中的投掷物无法访问，这是也许是合理的？？没写分层，要是需要到时候再改
        public readonly object publiclock = new object();
        protected readonly object privatelock = new object();
    protected Color  _color;
        protected ConcurrentDictionary<byte, GameObject> objects= new ConcurrentDictionary<byte, GameObject>();//此处的东西,key为0，1，2
        //key0表示地面上的东西，1表示道具，2表示飞行物, 其他人物等东西不会共占一格
        public ConcurrentDictionary<byte, GameObject> Objects {
            get => objects;
        }
    public Color color {
            get => _color;
            set{
                Operations.Add(
                () =>
                {
                    _color = value;
                }
                );
            } 
     }
        public MapCell() {
            _color = Color.empty;
        }
        protected void AddGameObject(GameObject obj, byte key) {
            if (key != 0 && key != 1 && key != 2)
                return;
            lock (privatelock)
            {
                if (!objects.ContainsKey(key))
                {
                    objects.TryAdd(key, obj);
                }
            }
        }

        protected void DeleteGameObject(byte key)
        {
            lock (privatelock) {
                if (!objects.ContainsKey(key))
                    return;
                GameObject? tmp;

                if (objects.TryRemove(key, out tmp))
                    Debug(this, "delete succeed"); 
            
            }
        }

        protected GameObject GetObject(byte key) {
            lock(privatelock){
                if (!objects.ContainsKey(key))
                {
                    return null;
                }
                return objects[key];
            }
        }

    }
}
