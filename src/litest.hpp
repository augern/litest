/**
 @file
 @brief The main file of LiTest.
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
 
 The time class represents a moment of time.
 */

#ifndef AUGERN_LITEST_HPP
#define AUGERN_LITEST_HPP

#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <exception>
#include <map>
#include <sstream>
#include <type_traits>
#include <iomanip>
#include <numeric>
#include <chrono>

/**@{*/
/** @name Internal-use macros */

/** The name of the context argument in the closures generated from other LiTest macros. */
#define LITEST_CONTEXT_ARG litest_macro_arg_ctx

/** Parameters to a test function. */
#define LITEST_ARGS litest::TestSuite &LITEST_CONTEXT_ARG

/**
 *Internal* Assert that an expression evanluates to `true`.
 @param expr Expression to be compared to `true`. Must be convertible to `bool`.
 @param onFail Action to take if the assertion fails.
 */
#define LITEST_INTERNAL_CHECK(expr, onFail)\
	static_assert(std::is_convertible<decltype(expr),bool>::value,"Expression is not convertible to bool");\
	litest::check(LITEST_CONTEXT_ARG, [&] { return (expr); }, onFail, #expr, __LINE__)

/**
 *Internal* Assert that an expression evalutates to a particular value.
 @param expr Expression to be compared to `val`. Must be convertible to the type of `val`.
 @param val Value to compare `expr` to.
 @param onFail Action to take if the assertion fails.
 */
#define LITEST_INTERNAL_EQUAL(expr, val, onFail)\
	static_assert(std::is_convertible<std::remove_reference<decltype(expr)>::type, decltype(val)>::value,"Right argument is not convertible to left argument's type");\
	litest::equal<std::remove_cv<decltype(val)>::type>(LITEST_CONTEXT_ARG, val, [&] { return (decltype(val))(expr); }, onFail, #expr, __LINE__)

/**
 *Internal* Assert that an expression throws.
 @param expr Expression
 @param onFail Action to take if the assertion fails.
 */
#define LITEST_INTERNAL_THROWS(expr, onFail)\
	litest::throws(LITEST_CONTEXT_ARG, [&] { (void)(expr); }, onFail, #expr, __LINE__)

/**
 *Internal* Assert that an expression throws a particular type.
 @param expr Expression to be compared to `val`. Must be convertible to the type of `val`.
 @param type Value to compare `expr` to.
 @param onFail Action to take if the assertion fails.
 */
#define LITEST_INTERNAL_THROWTYPE(expr, type, onFail)\
	litest::throws<type>(LITEST_CONTEXT_ARG, [&] { (void)(expr); }, litest::OnAssertionFailure::Continue, #expr, __LINE__)

/**@}*/
/**@{*/
/** @name Public macros */

/**
 Header of a test function.
 @param name Name of the resulting function.
 */
#define LT_TEST(name) void name(LITEST_ARGS)

/**
 Add a test (a statement block) to a TestSuite.
 @param suite TestSuite to add the test to.
 @param name Name of the test (std::string).
 @param block Test body; a compound statement.
 */
#define LT_ADD_TEST(suite, name, block) suite.addTest(name, [&] (LITEST_ARGS) block, __FILE__)

/**
 Assert that an expression evaluates to `true`. Test will **resume** on failure.
 @param expr Expression to evaluate.
 */
#define LT_CHECK(expr) LITEST_INTERNAL_CHECK(expr, litest::OnAssertionFailure::Continue)

/**
 Assert that an expression evaluates to `true`. Test will **abort** on failure.
 @param expr Expression to evaluate.
 */
#define LT_REQUIRE(expr) LITEST_INTERNAL_CHECK(expr, litest::OnAssertionFailure::Abort)

/**
 Assert that an expression evaluates to a certain value. Test will **resume** on failure.
 @param expr Expression to evaluate.
 @param val Value to compare against.
 */
#define LT_EQUAL(expr, val) LITEST_INTERNAL_EQUAL(expr, val, litest::OnAssertionFailure::Continue)

/**
 Assert that an expression evaluates to a certain value. Test will **abort** on failure.
 @param expr Expression to evaluate.
 @param val Value to compare against.
 */
#define LT_EQUAL_REQ(expr, val) LITEST_INTERNAL_EQUAL(expr, val, litest::OnAssertionFailure::Abort)

/**
 Assert that an expression will lead to a `throw`. Test will **resume** on failure.
 @param expr Expression to evaluate.
 */
#define LT_THROWS(expr) LITEST_INTERNAL_THROWS(expr, litest::OnAssertionFailure::Continue)

/**
 Assert that an expression will lead to a `throw`. Test will **abort** on failure.
 @param expr Expression to evaluate.
 */
#define LT_THROWS_REQ(expr) LITEST_INTERNAL_THROWS(expr, litest::OnAssertionFailure::Abort)

/**
 Assert that an expression will lead to a `throw` of a particular type. Test will **resume** on failure.
 @param expr Expression to evaluate.
 @param type Type of instance thrown.
 */
#define LT_EXCEPT(expr, type)\
	litest::throwsType<type>(LITEST_CONTEXT_ARG, [&] { (void)(expr); }, litest::OnAssertionFailure::Continue, #expr, __LINE__)

/**
 Assert that an expression will lead to a `throw` of a particular type. Test will **abort** on failure.
 @param expr Expression to evaluate.
 @param type Type of instance thrown.
 */
#define LT_EXCEPT_REQ(expr, type)\
	litest::throwsType<type>(LITEST_CONTEXT_ARG, [&] { (void)(expr); }, litest::OnAssertionFailure::Abort, #expr, __LINE__)

/**
 Manually generate an assertion failure during a test. Test will **resume** afterwards.
 @param reason Explanation for the failure (std::string).
 */
#define LT_FAIL(reason) litest::generateFailure(LITEST_CONTEXT_ARG, reason, litest::OnAssertionFailure::Continue, __LINE__)

/**
 Manually generate an assertion failure during a test. Test will **abort** afterwards.
 @param reason Explanation for the failure (std::string).
 */
#define LT_ABORT(reason) litest::generateFailure(LITEST_CONTEXT_ARG, reason, litest::OnAssertionFailure::Abort, __LINE__)

/**
 Generate a message during a test.
 @param message Message (std::string).
 */
#define LT_MESSAGE(message) LITEST_CONTEXT_ARG.output->formatMessage(__LINE__, message)

/**
 Print the value of an expression during a test.
 @param expr Expresstion to evaluate and print.
 */
#define LT_PRINT_EXPR(expr) LITEST_CONTEXT_ARG.output->formatExpr(__LINE__, #expr, litest::internal::descriptionIfAvailable(expr))

/**@}*/

/**
 The LiTest namespace.
 */
namespace litest
{
	
	/** Result of a single assertion. */
	enum class AssertionResult
	{
		Passed, /**< The assertion was evaluated and checked successfully. */
		Failed /**< The assertion was not completed successfully. */
	};
	
	/** Action to take after an assertion failure. */
	enum class OnAssertionFailure
	{
		Continue, /**< Continue the test. */
		Abort /**< Abort the test. */
	};
	
	/** Thrown when a assertion fails. */
	class AssertionFailureException : public std::exception
	{
	public:
		
		/**
		 Constructor.
		 @param msg Descriptional message about the assertion failure.
		 */
		explicit AssertionFailureException(std::string msg) noexcept
		: _msg(msg) {}
		
		/**
		 Get a descriptional message about the assertion failure.
		 @return Descriptional message.
		 */
		const char* what() const noexcept
		{
			return _msg.c_str();
		}
		
	private:
		
		/** Exception description. */
		const std::string _msg;
	};
	
	/** Thrown when a test is aborted. */
	class TestAbortException : public std::exception
	{
	public:
		
		/**
		 Constructor.
		 @param line Line number of test abortion.
		 @param msg Exception message.
		 */
		explicit TestAbortException(int line, std::string msg) noexcept
		: _msg(msg), lineNumber(line) {}
		
		/** Line number where the test was aborted. */
		const int lineNumber;
		
		/**
		 Get a descriptional message about the assertion failure.
		 @return Descriptional message.
		 */
		const char* what() const noexcept
		{
			return _msg.c_str();
		}
		
	private:
		const std::string _msg;
	};
	
	/** Statistics of several assertions in a Test. */
	struct TestStats
	{
		/** Number of passed assertions in the Test. */
		int passes = 0;
		
		/** Number of failed assertions in the Test. */
		int fails = 0;
	};
	
	class TestSuite;
	
	/**
	 Type of a test function object.
	 A test function is a callable object with no return value, taking a single TestSuite parameter.
	 */
	using TestFunc = std::function<void(TestSuite &)>;
	
	/** Type used for time points. */
	using TimeType = std::chrono::time_point<std::chrono::system_clock>;
	
	/** Type used for calculating test running times. */
	using TimeTypeHiRes = std::chrono::time_point<std::chrono::high_resolution_clock>;
	
	/**
	 A test.
	 */
	struct Test
	{
		/**
		 Test constructor.
		 @param pfile File name.
		 @param pname Test name.
		 @param pfunc Test function.
		 @param ind Test index.
		 */
		Test(std::string pfile, std::string pname, TestFunc pfunc, int ind)
		: file(pfile), name(pname), func(pfunc), index(ind) {}
		
		/** File name of the file this test was defined in. */
		std::string file;
		
		/** Name of this test. */
		std::string name;
		
		/** The test function (payload) of this test. */
		TestFunc func;
		
		/** Index of this test in it's TestSuite. */
		int index;
		
		/** Whether this test was aborted. */
		bool aborted = false;
		
		/**
		 Time taken to run this test, in seconds.
		 Invalid if aborted is true.
		 */
		double duration;
	};
	
	// --------------------------
	// Output formatting
	// --------------------------
	
	/** Namespace for internal LiTest functions. */
	namespace internal
	{
		/**
		 Generates a 'not available' description for a value (used in a LiTest assertion).
		 Overload resolution fallback for types used in LiTest assertions that does not
		 implement any of the overloaded mechanisms for printing values.
		 @param ... The value to describe. Elipsis parameter used for a weak overload precedence.
		 @return A string representing 'not available'
		 */
		std::string descriptionIfAvailable(...)
		{
			return "N/A";
		}
		
		/**
		 Generates a proper description for a value (used in a LiTest assertion).
		 This overload prints by appyling operator<< with a std::stringstream.
		 @tparam T Type of the value to describe.
		 @param val Value to describe.
		 @return A string description of val.
		 */
		template<typename T>
		decltype(std::declval<std::stringstream&>() << std::declval<T>(), std::string())
		descriptionIfAvailable(T &val)
		{
			std::stringstream vals;
			vals << val;
			return vals.str();
		}
		
		/**
		 Generates a proper description for a value (used in a LiTest assertion).
		 For iterable containers, by std::copy():ing the contents of the containter to a std::stringstream with
		 std::ostream_iterator.
		 @tparam T Type of the value to describe.
		 @param val Value to describe.
		 @return A string description of val.
		 */
		template<typename T>
		decltype(begin(std::declval<T>()), std::string())
		descriptionIfAvailable(T const& val)
		{
			std::stringstream vals;
			std::copy(begin(val), end(val), std::ostream_iterator<typename T::value_type> { vals, ", " });
			return "{ " + vals.str() + " }";
		}
	}
	
	/**
	 Astract class for formatting of test result output.
	 
	 A class to be overridden, not instanced.
	 Derived classes implement a specific output format; this root class
	 provies stubs of virtual member functions for the stages in the test life cycle
	 (like formatTestHeader()) and for test events (like formatPassedCheck()).
	 */
	class TestResultFormatter
	{
	public:
		
		/**
		 Constructor.
		 @param ostr Output stream to be used by this instance.
		 */
		TestResultFormatter(std::ostream &ostr)
		: s(ostr)
		{}
		
		/**
		 Pure virtual destructor.
		 Ensures that TestResultFormatter is an abstract class.
		 */
		virtual ~TestResultFormatter() = 0;
		
		// For entire tests
		
		/**@{*/
		/** @name Test lifecycle functions */
		
		/**
		 Called at the start of a test run with information about the test.
		 Does nothing unless overridden.
		 @param test Test to be run.
		 */
		virtual void formatTestHeader(Test const& test) {}
		
		/**
		 Called at the end of a test run with statistics from the run.
		 Does nothing unless overridden.
		 @param test The running test.
		 @param stats TestStats from the current test run.
		 */
		virtual void formatTestFooter(Test const& test, TestStats stats) {}
		
		/**
		 Called when a test is aborted.
		 Does nothing unless overridden.
		 @param line The source line number where the test was aborted.
		 @param reason Reason for the abortion.
		 */
		virtual void formatAbortedTest(int line, std::string reason) {}
		
		/**
		 Called when an entire TestSuite starts to run.
		 Does nothing unless overridden.
		 @param suite Current test suite.
		 */
		virtual void formatTestSuiteStart(TestSuite const& suite) {}
		
		/**
		 Called at the end of a test suite.
		 Does nothing unless overridden.
		 @param suite Current test suite.
		 */
		virtual void formatTestSuiteEnd(TestSuite const& suite) {}
		
		/**@}*/
		/**@{*/
		/** @name Test event functions */
		
		/**
		 Called when a check assertion passed.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param expr String representation of the expression in the assertion.
		 */
		virtual void formatPassedCheck(int line, std::string expr) {}
		
		/**
		 Called when a throw assertion passed.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param expr String representation of the expression in the assertion.
		 */
		virtual void formatPassedThrow(int line, std::string expr) {}
		
		/**
		 Called when an equals assertion passed.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param expr String representation of the expression in the assertion.
		 @param val Value of asserted expression.
		 */
		virtual void formatPassedEquals(int line, std::string expr, std::string val) {}
		
		/**
		 Called when a message should be written to output.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param message Message to log.
		 */
		virtual void formatMessage(int line, std::string message) {}
		
		/**
		 Called when an expression should be logged to output.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param exprstr String representation of the expression in the assertion.
		 @param valstr String representation of the value of the expression.
		 */
		virtual void formatExpr(int line, std::string exprstr, std::string valstr) {}
		
		/**
		 Called when an unexpected exception was caught during an assertion.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param expr String representation of the expression in the assertion.
		 @param msg @optional Message from the exception, if any.
		 */
		virtual void formatUnexpectedException(int line, std::string expr, std::string msg = "N/A") {}
		
		/**
		 Called when a check assertion failed.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param expr String representation of the expression in the assertion.
		 */
		virtual void formatFailedCheck(int line, std::string expr) {}
		
		/**
		 Called when an equal assertion failed.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param expr String representation of the expression in the assertion.
		 @param val String representation of the expected value.
		 @param res String representation of the actual value.
		 */
		virtual void formatFailedEquals(int line, std::string expr, std::string val, std::string res) {}
		
		/**
		 Called when a throw assertion failed.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param expr String representation of the expression in the assertion.
		 */
		virtual void formatFailedThrow(int line, std::string expr) {}
		
		/**
		 Called when a manual failure was encountered.
		 Does nothing unless overridden.
		 @param line Line number where the assertion was defined.
		 @param reason Description of the reason for a failure.
		 */
		virtual void formatManualFailure(int line, std::string reason) {}
		
		/**@}*/
		
		/**
		 Called when a check assertion passed.
		 Does nothing unless overridden.
		 Note: Not possible to have virtual template member functions yet
		 
		 @param line Line number where the assertion was defined.
		 @param expr String representation of the expression in the assertion.
		 @param val Expected value.
		 @param res Actual value.
		 */
		template<typename T>
		inline void formatFailedEquals(int line, std::string expr, T val, T res)
		{
			this->formatFailedEquals(line, expr, internal::descriptionIfAvailable(val), internal::descriptionIfAvailable(res));
		}
		
		/**
		 Format line number.
		 @param line Line number or 0 for unknown line number.
		 @return Formatted line number or string representing unknown line number.
		 */
		inline virtual std::string lineNr(int line)
		{
			if (line > 0)
			{
				std::stringstream ss;
				ss << line;
				return ss.str();
			}
			else return "???";
		}
		
	protected:
		
		/** Output stream used by this formatter. */
		std::ostream &s;
	};
	
	/** A collection of tests. */
	class TestSuite
	{
	public:
		
		/** Behaviour of a test suite when a assertion fails. */
		enum class Mode
		{
			Continue, /**< The test will continue without interruption. */
			Throw /**< AssertionFailureException will be thrown. (Useful for debugging.) */
		};
		
		/** Constructor.
		 @param name Identifying name for this suite.
		 */
		TestSuite(std::string name)
		: suiteName(name) {}
		
		/**
		 Add a test to this TestSuite.
		 @param name Short description of the test.
		 @param func Payload of the test.
		 @param file @optional File name where the test was defined.
		 */
		inline void addTest(std::string name, TestFunc func, std::string file = "N/A")
		{
			this->tests.emplace_back(file, name, func, this->tests.size()+1);
		}
		
		/**
		 Runs the Test s in this TestSuite.
		 @tparam TestResultFormatterType The formatter type to use for output. Must be a subclass of TestResultFormatter.
		 @param out Stream to direct the output to.
		 @param testIdx Indexes of the tests to run.
		 @param mode @optional Action to take if an assertion fails.
		 */
		template<typename TestResultFormatterType>
		inline void runSome(std::ostream &out, std::vector<int> testIdx, Mode mode = Mode::Continue)
		{
			this->mode = mode;
			this->output = new TestResultFormatterType(out);
			this->totalStats_ = TestStats();
			
			auto startTime = TimeType::clock::now();
			this->output->formatTestSuiteStart(*this);
			
			// Run each test in turn
			for (int index : testIdx)
			{
				if (!(index >= 0 && index < (int)this->tests.size())) continue;
				auto test = this->tests[index];
				
				this->startTest();
				this->output->formatTestHeader(test);
				
				try
				{
					// Run the test
					auto testStartTime = TimeTypeHiRes::clock::now();
					test.func(*this);
					auto endTime = TimeTypeHiRes::clock::now();
					test.duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - testStartTime).count() / 1e6;
				}
				catch (TestAbortException &e)
				{
					test.aborted = true;
					output->formatAbortedTest(e.lineNumber, e.what());
				}
				catch (std::exception &e)
				{
					test.aborted = true;
					output->formatAbortedTest(0, "Uncaught exception: " + std::string{e.what()});
				}
				catch (...)
				{
					test.aborted = true;
					output->formatAbortedTest(0, "Uncaught exception outside of assertion.");
				}
				
				this->output->formatTestFooter(test, this->currentTestStats());
			}
			
			this->endTime = TimeType::clock::now();
			this->duration = std::chrono::duration_cast<std::chrono::microseconds>(this->endTime - startTime).count() / 1e6;
			this->output->formatTestSuiteEnd(*this);
			delete output;
		}
		
		/**
		 Runs the Test s in this TestSuite.
		 @tparam TestResultFormatterType The formatter type to use for output. Must be a subclass of TestResultFormatter.
		 @param out Stream to direct the output to.
		 @param mode @optional Action to take if an assertion fails.
		 */
		template<typename TestResultFormatterType>
		inline void run(std::ostream &out, Mode mode = Mode::Continue)
		{
			std::vector<int> allIndexes(this->tests.size());
			iota(allIndexes.begin(), allIndexes.end(), 0);
			this->runSome<TestResultFormatterType>(out, allIndexes, mode);
		}
		
		/**
		 Start a new test.
		 Saves the previous TestStats and prepares for a new test.
		 */
		inline void startTest()
		{
			counter++;
			this->stats_.push_back({});
		}
		
		/**
		 Register a passed assertion.
		 Updates current and total TestStats.
		 @return AssertionResult::Passed.
		 */
		inline AssertionResult passed()
		{
			this->totalStats_.passes++;
			this->stats_[counter].passes++;
			return AssertionResult::Passed;
		}
		
		/**
		 Register a failed assertion.
		 Updates current and total TestStats.
		 @return AssertionResult::Failed.
		 */
		inline AssertionResult failed()
		{
			this->totalStats_.fails++;
			this->stats_[counter].fails++;
			return AssertionResult::Failed;
		}
		
		/**
		 Get the stats of the current test.
		 @return Current TestStats.
		 */
		inline TestStats currentTestStats() const
		{
			return this->stats_[counter];
		}
		
		/**
		 Get the total stats of this TestSuite.
		 @return Total TestStats.
		 */
		inline TestStats totalTestStats() const
		{
			return this->totalStats_;
		}
		
		/** Output formatter used by this TestSuite. */
		TestResultFormatter *output;
		
		/** The action to take for this TestSuite if an assertion fails. */
		Mode mode;
		
		/** Descriptive name of this test suite. */
		std::string suiteName;
		
		/** Ordered list of the tests to run. */
		std::vector<Test> tests;
		
		/** Time point when this test suite completed. */
		TimeType endTime;
		
		/** Time taken to run this test suite, in seconds. */
		double duration;
		
	private:
		
		/** Test counter */
		int counter = -1;
		
		/** Ordered list of test stats. */
		std::vector<TestStats> stats_;
		
		/** Total stats. */
		TestStats totalStats_;
	};

	
#pragma mark - Free Functions for Testing
	
	/**
	 Helper function.
	 For the common task of reporting an assertion failure caused by a throw.
	 @param suite TestSuite used as context.
	 @param line @optional The line number where this assertion was defined.
	 @param exprstr @optional A string representation of the tested code.
	 @param msg Exception message.
	 @param onFail @optional Action to take after reporting.
	 @return Failed assertion result.
	 */
	AssertionResult reportException(TestSuite &suite, int line, std::string exprstr, std::string msg, OnAssertionFailure onFail = OnAssertionFailure::Abort)
	{
		suite.failed();
		suite.output->formatUnexpectedException(line, exprstr, msg);
		if (suite.mode == TestSuite::Mode::Throw) throw AssertionFailureException("Unexpected exception in: " + exprstr);
		if (onFail == OnAssertionFailure::Abort) throw TestAbortException(line, "Caught in assertion");
		return AssertionResult::Failed;
	}
	
	/**
	 Asserts that the return value of a lambda expression is equal to a particular value.
	 
	 @tparam T Type of values to compare. Must implement `operator ==()`.
	 
	 @param suite TestSuite used as context.
	 @param val Expected value.
	 @param func Function object wrapping the code, whose return value is compared.
	 @param onFail @optional Action to take if the values are not equal.
	 @param exprstr @optional A string representation of the tested code.
	 @param line @optional The line number where this assertion was defined.
	 
	 @throws AssertionFailureException
	 @throws TestAbortException
	 
	 @return Result of the assertion.
	 */
	template<typename T>
	inline AssertionResult equal(TestSuite &suite, T val, std::function<T(void)> func, OnAssertionFailure onFail = OnAssertionFailure::Continue, std::string exprstr = "N/A", int line = 0)
	{
		try
		{
			T res = func();
			if (!(res == val))
			{
				suite.failed();
				suite.output->formatFailedEquals(line, exprstr, val, res);
				if (suite.mode == TestSuite::Mode::Throw) throw AssertionFailureException("Unexpected value in: " + exprstr);
				if (onFail == OnAssertionFailure::Abort) throw TestAbortException(line, "Equal failed.");
				return AssertionResult::Failed;
			}
			else
			{
				suite.passed();
				suite.output->formatPassedEquals(line, exprstr, internal::descriptionIfAvailable(val));
				return AssertionResult::Passed;
			}
		}
		catch (std::exception &e) { return reportException(suite, line, exprstr, e.what()); }
		catch (...) { return reportException(suite, line, exprstr, "N/A"); }
	}
	
	/**
	 Asserts that the return value of a lambda expression is equal to `true`.
	 
	 @param suite TestSuite used as context.
	 @param func Function object wrapping the code, returning a `bool`.
	 @param onFail @optional Action to take if the return value is `false`.
	 @param exprstr @optional A string representation of the tested code.
	 @param line @optional The line number where this assertion was defined.
	 
	 @throws AssertionFailureException
	 @throws TestAbortException
	 
	 @return Result of the assertion.
	 */
	inline AssertionResult check(TestSuite &suite, std::function<bool(void)> func, OnAssertionFailure onFail = OnAssertionFailure::Continue, std::string exprstr = "N/A", int line = 0)
	{
		bool res;
		try { res = func(); }
		catch (std::exception &e) { return reportException(suite, line, exprstr, e.what()); }
		catch (...) { return reportException(suite, line, exprstr, "N/A"); }
		
		if (!res)
		{
			suite.failed();
			suite.output->formatFailedCheck(line, exprstr);
			if (suite.mode == TestSuite::Mode::Throw) throw AssertionFailureException("Broken assertion in: " + exprstr);
			if (onFail == OnAssertionFailure::Abort) throw TestAbortException(line, "Check failed.");
			return AssertionResult::Failed;
		}
		else
		{
			suite.passed();
			suite.output->formatPassedCheck(line, exprstr);
			return AssertionResult::Passed;
		}
	}
	
	/**
	 Asserts that some code throws of a particular type.
	 
	 @tparam ThrownType Type of instance thrown.
	 
	 @param suite TestSuite used as context.
	 @param func Function object wrapping the code, whose return value is compared.
	 @param onFail @optional Action to take if `func` did not throw, or threw of another type.
	 @param exprstr @optional A string representation of the tested code.
	 @param line @optional The line number where this assertion was defined.
	 
	 @throws AssertionFailureException
	 @throws TestAbortException
	 
	 @return Result of the assertion.
	 */
	template<typename ThrownType>
	inline AssertionResult throwsType(TestSuite &suite, std::function<void(void)> func, OnAssertionFailure onFail = OnAssertionFailure::Continue, std::string exprstr = "N/A", int line = 0)
	{
		try { func(); }
		catch (ThrownType &e)
		{
			suite.passed();
			suite.output->formatPassedThrow(line, exprstr);
			return AssertionResult::Passed;
		}
		catch (std::exception &e) { return reportException(suite, line, exprstr, e.what()); }
		catch (...) { return reportException(suite, line, exprstr, "Uncaught exception in exception assertion"); }
		
		suite.failed();
		suite.output->formatFailedThrow(line, exprstr);
		if (suite.mode == TestSuite::Mode::Throw) throw AssertionFailureException("No exception in " + exprstr);
		if (onFail == OnAssertionFailure::Abort) throw TestAbortException(line, "No exception in throw assertion.");
		return AssertionResult::Failed;
	}
	
	/**
	 Asserts that some code throws.
	 
	 @param suite TestSuite used as context.
	 @param func Function object wrapping the code, whose return value is compared.
	 @param onFail @optional Action to take if `func` did not throw.
	 @param exprstr @optional A string representation of the tested code.
	 @param line @optional The line number where this assertion was defined.
	 
	 @throws AssertionFailureException
	 @throws TestAbortException
	 
	 @return Result of the assertion.
	 */
	inline AssertionResult throws(TestSuite &suite, std::function<void(void)> func, OnAssertionFailure onFail = OnAssertionFailure::Continue, std::string exprstr = "N/A", int line = 0)
	{
		try { func(); }
		catch (...)
		{
			suite.passed();
			suite.output->formatPassedThrow(line, exprstr);
			return AssertionResult::Passed;
		}
		suite.failed();
		suite.output->formatFailedThrow(line, exprstr);
		if (suite.mode == TestSuite::Mode::Throw) throw AssertionFailureException("No exception in: " + exprstr);
		if (onFail == OnAssertionFailure::Abort) throw TestAbortException(line, "No exception in throw assertion.");
		return AssertionResult::Failed;
	}
	
	/**
	 Manually generates a failure.
	 
	 @param suite TestSuite used as context.
	 @param reason Reason for the failure.
	 @param onFail @optional Action to take.
	 @param line @optional The line number where this assertion was defined.
	 
	 @throws AssertionFailureException
	 @throws TestAbortException
	 
	 @return Result of the assertion.
	 */
	inline AssertionResult generateFailure(TestSuite &suite, std::string reason, OnAssertionFailure onFail = OnAssertionFailure::Continue, int line = 0)
	{
		suite.failed();
		suite.output->formatManualFailure(line, reason);
		if (suite.mode == TestSuite::Mode::Throw) throw AssertionFailureException("Manual failure, reason: " + reason);
		if (onFail == OnAssertionFailure::Abort) throw TestAbortException(line, "Manual failure");
		return AssertionResult::Failed;
	}
	
#pragma mark - Result Formatter
	
	/** Destructor. Does nothing. */
	TestResultFormatter::~TestResultFormatter() {}
	
	

#pragma mark - Markdown Formatter
	
	/** Specifies what the formatter will log during a test. */
	enum class LogLevel : int
	{
		Errors = 1, /**< Only log errors (failed assertions, aborted tests). */
		Messages = 2, /**< Also log messages. */
		Everything = 3 /**< Also log passed assertions. */
	};
	
	/** Class for formatting of test result output in a Markdown format.
	 @tparam level @optional Log level. Defaults to errors and messages.
	 */
	template<LogLevel level = LogLevel::Messages>
	class TestResultFormatterMarkdown : public TestResultFormatter
	{
	public:
		
		/** Whether messages should be logged; shorthand for comparing the log level. */
		static constexpr bool logMesages = level >= LogLevel::Messages;
		
		/** Whether passes should be logged; shorthand for comparing the log level. */
		static constexpr bool logPasses = level >= LogLevel::Everything;
		
		/**
		 Constructor.
		 @param ostr Output stream to write the Markdown formatted output to.
		 */
		TestResultFormatterMarkdown<level>(std::ostream &ostr)
		: TestResultFormatter(ostr) {}
		
		/**
		 Called at the start of a test run. Writes test information followed by a dashed line.
		 @param test The test to be run.
		 */
		inline void formatTestHeader(Test const& test) override
		{
			s << std::endl << " Test " << test.index << ": *" << test.name << "* in file *" << test.file << "*" << std::endl;
			s << "------------------------------------------------" << std::endl;
		}
		
		inline void formatTestFooter(Test const& test, TestStats stats) override
		{
			s << std::endl << "**Total passed / failed assertions: " << stats.passes << " / " << stats.fails << "**" << std::endl;
		}
		
		inline void formatTestSuiteEnd(TestSuite const& suite) override
		{
			s << std::endl << " Summary" << std::endl;
			s << "------------------------------------------------" << std::endl;
			s << "**Total passed / failed assertions: " << suite.totalTestStats().passes << " / " << suite.totalTestStats().fails <<  "**"  << std::endl << std::endl;
		}
		
		inline void formatAbortedTest(int line, std::string reason) override
		{
			s << "- " << lineNr(line) << ":\t**Test aborted: " << reason << "**"  << std::endl;
		}
		
		inline void formatPassedCheck(int line, std::string expr) override
		{
			if (logPasses) s << "- " << lineNr(line) << ":\tPassed check: " << " in `" << expr << "`" << std::endl;
		}
		
		inline void formatPassedThrow(int line, std::string expr) override
		{
			if (logPasses) s << "- " << lineNr(line) << ":\tPassed throw: " << " in `" << expr << "`" << std::endl;
		}
		
		inline void formatPassedEquals(int line, std::string expr, std::string val) override
		{
			if (logPasses) s << "- " << lineNr(line) << ":\tPassed equals: `" << expr << "` == `" << val << "`" << std::endl;
		}
		
		inline void formatMessage(int line, std::string message) override
		{
			if (logMesages) s << "- " << lineNr(line) << ":\t" << message << "."  << std::endl;
		}
			
		inline void formatExpr(int line, std::string exprstr, std::string valstr)
		{
			if (logMesages) s << "- " << lineNr(line) << ":\t`" << exprstr << "` evaluates to `" << valstr << "`." << std::endl;
		}
		
		inline void formatUnexpectedException(int line, std::string expr, std::string msg) override
		{
			s << "- " << lineNr(line) << ":\tException was caught: " << msg << " in `" << expr << "`" << std::endl;
		}
		
		inline void formatFailedCheck(int line, std::string expr) override
		{
			s << "- " << lineNr(line) << ":\tAssertion failed: `" << expr << "`" << std::endl;
		}
		
		inline void formatFailedThrow(int line, std::string expr) override
		{
			s << "- " << lineNr(line) << ":\tExpected exception: `" << expr << "`" << std::endl;
		}
		
		inline void formatFailedEquals(int line, std::string expr, std::string val, std::string res) override
		{
			s << "- " << lineNr(line) << ":\tEquals failed: `" << expr << "` != `" <<  val << "` (got `" << res << "`)" << std::endl;
		}
		
		inline void formatManualFailure(int line, std::string reason) override
		{
			s << "- " << lineNr(line) << ":\tManual failure, reason: '" << reason << "'" << std::endl;
		}
			
		inline std::string lineNr(int line)
		{
			return "Line " + TestResultFormatter::lineNr(line);
		}
		
	};
	
	
#pragma mark - HTML Formatter
	
	/** Class for formatting of test result output in a HTML format, styled with inline CSS. */
	class TestResultFormatterHTML : public TestResultFormatter
	{
		public:
		
		/**
		 Constructor.
		 @param ostr Output stream to write the HTML formatted output to.
		 */
		TestResultFormatterHTML(std::ostream &ostr)
		: TestResultFormatter(ostr) {}
		
		inline void formatTestHeader(Test const& test)
		{
			s << "<div class='test' id='test" << test.index << "'>";
			s << "<h2 id='test-" << test.index << "-header'> Test " << test.index << ": <span class='test-title'>" << test.name << "</span></h2>";
			s << "<p>In file <span class='test-file'><a href='file://" << test.file << "'>" << test.file << "</a></span></p>";
			s << "<div class='output'>";
		}
		
		inline void formatTestFooter(Test const& test, TestStats stats) override
		{
			std::string clas;
			std::string annotation;
			if (test.aborted) { clas = "aborted"; annotation = "╳"; }
			else if (stats.fails == 0) { clas = "passed"; annotation = "✓"; }
			else { clas = "failed"; annotation = "×"; }
			
			s << "<script type='text/javascript'>document.getElementById('test-" << test.index <<"-header').classList.add('" << clas << "');</script>";
			s << "</div><div class='result-badge'>" << annotation << "</div></div>";
		}
		
		inline void formatAbortedTest(int line, std::string reason) override
		{
			s << "<div class='log-item abort'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "↳ Test aborted: <span class='abort-msg'>" << reason << "</span></div>";
		}
		
		inline void formatMessage(int line, std::string message) override
		{
			s << "<div class='log-item message'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "<span class='msg-txt'>" << message << "</em></div>";
		}
		
		inline void formatExpr(int line, std::string exprstr, std::string valstr)
		{
			s << "<div class='log-item message'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Print expression <code>" << exprstr << "</code>: <code>" << valstr << "</code></div>";
		}
		
		inline void formatPassedCheck(int line, std::string expr) override
		{
			s << "<div class='log-item pass check'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Passed check: <code>" << expr << "</code></div>";
		}
		
		inline void formatPassedThrow(int line, std::string expr) override
		{
			s << "<div class='log-item pass throw'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Passed throw check: <code>" << expr << "</code></div>";
		}
		
		inline void formatPassedEquals(int line, std::string expr, std::string val) override
		{
			s << "<div class='log-item pass equals'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Passed equals: <code>" << expr << "</code> == <code>" << val << "</code></div>";
		}
		
		inline void formatUnexpectedException(int line, std::string expr, std::string msg) override
		{
			s << "<div class='log-item fail unexpected-exception'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Caught exception: <em>" << msg << "</em> in: <code>" << expr << "</code></div>";
		}
		
		inline void formatFailedCheck(int line, std::string expr) override
		{
			s << "<div class='log-item fail broken-assertion'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Failed check: <code>" << expr << "</code></div>";
		}
		
		inline void formatFailedThrow(int line, std::string expr) override
		{
			s << "<div class='log-item fail no-exception'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Expected exception: <code>" << expr << "</code></div>";
		}
		
		inline void formatFailedEquals(int line, std::string expr, std::string val, std::string res) override
		{
			s << "<div class='log-item fail unexpected-value'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Failed equals: <code>" << expr << "</code> != <code>" << val << "</code>, got <code>" << res << "</code></div>";
		}
		
		inline void formatManualFailure(int line, std::string reason) override
		{
			s << "<div class='log-item fail manual'><span class='line-nr'>" << lineNr(line) << "</span>";
			s << "Manual failure: <em>" << reason << "</em></div>";
		}
		
		inline void formatTestSuiteStart(TestSuite const& suite) override
		{
			std::time_t genTime = std::time(nullptr);
			
			s <<
			"<!doctype html><head><style type='text/css'>\
				body {\
					font-family: 'Helvetica', sans-serif;\
					max-width: 900px; margin: auto;\
					background-color: #555; padding: 1em; }\
				h1 { text-align: center; border-bottom: 2px dashed black; }\
				div.test { position: relative; margin: 4em 0; }\
				div#content { padding: 1em 2em; background-color: #eee; box-shadow: 0px 0px 5px #333; }\
				div.output:not(:empty) { margin: 1em; border: 2pt solid #999; }\
				h2 { font-size: 15pt; padding: 0.2em; border-bottom: 2pt solid grey; color: white; }\
				div.log-item:after {\
					display: inline-block; width: 2em; text-align: center;\
					float: right; border-left: 2pt solid grey; }\
				div.fail:after { content: '×'; background-color: darkred; color: white; }\
				div.message:after { content: '!'; background-color: yellow; color: black; }\
				div.abort:after { content: '╳'; background-color: black; color: white; }\
				div.pass:after { content: '✓'; background-color: green; color: white;}\
				div.pass { color: darkgreen; }\
				div.failure { color: darkred; }\
				span.abort-msg { background-color: black; color: white; }\
				code {\
					background-color: darkgreen; color: white;\
					padding: 0.1em 0.5em; border-radius: 0.5em; }\
				.log-item.fail code { background-color: darkred;}\
				.log-item.message code { color: black; background-color: yellow;}\
				.line-nr {\
					color: black; background-color: rgb(200, 200, 200);\
					border-right: 3px solid #999;\
					padding-right: 0.5em; width: 3em; margin-right: 1em;\
					text-align: right; font-family: monospace;\
					display: inline-block; }\
				.log-item { line-height: 22pt; }\
				.code { white-space: pre; }\
				.log-item:nth-child(even) { background-color: white; }\
				.log-item:nth-child(odd) { background-color: #eee; }\
				.result-badge { position: absolute; top: 0; right: 0; padding: 0.5em; color: white; }\
				h2.passed { background-color: darkgreen; }\
				h2.failed { background-color: darkred; }\
				h2 { background-color: black; }\
			</style></head><body><div id='content'>\
			<h1>" << suite.suiteName << "</h1>\
			<p>Generated by LiTest at <time>" << std::put_time(std::localtime(&genTime), "%F %T") << "</time>.</p>\
			<script type='text/javascript'>\
			var pass = document.getElementsByClassName('pass'); var passVisible = true;\
			var mess = document.getElementsByClassName('message'); var messagesVisible = true;\
			</script>\
			<button onclick=\"for (var i = 0; i < pass.length; i++) pass[i].style.display = passVisible ? \'none\' : \'block\'; passVisible = !passVisible;\">Toggle passes</button>\
			<button onclick=\"for (var i = 0; i < mess.length; i++) mess[i].style.display = messagesVisible ? \'none\' : \'block\'; messagesVisible = !messagesVisible;\">Toggle messages</button>";
		}
		
		inline void formatTestSuiteEnd(TestSuite const& suite) override
		{
			float prc = (float)suite.totalTestStats().passes / (suite.totalTestStats().passes + suite.totalTestStats().fails) * 100;
			s << "<h2>Summary</h2>";
			s << "<p>Total passed assertions: " << suite.totalTestStats().passes << "</p>";
			s << "<p>Total failed assertions: " << suite.totalTestStats().fails <<  "</p>";
			s << "<p>Success rate: " << prc << "%</p>";
			s << "</div></body>";
		}
	
	};

}

#endif // AUGERN_LITEST_HPP