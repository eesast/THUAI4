#include<tclap/CmdLine.h>
#include"Base.h"
#include"Logic.h"

Logic logic;

int thuai4_main(int argc, char** argv, CreateAIFunc AIBuilder)
{
	std::string aIP;
	uint16_t aPort;
	int pID;
	int tID;
	THUAI4::JobType jType;
	int level = 0;
	std::string filename;

	try {
		TCLAP::CmdLine cmd("THUAI4 C++接口命令行参数介绍");

		TCLAP::ValueArg<std::string> agentIP("I", "agentIP", "Agent`s IP 127.0.0.1 in default", false, "127.0.0.1", "string");
		cmd.add(agentIP);

		TCLAP::ValueArg<uint16_t> agentPort("P", "agentPort", "Port the agent listens to", true, 0, "USHORT");
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

		std::string DebugDesc = "Set this flag to use API for debugging.\n"\
			"If \"-f\" is not set, the log will be printed on the screen.\n"\
			"Or you could specify a file to store it.";
		TCLAP::SwitchArg debug("d", "debug", DebugDesc);
		cmd.add(debug);
		
		TCLAP::ValueArg<std::string> FileName("f", "filename", "Specify a file to store the log.", false, "", "string");
		cmd.add(FileName);
		
		TCLAP::SwitchArg warning("w", "warning", "Warn of some obviously invalid operations (only when \"-d\" is set).");
		cmd.add(warning);

		cmd.parse(argc, argv);
		aIP = agentIP.getValue();
		aPort = agentPort.getValue();
		pID = playerID.getValue();
		tID = teamID.getValue();
		jType = (THUAI4::JobType)jobType.getValue();
		bool d = debug.getValue();
		bool w = warning.getValue();
		if (d) {
			level = 1 + w;
		}
		filename = FileName.getValue();
	}
	catch (TCLAP::ArgException& e)  // catch exceptions
	{
		std::cerr << "Parsing error: " << e.error() << " for arg " << e.argId() << std::endl;
		return 0;
	}

	logic.Main(aIP.c_str(), aPort, pID, tID, jType, AIBuilder, level,filename);
	return 0;
}
