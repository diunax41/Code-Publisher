/////////////////////////////////////////////////////////////////////////////
// Loader.h - Convert cpp&header files to valid HTML files - Project 2     //
// ver 1.0                                                                 //
// ----------------------------------------------------------------------- //
// copyright ? Yisheng Pang, 2019                                          //
// All rights granted provided that this notice is retained                //
// ----------------------------------------------------------------------- //
// Language:    Visual C++, Visual Studio 2017                             //
// Platform:    Dell XPS 9360, Core i5, Windows 10                         //
// Application: CSE 687 Projects, Spring 2019                              //
// Author:      Yisheng Pang, Syracuse University                          //
//              yipang@syr.edu                                             //
/////////////////////////////////////////////////////////////////////////////
/*
*	Package Operations:
*	===================
*	This package defines a Loader class.
*	support finding and loading all the files for conversion,
*	starting at a path specified by the Executive.
*
*	Public Interface:
*	=================
*	Loader();							//void constructor
*	Loader(vector<string>, string);		//constructor
*	<vector> string get_loader_lines();	//return the final lines
*
*	Build Process:
*	================
*	Required files
*	- none
*
*
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace Load
{
	class Loader
	{
	public:
		Loader();
		Loader(std::string fileSpec);
		std::vector<std::string> get_loaded_lines();

	private:
		std::vector<std::string> lines;
	};

	Loader::Loader()
	{
	}

	Loader::Loader(std::string fileSpec)
	{
		std::cout << "\n    using native c++ loader package to load " << fileSpec << std::endl;
		//open src and dst file
		std::ifstream file;
		file.open(fileSpec, std::ios::in);
		if (file.is_open())
		{
			std::string line;
			while (std::getline(file, line))
			{
				lines.push_back(line);
			}
		}
		file.close();
	}

	std::vector<std::string> Loader::get_loaded_lines()
	{
		return lines;
	}



}