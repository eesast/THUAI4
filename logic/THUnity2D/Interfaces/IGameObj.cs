using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
	public enum ShapeType
	{
		Null = 0,
		Circle = 1,
		Square = 2
	}

	public enum GameObjType
	{
		Character = 0,
		Obj = 1
	}

	public interface IGameObj
	{
		public long ID { get; }
		public XYPosition Position { get; }
		public double FacingDirection { get; }
		public bool IsRigid { get; }
		public ShapeType Shape { get; }
		public bool CanMove { get; set; }
		public bool IsMoving { get; set; }
		public bool IsResetting { get; set; }
		public bool IsAvailable { get; }
		public int Radius { get; }
		public object MoveLock { get; }
	}
}
