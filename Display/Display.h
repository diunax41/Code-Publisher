/////////////////////////////////////////////////////////////////////////////
// Display.h - Convert cpp&header files to valid HTML files - Project 2    //
// ver 1.0                                                                 //
// ----------------------------------------------------------------------- //
// copyright   Yisheng Pang, 2019                                          //
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
*	This package defines a Displayer class.
*	Support for wrapping up the file with html tag <span>
*	in order to make it display on the browser.
*	
*	Public Interface:
*	=================
*	Display();							//void constructor
*	Display(vector<string>, string);	//constructor
*	<vector> string get_final_lines();	//return the final lines
*
*	Build Process:
*	================
*	Required files
*	- AbstrSynTree.h, FileSystem.h, Parser.h, ConfigureParser.h
*	
*	
*/
#include <vector>
#include "../AbstractSyntaxTree/AbstrSynTree.h"


#include "../FileSystem/FileSystem.h"
#include "../Parser/Parser.h"
#include "../Parser/ConfigureParser.h"

namespace Displayer
{
	class Display
	{
	public:
		Display();
		Display(std::vector<std::string>, std::string);
		std::vector<std::string> get_final_lines();


	private:
		std::vector<std::string> lines;
		std::string file_name;
		void getDisplay();
		void hide_comment();
	};

	Display::Display()
	{
	}

	Display::Display(std::vector<std::string> raw, std::string file)
	{
		std::cout << "\n    using native c++ Display package to handle the file" << std::endl;
		file_name = file;
		lines = raw;
		getDisplay();
		hide_comment();
	}

	void Display::getDisplay()
	{
		CodeAnalysis::ConfigParseForCodeAnal configure;
		CodeAnalysis::Parser* pParser = configure.Build();
		//parse the file and hide all class and function
		try
		{
			if (pParser)
			{

				if (!configure.Attach(file_name))
				{
					std::cout << "\n  could not open file " << file_name << std::endl;
					return;
				}
			}
			else
			{
				std::cout << "\n\n  Parser not built\n\n";
				return;
			}

			// save current package name

			CodeAnalysis::Repository* pRepo = CodeAnalysis::Repository::getInstance();
			pRepo->package() = file_name;

			// parse the package

			while (pParser->next())
			{
				pParser->parse();
			}
			std::cout << "\n";

			// final AST operations
			CodeAnalysis::ASTNode* pGlobalScope = pRepo->getGlobalScope();


			// Walk AST, displaying each element, indented
			// by generation

			CodeAnalysis::myTreeWalk(pGlobalScope, lines);
		}
		catch (std::exception& ex)
		{
			std::cout << "\n\n    " << ex.what() << "\n\n";
			std::cout << "\n  exception caught at line " << __LINE__ << " ";
			std::cout << "\n  in package \"" << file_name << "\"";
		}
	}

	std::vector<std::string> Display::get_final_lines()
	{
		return lines;
	}

	void Display::hide_comment()
	{
		size_t single_pos;
		size_t mul_pos;
		size_t end;
		bool flag = false;
		for (size_t i = 0; i < lines.size(); i++)
		{
			if (flag)
			{
				//in the multiple comment
				end = lines.at(i).find("*/");
				if (end != std::string::npos)
				{
					lines.at(i).insert(end + 2, "</span>");
					flag = false;
				}
			}
			else
			{
				mul_pos = lines.at(i).find("/*");
				single_pos = lines.at(i).find("//");
				if (mul_pos != std::string::npos)
				{
					lines.at(i).insert(mul_pos, "<span class=\"toggleCommentDisplay\">");
					flag = true;
				}
				else if (single_pos != std::string::npos)
				{
					lines.at(i).insert(single_pos, "<span class=\"toggleCommentDisplay\">");
					lines.at(i).append("</span>");
				}
			}
		}
	}


}