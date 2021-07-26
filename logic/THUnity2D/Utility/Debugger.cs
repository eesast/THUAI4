using System;

namespace THUnity2D.Utility
{
	public class Debugger
	{

		//用于Debug时从控制台观察到各个游戏对象的状况
		public static void Output(object current, string str)
		{
#if DEBUG
			Console.WriteLine(current.GetType() + " " + current.ToString() + str);
#endif
		}
	}
}
