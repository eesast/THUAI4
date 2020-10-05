using System;
using System.Diagnostics;
namespace Communication.Proto
{
    public static class Debug
    {
        public delegate void DebugFunc(string DebugMessage);
        public static DebugFunc debug=(string DebugMessage)=> { };
        private static int debuglevel = -1;
        public static int DebugLevel
        {
            get => debuglevel;
            set
            {
                if (debuglevel != -1) return;//假设只能赋值一次好吧，要不然debug函数就gg了
                if (value > 2) debuglevel = 2;
                else if (value < 0) debuglevel = 0;
                else debuglevel = value;
                if (DebugLevel == 1)
                {
                    debug += new DebugFunc((string DebugMessage) =>
                    {
                        var stack = new StackTrace();
                        var method = stack.GetFrame(1).GetMethod();
                        Console.WriteLine($"[{method.DeclaringType.Name}/{method.Name}] {DebugMessage}");
                    }
                        );
                }
                else if (DebugLevel == 2)//这个就是microsoft文档中的样例（似乎没什么卵用）
                {
                    debug += new DebugFunc((string DebugMessage) =>
                    {
                        StackTrace st = new StackTrace(true);
                        var method = st.GetFrame(1).GetMethod();
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine($"[{method.DeclaringType.Name}/{method.Name}] {DebugMessage}");
                        Console.ResetColor();
                        string stackIndent = "";
                        for (int i = 0; i < st.FrameCount; i++)
                        {
                            StackFrame sf = st.GetFrame(i);
                            Console.WriteLine();
                            Console.WriteLine(stackIndent + " Method: {0}",
                                sf.GetMethod());
                            Console.WriteLine(stackIndent + " File: {0}",
                                sf.GetFileName());
                            Console.WriteLine(stackIndent + " Line Number: {0}",
                                sf.GetFileLineNumber());
                            stackIndent += "  ";
                        }
                    }
                        );
                }
            }
        }
    }
}
