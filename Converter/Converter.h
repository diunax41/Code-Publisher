/////////////////////////////////////////////////////////////////////////////
// Converter.h - Convert cpp&header files to valid HTML files - Project 2  //
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
*	This package defines a Converter class.
*	support wrapping code in an html document, and translating 
*	the "<" and ">" characters into their corresponding escape sequences.
*
*	Public Interface:
*	=================
*	Converter();							//void constructor
*	Converter(vector<string>, string);		//constructor
*	<vector> string get_converted_lines();	//return the converted lines
*	void wrap();							//wrap the lines
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

namespace Convert
{
	class Converter
	{
	public:
		Converter();
		Converter(std::vector<std::string>, std::string);
		std::vector<std::string> get_converted_lines();
		void wrap();


	private:
		std::string dst;
		std::vector<std::string> lines;
		void replace();
		//replace "<" to "&lt", ">" to "&gt" in a single line
		void replaceStr(std::string& subject, const std::string& search, const std::string& replace);
	};

	Converter::Converter()
	{
	}

	Converter::Converter(std::vector<std::string> raw, std::string dst_)
	{
		std::cout << "\n    using native c++ package to convert the file" << std::endl;
		lines = raw;
		dst = dst_;
		replace();
		wrap();
	}

	std::vector<std::string> Converter::get_converted_lines()
	{
		return lines;
	}

	void Converter::replaceStr(std::string& subject, const std::string& search,
		const std::string& replace)
	{
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}

	void Converter::replace()
	{
		std::string line;
		for (size_t i = 0; i < lines.size(); i++)
		{
			replaceStr(lines.at(i), "<", "&lt");
			replaceStr(lines.at(i), ">", "&gt");
		}
	}

	void Converter::wrap()
	{
		//FileSystem::Directory::setCurrentDirectory("../TestFiles");
		std::ifstream src_file("../template.html", std::ios::in);
		std::ofstream dst_file(dst, std::ios::out);

		if (!dst_file.is_open())
			std::cout << "\n     Error: can't add template dst" << std::endl;

		if (!src_file.is_open())
			std::cout << "\n     Error: can't add template src" << std::endl;

		if (dst_file.is_open() && src_file.is_open())
		{
			std::string line;
			while (std::getline(src_file, line))
			{
				dst_file << line << std::endl;
			}
			dst_file << std::endl;

		}
		else
		{
			std::cout << "\n can't add template" << std::endl;
			return;
		}

		src_file.close();
		dst_file.close();
	}


}
