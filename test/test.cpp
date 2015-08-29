/**
 @file
 @brief LiTest example program.
 @author  August Ernstsson <augern@icloud.com>
 @version 1.0
 
 @section LICENSE
 Copyright (c) 2015 August Ernstsson.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 - The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 **THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.**
 
 @section DESCRIPTION
 See README.md.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <exception>
#include <regex>

#include "litest.hpp"

/**
 @brief A custom litest::TestResultFormatter.
 
 This formatter outputs plain text.
 */
class MyCustomTestResultFormatter : public litest::TestResultFormatter
{
public:
	
	/** Constructor wrapping the superclass's constructor. */
	MyCustomTestResultFormatter(std::ostream &ostr)
	: litest::TestResultFormatter(ostr) {}
	
	virtual void formatTestHeader(litest::Test const& test) override
	{
		s << "Starting new test: " << test.name << std::endl;
	}
	
	virtual void formatUnexpectedException(int line, std::string expr, std::string msg) override
	{
		s << "Unexpected exception at line " << line << "!" << std::endl;
	}
	
	virtual void formatTestSuiteEnd(litest::TestSuite const& suite)
	{
		int passes = suite.totalTestStats().passes;
		int fails = suite.totalTestStats().fails;
		int assertions = passes + fails;
		size_t test_cases = suite.tests.size();
		
		s << "\n===============================================================================\n";
		if (fails == 0)
			s << "All test passed (" << assertions << " assertions in " << test_cases << " test cases).\n";
		else
			s << "Not all test cases passed.";
	}
};


// Custom type for testing
struct TestType
{
	int value;
	TestType(int val) : value(val) {}
	bool operator==(TestType rhs) { return this->value == rhs.value; }
};


//, implemeting operator<<
struct PrintableType : TestType
{
	PrintableType(int val = 0) : TestType(val) {}
};

std::ostream& operator<<(std::ostream& os, PrintableType pt)
{
	return os << pt.value;
}

// Custom type for testing, NOT implemeting operator<<
struct NonPrintableType : TestType
{
	NonPrintableType(int val = 0) : TestType(val) {}
};


/** The main function. */
int main()
{
//	runExample();
	
	litest::TestSuite suite("LiTest demonstration");
	
	LT_ADD_TEST(suite, "Tests that pass",
	{
		std::vector<int> vec;
		
		// Assert that an expression evaluates to true:
		LT_CHECK(vec.empty());
		
		// Add a message to test output:
		LT_MESSAGE("Adding an element to the vector");
		
		vec.push_back(42);
		vec.push_back(56);
		
		// Assert equality of two expressions:
		LT_EQUAL(vec.size(), 2);
		
		// Assert that an expression throws:
		LT_THROWS(throw "Bad code");
		
		// Assert that an expression throws a certain type:
		LT_EXCEPT(vec.at(5), std::out_of_range);
		
		// Print the value of an expression
		LT_PRINT_EXPR(vec);
	});
	
	LT_ADD_TEST(suite, "Tests that fail",
	{
		// Some failing assertions:
		LT_CHECK(1 > 2);
		LT_EQUAL(1 + 1, 3);
		LT_THROWS(1);
		LT_EXCEPT(throw std::logic_error("ERROR!"), std::out_of_range);
		
		// Add an assertion failure manually:
		LT_FAIL("Some code went awry!");
	});
	
	LT_ADD_TEST(suite, "Test that is aborted early",
	{
		// An assertion can be required to pass, otherwise the test will be aborted.
		// This assertion will fail ...
		LT_MESSAGE("Test should be aborted on the next line");
		LT_REQUIRE(42 > 1E100);
		
		// ... so this assertion will be ignored:
		LT_CHECK(true);
	});
	
	LT_ADD_TEST(suite, "Test with throw outside of assertions",
	{
		LT_CHECK(INT_MAX > 5);
		throw std::logic_error("ERROR!");
		throw 4;
	});
	
	LT_ADD_TEST(suite, "Test with custom types in assertion",
	{
		LT_MESSAGE("Printable type");
		PrintableType printableA(5);
		PrintableType printableB(5);
		
		LT_EQUAL(printableA, printableB);
		
		LT_MESSAGE("Non-printable type");
		NonPrintableType nonPrintableA(5);
		NonPrintableType nonPrintableB(5);
		
	//	nonPrintableA = nonPrintableB;
		
		LT_EQUAL(nonPrintableA, nonPrintableB);
	});
	
	// It is possble to bypass the C macros and use the C++ lambda interface.
	// This results in a lot of boilerplate code.
	// Macros are still needed to get file and line information.
	suite.addTest("Test implemented without macros", [] (litest::TestSuite &ctx)
	{
		std::vector<int> vec;
		
		// Assert that an expression evaluates to true:
		litest::check(ctx, [&] { return vec.empty(); });
		
		vec.push_back(1);
		
		// Assert equality of two expressions:
		litest::equal<size_t>(ctx, 1, [&] { return vec.size(); });
		
		// Assert that an expression throws:
		litest::throws(ctx, [&] { throw "Bad code"; });
		
		// Assert that an expression throws a certain type:
		litest::throwsType<std::out_of_range>(ctx, [&] { vec.at(5); });
		
		// The above assertions does not include much information if they fail.
		// This information can be supplied:
		litest::check(ctx, [&] { return vec.empty(); }, litest::OnAssertionFailure::Continue, "vec.empty", __LINE__);
		
		// TODO: other tests here
	});
	
	// Format output as HTML
	std::ofstream outfile{"litest_example.html"};
	suite.run<litest::TestResultFormatterHTML>(outfile);
	
	// Or Markdown
	suite.run<litest::TestResultFormatterMarkdown<litest::LogLevel::Everything>>(std::cout);
	
	// Or add your own formatter
	suite.run<MyCustomTestResultFormatter>(std::cout);
}
