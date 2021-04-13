using System;
using System.IO;
using Communication.Proto;
using Google.Protobuf;

namespace playback
{
	public class FileFormatNotLegalException : Exception
	{
		private readonly string fileName;
		public FileFormatNotLegalException(string fileName)
		{
			this.fileName = fileName;
		}
		public override string Message => $"The file: " + this.fileName + " is not a legal playback file for THUAI4.";
	}

	public class MessageReader : IDisposable
	{
		FileStream fs;
		CodedInputStream cos;
		public readonly uint teamCount;
		public readonly uint playerCount;

		public MessageReader(string fileName)
		{
			if (!fileName.EndsWith(PlayBackConstant.ExtendedName))
			{
				fileName += PlayBackConstant.ExtendedName;
			}

			fs = new FileStream(fileName, FileMode.Open, FileAccess.Read);
			cos = new CodedInputStream(fs);

			try
			{
				uint prefix = cos.ReadFixed32();
				if (prefix != PlayBackConstant.Prefix) throw new FileFormatNotLegalException(fileName);
				teamCount = cos.ReadFixed32();
				playerCount = cos.ReadFixed32();
			}
			catch
			{
				throw new FileFormatNotLegalException(fileName);
			}
		}

		public MessageToClient ReadOne()
		{
			try
			{
				MessageToClient msg = new MessageToClient();
				cos.ReadMessage(msg);
				return msg;
			}
			catch { }
			return null;
		}

		public void Dispose()
		{
			if (fs.CanRead)
			{
				fs.Close();
			}
		}

		~MessageReader()
		{
			Dispose();
		}
	}
}
