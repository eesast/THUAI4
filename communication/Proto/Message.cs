using Google.Protobuf;
using Google.Protobuf.Reflection;
using System;
using System.Linq;
using System.Collections.Generic;
using System.IO;
namespace Communication.Proto
{
    class Message: IMessage
    {
        public int Address;//从哪里发过来的
        public IMessage Content;//信息本身

        public MessageDescriptor Descriptor => null;

        public int CalculateSize()//没太懂为啥要考虑名字的长度，是发送的时候直接把名字也发过去了吗……
        {
            return Content.CalculateSize() + 4 + Content.GetType().FullName.Length;
        }

        public void MergeFrom(Stream stream)
        {
            try
            {
                MergeFrom(new CodedInputStream(stream));
            }
            catch (Exception e)
            {
                Debug.debug($"Unhandled exception while trying to deserialize packet: {e}");
                //stream should be instance of MemoryStream only.
                Debug.debug($"Packet: {string.Concat((stream as MemoryStream).ToArray().Select((b) => $"{b:X2} "))}");
            }
        }

        public void MergeFrom(CodedInputStream input)
        {
                Address = input.ReadInt32();
                string PacketType = input.ReadString();

                Content = Activator.CreateInstance(Type.GetType(PacketType)) as IMessage;

                Content.MergeFrom(input);
                Debug.debug($"{PacketType} received ({Content.CalculateSize()} bytes)");
        }

        public void WriteTo(Stream stream)
        {
            //If true, output is left open when the returned CodedOutputStream is disposed; if false, the provided stream is disposed as well.
            using (CodedOutputStream costream = new CodedOutputStream(stream, true))
            {
                WriteTo(costream);
                costream.Flush();
            }
        }

        public void WriteTo(CodedOutputStream output)
        {
            output.WriteInt32(Address);
            output.WriteString(Content.GetType().FullName);
            Content.WriteTo(output);
            Debug.debug($"{Content.GetType().FullName} sent ({Content.CalculateSize()} bytes)");
        }
    }
}
