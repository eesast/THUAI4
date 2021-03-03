#include<tclap/CmdLine.h>
#include"Logic.h"

Logic logic;

int main(int argc, char** argv)
{
	std::string aIP;
	unsigned short aPort;
	int pID;
	int tID;
	Protobuf::JobType jType;
	try
	{
		TCLAP::CmdLine cmd("THUAI4 C++接口命令行参数介绍");

		TCLAP::ValueArg<std::string> agentIP("I", "agentIP", "Agent`s IP 127.0.0.1 in default", false, "127.0.0.1", "string");
		cmd.add(agentIP);

		TCLAP::ValueArg<unsigned short> agentPort("P", "agentPort", "Port the agent listens to", true, 0, "unsigned short");
		cmd.add(agentPort);

		std::vector<int> validPlayerIDs{ 0,1,2,3, };
		TCLAP::ValuesConstraint<int> playerIdConstraint(validPlayerIDs);
		TCLAP::ValueArg<int> playerID("p", "playerID", "Player ID 0,1,2,3 valid only"
			, true, -1, &playerIdConstraint);
		cmd.add(playerID);

		std::vector<int> validTeamIDs{ 0,1 };
		TCLAP::ValuesConstraint<int> temIdConstraint(validTeamIDs);
		TCLAP::ValueArg<int> teamID("t", "teamID", "Team ID, which can only be 0 or 1"
			, true, -1, &temIdConstraint);
		cmd.add(teamID);

		std::string jobDesc = "Player`s job, represented by an integer from 0 to 6\nbalabala";

		std::vector<int> validJobTypes{ 0,1,2,3,4,5,6 };
		TCLAP::ValuesConstraint<int> jobTypeConstraint(validJobTypes);
		TCLAP::ValueArg<int> jobType("j", "jobType", jobDesc
			, true, 0, &jobTypeConstraint);
		cmd.add(jobType);

		cmd.parse(argc, argv);
		aIP = agentIP.getValue();
		aPort = agentPort.getValue();
		pID = playerID.getValue();
		tID = teamID.getValue();
		jType = (Protobuf::JobType)jobType.getValue();
	}
	catch (TCLAP::ArgException& e)  // catch exceptions
	{
		std::cerr << "Parsing error: " << e.error() << " for arg " << e.argId() << std::endl;
		return 0;
	}
	logic.Main(aIP.c_str(), aPort, pID, tID, jType);
	return 0;
}
