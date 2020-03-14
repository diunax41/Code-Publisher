/////////////////////////////////////////////////////////////////////
// Publisher.cpp - A publisher to publish cpp or header file       //
// ver 1.0                                                         //
//-----------------------------------------------------------------//
// Language:    C++, Visual Studio 2017                            //
// Author:      Yisheng Pang, Syracuse University, Spring 2019     //
//              yipang@syr.edu		                               //
/////////////////////////////////////////////////////////////////////
/*
*  This file defines a private class, Publisher, that implements the
*  IPublisher interface.  Clients only need to use the createPublisher
*  factory function and the interface pointer it returns.
*
*  ---------- < required files > -------------------------
*	1. IPublisher.h
*	2. Loader.h
*	3. Converter.h
*	4. Dependencies.h
*	5. Display.h
*	6. FileSystem.h
*	7. Utilities.h
*
*	--------- < maintenance history> ----------------------
*	version 1.0 1 April 2019
*	version 1.1 29 April 2019
*/

#include <fstream>
#include <sstream>
#include <cstring>
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <iostream>


#include "IPublisher.h"
#include "../Loader/Loader.h"
#include "../Converter/Converter.h"
#include "../Dependencies/Dependencies.h"
#include "../Display/Display.h"
#include "../FileSystem/FileSystem.h"
#include "../Utilities/Utilities.h"


class Publisher : public IPublisher
{
public:
	virtual void putString(const std::string& str) override
	{
		str_ = str;
	}
	virtual std::string getString() override
	{
		return str_;
	}

	virtual std::vector<double> convertToSquares(const std::vector<double>& vecDble) override
	{
		std::vector<double> squares;
		for (double item : vecDble)
			squares.push_back(item*item);
		return squares;
	}

	//----< convert a cpp or header file to html and store it in the current directory >--------

	virtual void ConvertToHtml(std::string file_spec) override
	{

		std::string destination = file_spec + ".html";

		std::ifstream f(destination);
		if (f.good())
		{
			std::cout << destination <<" is already exist, no need to convert" << std::endl;
			return;
		}
		std::vector<std::string> lines;
		loader = new Load::Loader(file_spec);
		lines = loader->get_loaded_lines();
		converter = new Convert::Converter(lines, destination);
		lines = converter->get_converted_lines();
		depend = new Dependence::Dependencies(lines);
		lines = depend->get_cooked_lines();
		dplay = new Displayer::Display(lines, file_spec);
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

	//----< return a string of html file >-------------------------------
	
	virtual std::string getHtml(std::string file_spec) override
	{
		std::ifstream ifs(file_spec);
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

private:
	Load::Loader* loader;
	Convert::Converter* converter;
	Dependence::Dependencies* depend;
	Displayer::Display* dplay;
	std::string str_;
};



///////////////////////////////////////////////////////////
// Factory function for NativeCpp

IPublisher* createPublisher()
{
	return new Publisher();
}

#ifdef TEST_PUBLISHER
int main()
{
	std::cout << "**********Publisher test************" << std::endl;
	std::string current_dir = FileSystem::Directory::getCurrentDirectory();
	Publisher* p;
	p->ConvertToHtml(current_dir);
	std::string htmlStr = p->getHtml(current_dir);
	std::cout << htmlStr << std::endl;
	return 0;
}
#endif // TEST_PUBLISHER

