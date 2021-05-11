using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
	public interface ICharacter : IGameObj, IMovable
	{
		public long TeamID { get; }
	}
}
