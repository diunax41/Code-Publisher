/////////////////////////////////////////////////////////////////////////////
// Dependencies.h - Convert cpp&header files to valid HTML files-Project 2 //
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
*	This package defines a Dependencies class.
*	support insertion, in each analyzed file, 
*	links to all the files in the directory tree, on which it depends.
*
*	Public Interface:
*	=================
*	Dependencies();							//void constructor
*	Dependencies()(vector<string>, string);	//constructor
*	vector <string> get_cooked_lines();		//return the cooked lines
*
*	Build Process:
*	================
*	Required files
*	- none
*
*
*/
#include <iostream>
#include <vector>

namespace Dependence
{
	class Dependencies
	{
	public:
		Dependencies();
		Dependencies(std::vector<std::string>);
		std::vector<std::string> get_cooked_lines();


	private:
		std::vector<std::string> lines;

		void find_dependencies();
	};

	Dependencies::Dependencies()
	{
	}

	Dependencies::Dependencies(std::vector<std::string> raw)
	{
		std::cout << "\n    using native c++ to find all the dependencies" << std::endl;
		lines = raw;
		find_dependencies();
	}

	void Dependencies::find_dependencies()
	{
		size_t start;
		std::string slice;
		std::string path;
		for (size_t i = 0; i < lines.size(); i++)
		{
			if (lines.at(i).find("#include \"") != std::string::npos)
			{
				for (size_t j = 0; j < lines.at(i).size(); j++)
				{
					if (lines.at(i)[j] == '\"')
					{
						start = j;
						break;
					}
				}
				start++;
				size_t len = lines.at(i).size() - start - 1;
				slice = lines.at(i).substr(start, len);
				path = "<a href=\"../convertedPages/" + slice + ".html\">";
				lines.at(i).insert(start, path);
				lines.at(i).insert(lines.at(i).size() - 1, "</a>");
			}
		}
	}

	std::vector<std::string> Dependencies::get_cooked_lines()
	{
		return lines;
	}

}