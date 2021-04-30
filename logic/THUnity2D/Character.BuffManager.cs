using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;

namespace THUnity2D
{
	public sealed partial class Character
	{

		private BuffManeger buffManeger;

		// Buff 管理类

		private class BuffManeger
		{

			private enum BuffType : uint        //有哪些加成
			{
				MoveSpeed = 0u,
				AP = 1u,
				CD = 2u,
				Shield = 3u,
				Totem = 4u,
				Spear = 5u
			}
			private const uint BuffTypeNum = 6u;        //加成的种类个数，即enum BuffType的成员个数

			[StructLayout(LayoutKind.Explicit, Size = 8)]
			private struct BuffValue                    //加成参数联合体类型，可能是int或double
			{
				[FieldOffset(0)]
				public int iValue;
				[FieldOffset(0)]
				public double lfValue;

				public BuffValue(int intValue) { this.lfValue = 0.0; this.iValue = intValue; }
				public BuffValue(double longFloatValue) { this.iValue = 0; this.lfValue = longFloatValue; }
			}

			private LinkedList<BuffValue>[] buffList;
			private object[] buffListLock;

			private void AddBuff(BuffValue bf, int buffTime, BuffType buffType, Action ReCalculateFunc)
			{
				new Thread
					(
						() =>
						{
							LinkedListNode<BuffValue> buffNode;
							lock (buffListLock[(uint)buffType])
							{
								buffNode = buffList[(uint)buffType].AddLast(bf);
							}
							ReCalculateFunc();
							Thread.Sleep(buffTime);
							try
							{
								lock (buffListLock[(uint)buffType])
								{
									buffList[(uint)buffType].Remove(buffNode);
								}
							}
							catch { }
							ReCalculateFunc();
						}
					)
				{ IsBackground = true }.Start();
			}

			private int ReCalculateFloatBuff(BuffType buffType, int orgVal, int maxVal, int minVal)
			{
				double times = 1.0;
				lock (buffListLock[(uint)buffType])
				{
					foreach (var add in buffList[(uint)buffType])
					{
						times *= add.lfValue;
					}
				}
				return Math.Max(Math.Min((int)Math.Round(orgVal * times), maxVal), minVal);
			}

			public void AddMoveSpeed(double add, int buffTime, Action<int> SetNewMoveSpeed, int orgMoveSpeed)
				=> AddBuff(new BuffValue(add), buffTime, BuffType.MoveSpeed, () => SetNewMoveSpeed(ReCalculateFloatBuff(BuffType.MoveSpeed, orgMoveSpeed, MaxSpeed, MinSpeed)));

			public void AddAP(double add, int buffTime, Action<int> SetNewAp, int orgAp)
				=> AddBuff(new BuffValue(add), buffTime, BuffType.AP, () => SetNewAp(ReCalculateFloatBuff(BuffType.AP, orgAp, MaxAP, MinAP)));

			public void ChangeCD(double discount, int buffTime, Action<int> SetNewCD, int orgCD)
				=> AddBuff(new BuffValue(discount), buffTime, BuffType.CD, () => SetNewCD(ReCalculateFloatBuff(BuffType.CD, orgCD, int.MaxValue, 1)));

			public void AddShield(int shieldTime) => AddBuff(new BuffValue(), shieldTime, BuffType.Shield, () => { });
			public bool HasShield
			{
				get
				{
					lock (buffListLock[(uint)BuffType.Shield])
					{
						return buffList[(uint)BuffType.Shield].Count != 0;
					}
				}
			}

			public void AddTotem(int totemTime) => AddBuff(new BuffValue(), totemTime, BuffType.Totem, () => { });
			public bool HasTotem
			{
				get
				{
					lock (buffListLock[(uint)BuffType.Totem])
					{
						return buffList[(uint)BuffType.Totem].Count != 0;
					}
				}
			}
			public bool TryActivatingTotem()
			{
				if (HasTotem)
				{
					lock (buffListLock[(uint)BuffType.Totem])
					{
						buffList[(uint)BuffType.Totem].Clear();
					}
					return true;
				}
				return false;
			}

			public void AddSpear(int spearTime) => AddBuff(new BuffValue(), spearTime, BuffType.Spear, () => { });
			public bool HasSpear
			{
				get
				{
					lock (buffListLock[(uint)BuffType.Spear])
					{
						return buffList[(uint)BuffType.Spear].Count != 0;
					}
				}
			}

			public void ClearAll()
			{
				for (int i = 0; i < BuffTypeNum; ++i)
				{
					lock (buffListLock[i])
					{
						buffList[i].Clear();
					}
				}
			}

			public BuffManeger()
			{

				buffList = new LinkedList<BuffValue>[BuffTypeNum];
				for (int i = 0; i < buffList.Length; ++i)
				{
					buffList[i] = new LinkedList<BuffValue>();
				}

				buffListLock = new object[buffList.Length];
				for (int i = 0; i < buffListLock.Length; ++i)
				{
					buffListLock[i] = new object();
				}

			}
		}
	}
}
