//效法前人，套层壳
using System;
using System.IO;
using Google.Protobuf;

namespace Communication.Proto
{
    public enum PacketType
    {
        MessageToClient = 0,
        MessageToServer = 1,
        MessageToOneClient = 2
    }

    public interface IMsg//暴露给用户的接口 或许名起的不太好
    {
        PacketType PacketType { get; set; }
        IMessage Content { get; set; }
    }

    public class Message : IMsg
    {
        private PacketType type;
        public PacketType PacketType { get => type; set => type = value; }
        private IMessage content;
        public IMessage Content { get => content; set => content = value; }

        //这两个方法也是效法前人 传递类名字符串用来创建（似乎比较低效？但写起来简单hhh）
        public void MergeFrom(byte[] bytes)
        {
            MemoryStream istream = new MemoryStream(bytes);
            BinaryReader br = new BinaryReader(istream);
            type = (PacketType)br.ReadInt32();
            try
            {
                CodedInputStream input = new CodedInputStream(istream);
                string typename = input.ReadString();
                content = Activator.CreateInstance(Type.GetType(typename)) as IMessage;
                content.MergeFrom(input);
            }
            catch (Exception e)
            {
                Console.WriteLine($"Unhandled exception while trying to deserialize packet: {e}");
                //stream should be instance of MemoryStream only.
            }
        }


        public void WriteTo(out byte[] bytes)
        {
            MemoryStream ostream = new MemoryStream();
            BinaryWriter bw = new BinaryWriter(ostream);
            bw.Write((int)type);
            bw.Flush();
            //If true, output is left open when the returned CodedOutputStream is disposed; if false, the provided stream is disposed as well.
            using (CodedOutputStream output = new CodedOutputStream(ostream, true))
            {
                output.WriteString(Content.GetType().FullName);
                Content.WriteTo(output);
                output.Flush();
            }
            bytes = ostream.ToArray();
        }
    }
}