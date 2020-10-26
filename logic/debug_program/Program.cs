using System;
using THUnity2D;
//调试代码，没什么用
namespace debug_program
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");
            Console.WriteLine(Mapinfo.map[49,49]);
            //THUnity2D.GameObject a=new THUnity2D.GameObject( new THUnity2D.XYPosition(900,900));//调试
            //THUnity2D.Vector b=new THUnity2D.Vector(Math.PI/3, 100);//ok
            //a.Movable = true;
            //a.Velocity = b;//move ok,第一次设置速度的时候速度处理线程开始执行
            Character a = new Character(JobType.job0, 950, 950);
            a.attack(new Vector(Math.PI/3,100));//ok
           // a.Velocity = new Vector(0, 100);
            System.Threading.Thread.Sleep(5000);
        }
    }
}
