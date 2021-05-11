using System.Collections.Generic;
using System.Threading;
using THUnity2D.Utility;

namespace THUnity2D.Interfaces
{
	public interface ITimer
	{
		bool IsGaming { get; }
		public bool StartGame(int timeInMilliseconds);
	}

	public interface IMap
	{
		bool OutOfBound(IGameObj obj);
		IOutOfBoundBlock GetOutOfBoundBlock(XYPosition pos);
		ITimer Timer { get; }
		List<ICharacter> PlayerList { get; }
		List<IObj> ObjList { get; }
		ReaderWriterLockSlim PlayerListLock { get; }
		ReaderWriterLockSlim ObjListLock { get; }
	}
}
