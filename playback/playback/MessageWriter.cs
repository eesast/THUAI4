using System;
using System.IO;
using Communication.Proto;
using Google.Protobuf;

namespace playback
{
	public class MessageWriter : IDisposable
	{
		FileStream fs;
		CodedOutputStream cos;

		public MessageWriter(string fileName, uint teamCount, uint playerCount)
		{
			if (!fileName.EndsWith(PlayBackConstant.ExtendedName))
			{
				fileName += PlayBackConstant.ExtendedName;
			}

			fs = new FileStream(fileName, FileMode.Create, FileAccess.Write);
			cos = new CodedOutputStream(fs);
			cos.WriteFixed32(PlayBackConstant.Prefix);
			cos.WriteFixed32(teamCount);
			cos.WriteFixed32(playerCount);
		}

		public void WriteOne(MessageToClient msg)
		{
			cos.WriteMessage(msg);
		}

		public void Flush()
		{
			if (fs.CanWrite)
			{
				fs.Flush();
			}
		}

		public void Dispose()
		{
			if (fs.CanWrite)
			{
				fs.Close();
			}
		}

		~MessageWriter()
		{
			Dispose();
		}
	}
	//public static class PlayBackProducer
	//{
	//	public static void WriteMessage(string fileName, MessageToClient[] msgs)
	//	{
	//		using (FileStream fs = new FileStream(fileName, FileMode.Create, FileAccess.Write))
	//		{
	//			using (CodedOutputStream cos = new CodedOutputStream(fs))
	//			{
	//				cos.WriteFixed64((ulong)msgs.Length);
	//				foreach (var msg in msgs)
	//				{
	//					cos.WriteMessage(msg);
	//				}
	//			}
	//		}
	//	}

	//	public static void 
	//}
}
