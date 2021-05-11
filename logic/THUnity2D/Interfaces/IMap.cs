using System.Collections.Generic;
using System.Threading;

namespace THUnity2D
{
	public interface ITimer
	{
		bool IsGaming { get; }
		public bool StartGame(int timeInMilliseconds);
	}

	public interface IMap
	{
		bool OutOfBound(IGameObj obj);
		ITimer Timer { get; }
		List<ICharacter> PlayerList { get; }
		List<IObj> ObjList { get; }
		ReaderWriterLockSlim PlayerListLock { get; }
		ReaderWriterLockSlim ObjListLock { get; }
	}
}
