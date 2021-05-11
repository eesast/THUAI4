namespace THUnity2D.Interfaces
{
	public interface ICharacter : IGameObj, IMovable
	{
		public long TeamID { get; }
	}
}
