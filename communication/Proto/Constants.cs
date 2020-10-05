//这玩意有什么用呢？我觉得没什么用              
namespace Communication.Proto
{
    class Constants
    {
        public static ushort PlayerCount = 2;
        public static ushort AgentCount = 1;
        public static ushort ServerPort = 10086;
        public static readonly ushort AgentPort = 8887;
        public static int MaxMessage = 3;
        public static double TimeLimit = 1000;   // 在TimeLimt内agent只会转发MaxMessage条消息
        public static readonly int HeartbeatInternal = 1000;
        public static readonly int TokenExpire = 3600;
    }
}
