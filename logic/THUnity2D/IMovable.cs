using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
	public interface IMovable
	{
		public bool IsResetting { get; }
		public bool IsAvailable { get; }
		public bool IsMoving { get; set; }
		public bool IsRigid { get; }
		public bool CanMove { get; }
		public GameObject.ShapeType Shape { get; }
		public int Radius { get; }
		public int MoveSpeed { get; }
		public XYPosition Position { get; }
		public object MoveLock { get; }

		public long Move(Vector moveVec);
		public bool WillCollideWith(GameObject targetObj, XYPosition nextPos);
	}
}
