#pragma once
///////////////////////////////////////////////////////////////
// INativeCpp.h - Translates managed calls into native calls //
//                                                           //
// Yisheng Pang, CSE687 - Object Oriented Design, Spring 2019//
///////////////////////////////////////////////////////////////
/*
*  This package provides an interface for the Publisher class
*  and a declaration of an object factory function.  Using these
*  isolates the user from all implementation details in Publisher.
*
*/
#include <string>
#include <vector>

struct IPublisher
{
	virtual ~IPublisher() {}
	virtual void ConvertToHtml(std::string file_spec) = 0;
	virtual void putString(const std::string& str) = 0;
	virtual std::string getString() = 0;
	virtual std::vector<double> convertToSquares(const std::vector<double>& vecDble) = 0;
	virtual std::string getHtml(std::string file_spec) = 0;
};

IPublisher* createPublisher();