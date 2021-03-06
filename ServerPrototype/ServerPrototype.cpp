/////////////////////////////////////////////////////////////////////////
// Server.cpp - Console App that processes incoming messages		   //
// ver 1.0                                                             //
// Yisheng Pang, CSE687 - Object Oriented Design, Spring 2019          //
/////////////////////////////////////////////////////////////////////////
/*
	This is a server that processes incoming messages, which is
	basically receiving messages and replying messages.
//---- < required files > -------------
	1. server.h
	2. FileSystem.h
	3. Process.h
	4. Loader.h
	5. Converter.h
	6. Dependencies.h
	7. Display.h
//---- < maintenance > -----------------
	version 1.0 25 April, 2019
*/

#include "ServerPrototype.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../Process/Process/Process.h"
#include "../Loader/Loader.h"
#include "../Converter/Converter.h"
#include "../Dependencies/Dependencies.h"
#include "../Display/Display.h"

#include <chrono>
#include <regex>
#include <string>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;

//using namespace Publish;

using Msg = MsgPassingCommunication::Message;

//----< return name of every file on path >----------------------------

Files Server::getFiles(const Repository::SearchPath& path)
{

	return Directory::getFiles(path);
}
//----< return name of every subdirectory on path >--------------------

Dirs Server::getDirs(const Repository::SearchPath& path)
{
	return Directory::getDirectories(path);
}

namespace Publisher
{
	void convertFile(std::string file)
	{
		std::string destination = file + ".html";

		std::ifstream f(destination);
		if (f.good())
		{
			std::cout << destination << " is already exist, no need to convert" << std::endl;
			return;
		}
		std::vector<std::string> lines;
		Load::Loader* loader = new Load::Loader(file);
		lines = loader->get_loaded_lines();
		Convert::Converter* converter = new Convert::Converter(lines, destination);
		lines = converter->get_converted_lines();
		Dependence::Dependencies* depend = new Dependence::Dependencies(lines);
		lines = depend->get_cooked_lines();
		Displayer::Display* dplay = new Displayer::Display(lines, file);
		lines = dplay->get_final_lines();

		std::ofstream output(destination, std::ios::app);
		if (output.is_open())
		{
			output.seekp(output.end);
			for (size_t i = 0; i < lines.size(); i++)
			{
				output << lines.at(i) << std::endl;
			}
			output << "</pre>" << std::endl;
			output << "</body>" << std::endl;
			output << "</html>" << std::endl;
		}
		else
		{
			std::cout << "can't open output file" << std::endl;
		}
		output.close();
	}

	std::string getHtml(std::string file)
	{
		std::ifstream ifs(file);
		if (ifs.is_open())
		{
			std::string content;
			content.assign((std::istreambuf_iterator<char>(ifs)),
				(std::istreambuf_iterator<char>()));
			return content;
		}
		else
		{
			return "";
		}
	}
}

namespace MsgPassingCommunication
{
	// These paths, global to MsgPassingCommunication, are needed by 
	// several of the ServerProcs, below.
	// - should make them const and make copies for ServerProc usage

	std::string sendFilePath;
	std::string saveFilePath;

	//----< show message contents >--------------------------------------

	template<typename T>
	void show(const T& t, const std::string& msg)
	{
		std::cout << "\n  " << msg.c_str();
		for (auto item : t)
		{
			std::cout << "\n    " << item.c_str();
		}
	}
	//----< test ServerProc simply echos message back to sender >--------

	std::function<Msg(Msg)> echo = [](Msg msg) {
		Msg reply = msg;
		reply.to(msg.from());
		reply.from(msg.to());
		return reply;
	};
	//----< getFiles ServerProc returns list of files on path >----------

	std::function<Msg(Msg)> getFiles = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("getFiles");
		std::string path = msg.value("path");
		std::string regex_pattern = msg.value("regex");
		std::smatch m;
		std::regex pattern(regex_pattern);
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != ".")
				searchPath = searchPath + "\\" + path;
			Files files = Server::getFiles(path);
			size_t count = 0;
			for (auto item : files)
			{
				if (regex_pattern == "")
				{
					std::string countStr = Utilities::Converter<size_t>::toString(++count);
					reply.attribute("file" + countStr, item);
				}
				else if (std::regex_match(item, m, pattern))
				{
					std::string countStr = Utilities::Converter<size_t>::toString(++count);
					reply.attribute("file" + countStr, item);
				}

			}
		}
		else
		{
			std::cout << "\n  getFiles message did not define a path attribute";
		}
		return reply;
	};
	//----< getDirs ServerProc returns list of directories on path >-----

	std::function<Msg(Msg)> getDirs = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("getDirs");
		std::string path = msg.value("path");
		if (path != "")
		{
			//std::cout << path << std::endl;
			std::string searchPath = storageRoot;
			if (path != ".")
				searchPath = searchPath + "\\" + path;
			Files dirs = Server::getDirs(path);
			size_t count = 0;
			for (auto item : dirs)
			{
				if (item != ".." && item != ".")
				{
					std::string countStr = Utilities::Converter<size_t>::toString(++count);
					reply.attribute("dir" + countStr, item);
				}
			}
		}
		else
		{
			std::cout << "\n  getDirs message did not define a path attribute";
		}
		return reply;
	};

	//----< publish file >--------------------------------

	std::function<Msg(Msg)> getPublishFiles = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("getPublishFiles");
		std::string file = msg.value("file");
		reply.attribute("file", file);
		Publisher::convertFile(file);
		return reply;

	};

	//----< show published file >---------------------

	std::function<Msg(Msg)> showFile = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("showFile");
		std::string file = msg.value("file");
		reply.attribute("file", file);
		//std::string html = Publisher::getHtml(file);
		//reply.attribute("html", html);
		return reply;

	};

	//----< open html file in a browser >---------------------
	std::function<Msg(Msg)> browserFile = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("browserFile");
		std::string file = msg.value("file");
		reply.attribute("file", file);
		return reply;
	};


	//----< sendFile ServerProc sends file to requester >----------------
	/*
	*  - Comm sends bodies of messages with sendingFile attribute >------
	*/
	std::function<Msg(Msg)> sendFile = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("sendFile");
		reply.attribute("sendingFile", msg.value("fileName"));
		reply.attribute("fileName", msg.value("fileName"));
		reply.attribute("verbose", "blah blah");
		std::string path = msg.value("path");
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != "." && path != searchPath)
				searchPath = searchPath + "\\" + path;
			if (!FileSystem::Directory::exists(searchPath))
			{
				std::cout << "\n  file source path does not exist";
				return reply;
			}
			std::string filePath = searchPath + "/" + msg.value("fileName");
			std::string fullSrcPath = FileSystem::Path::getFullFileSpec(filePath);
			std::string fullDstPath = sendFilePath;
			if (!FileSystem::Directory::exists(fullDstPath))
			{
				std::cout << "\n  file destination path does not exist";
				return reply;
			}
			fullDstPath += "/" + msg.value("fileName");
			FileSystem::File::copy(fullSrcPath, fullDstPath);
		}
		else
		{
			std::cout << "\n  getDirs message did not define a path attribute";
		}
		return reply;
	};

	//----< analyze code on current server path >--------------------------
	/*
	*  - Creates process to run CodeAnalyzer on specified path
	*  - Won't return until analysis is done and logfile.txt
	*    is copied to sendFiles directory
	*/
	std::function<Msg(Msg)> codeAnalyze = [](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("sendFile");
		reply.attribute("sendingFile", "logfile.txt");
		reply.attribute("fileName", "logfile.txt");
		reply.attribute("verbose", "blah blah");
		std::string path = msg.value("path");
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != "." && path != searchPath)
				searchPath = searchPath + "\\" + path;
			if (!FileSystem::Directory::exists(searchPath))
			{
				std::cout << "\n  file source path does not exist";
				return reply;
			}
			// run Analyzer using Process class

			Process p;
			p.title("test application");
			//std::string appPath = "c:/su/temp/project4sample/debug/CodeAnalyzer.exe";
			std::string appPath = "CodeAnalyzer.exe";
			p.application(appPath);

			//std::string cmdLine = "c:/su/temp/project4Sample/debug/CodeAnalyzer.exe ";
			std::string cmdLine = "CodeAnalyzer.exe ";
			cmdLine += searchPath + " ";
			cmdLine += "*.h *.cpp /m /r /f";
			//std::string cmdLine = "c:/su/temp/project4sample/debug/CodeAnalyzer.exe ../Storage/path *.h *.cpp /m /r /f";
			p.commandLine(cmdLine);

			std::cout << "\n  starting process: \"" << appPath << "\"";
			std::cout << "\n  with this cmdlne: \"" << cmdLine << "\"";

			CBP callback = []() { std::cout << "\n  --- child process exited ---"; };
			p.setCallBackProcessing(callback);

			if (!p.create())
			{
				std::cout << "\n  can't start process";
			}
			p.registerCallback();

			std::string filePath = searchPath + "\\" + /*msg.value("codeAnalysis")*/ "logfile.txt";
			std::string fullSrcPath = FileSystem::Path::getFullFileSpec(filePath);
			std::string fullDstPath = sendFilePath;
			if (!FileSystem::Directory::exists(fullDstPath))
			{
				std::cout << "\n  file destination path does not exist";
				return reply;
			}
			fullDstPath += std::string("\\") + /*msg.value("codeAnalysis")*/ "logfile.txt";
			FileSystem::File::copy(fullSrcPath, fullDstPath);
		}
		else
		{
			std::cout << "\n  getDirs message did not define a path attribute";
		}
		return reply;
	};
}


using namespace MsgPassingCommunication;

int main()
{
	SetConsoleTitleA("Project4Sample Server Console");

	std::cout << "\n  Testing Server Prototype";
	std::cout << "\n ==========================";
	std::cout << "\n";

	//StaticLogger<1>::attach(&std::cout);
	//StaticLogger<1>::start();

	sendFilePath = FileSystem::Directory::createOnPath("./SendFiles");
	saveFilePath = FileSystem::Directory::createOnPath("./SaveFiles");

	Server server(serverEndPoint, "ServerPrototype");

	// may decide to remove Context
	MsgPassingCommunication::Context* pCtx = server.getContext();
	pCtx->saveFilePath = saveFilePath;
	pCtx->sendFilePath = sendFilePath;

	server.start();

	std::cout << "\n  testing getFiles and getDirs methods";
	std::cout << "\n --------------------------------------";
	Files files = server.getFiles();
	show(files, "Files:");
	Dirs dirs = server.getDirs();
	show(dirs, "Dirs:");
	std::cout << "\n";

	std::cout << "\n  testing message processing";
	std::cout << "\n ----------------------------";
	server.addMsgProc("echo", echo);
	server.addMsgProc("getFiles", getFiles);
	server.addMsgProc("getDirs", getDirs);
	server.addMsgProc("sendFile", sendFile);
	server.addMsgProc("codeAnalyze", codeAnalyze);
	server.addMsgProc("serverQuit", echo);
	server.addMsgProc("getPublishFiles", getPublishFiles);
	server.addMsgProc("showFile", showFile);
	server.addMsgProc("browserFile", browserFile);

	server.processMessages();

	Msg msg(serverEndPoint, serverEndPoint);  // send to self
	msg.name("msgToSelf");

	/////////////////////////////////////////////////////////////////////
	// Additional tests here, used during development
	//
	//msg.command("echo");
	//msg.attribute("verbose", "show me");
	//server.postMessage(msg);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//msg.command("getFiles");
	//msg.remove("verbose");
	//msg.attributes()["path"] = storageRoot;
	//server.postMessage(msg);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	//msg.command("getDirs");
	//msg.attributes()["path"] = storageRoot;
	//server.postMessage(msg);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	std::cout << "\n  press enter to exit\n";
	std::cin.get();
	std::cout << "\n";

	msg.command("serverQuit");
	server.postMessage(msg);
	server.stop();
	return 0;
}

