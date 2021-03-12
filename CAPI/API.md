## API（暂定）

## 一些说明

对于每一条MessageToClient对应的游戏状态，ai.play()**至多调用一次**。如果play跑得快，一次调用后下一条消息还没有来，ai线程会**阻塞**直到游戏状态更新。如果play跑得快，下一次调用play时对应**最新的**游戏状态，中间的信息不保存。



## API

```c++
class API final :public LogicInterface
{
private:
	virtual void StartTimer() {}
	virtual void EndTimer() {}
public:
	API(std::function<void(Protobuf::MessageToServer&)> sm,
		std::function<bool()> e, std::function<bool(std::string&)> tp,
		const State*& pS);
	virtual void MovePlayer(uint32_t timeInMilliseconds, double angle);
	virtual void MoveRight(uint32_t timeInMilliseconds);
	virtual void MoveUp(uint32_t timeInMilliseconds);
	virtual void MoveLeft(uint32_t timeInMilliseconds);
	virtual void MoveDown(uint32_t timeInMilliseconds);
	virtual void Use();
	virtual void Pick(THUAI4::PropType propType);
	virtual void Throw(uint32_t timeInMilliseconds, double angle);
	virtual void Attack(uint32_t timeInMilliseconds, double angle);
	virtual void Send(int toPlayerID, std::string message);

	//Information the player can get
	virtual bool MessageAvailable();
	virtual bool TryGetMessage(std::string&);

	virtual std::vector<const THUAI4::Character*> GetCharacters() const;
	virtual std::vector<const THUAI4::Wall*> GetWalls() const;
	virtual std::vector<const THUAI4::Prop*> GetProps() const;
	virtual std::vector<const THUAI4::Bullet*> GetBullets() const;
	virtual std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const;
	virtual const THUAI4::Character& GetSelfInfo() const;
	virtual THUAI4::ColorType GetSelfTeamColor() const;
	virtual uint32_t GetTeamScore() const;
	virtual const std::array<std::array<uint32_t, StateConstant::nPlayers>, StateConstant::nTeams>& GetPlayerGUIDs() const override;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
};
```



