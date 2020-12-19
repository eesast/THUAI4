using System;
using Communication.CSharpClient;
namespace clienttest
{
   class Test
   {
       static void Main(string[] args)
       {
           CSharpClient client = new CSharpClient();
           client.OnReceive += delegate (byte[] bytes)
           {
               string temp = System.Text.Encoding.Default.GetString(bytes);
               Console.WriteLine("Receive from server:" + temp);
           };
            Console.WriteLine("Connecting......");
           if(client.Connect("127.0.0.1", 7777))
            {
                Console.WriteLine("成功连接Agent.");
            }
            else
            {
                Console.WriteLine("连接Agent失败.");
            }
           string sendstr;
           byte[] bytes;
           while (true)
           {
               sendstr = Console.ReadLine();
               if (string.IsNullOrEmpty(sendstr)) break;
               bytes = System.Text.Encoding.Default.GetBytes(sendstr);
               if(!client.Send(bytes)) Console.WriteLine("发送失败");
           }
           client.Stop();
           client.Dispose();
       }
   }
}
