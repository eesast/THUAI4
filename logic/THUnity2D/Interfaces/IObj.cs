namespace THUnity2D.Interfaces
{
	public interface IObj : IGameObj
	{
		ICharacter? Parent { get; set; }
	}
}
