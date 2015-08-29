
LiTest ReadMe
-----------------------------

![LiTest logo](litest_logo.png)

*A C++ header framework for testing*

-----------------------------

# Usage

LiTest is simple to integrate in your project. Just include the header, `litest.hpp`, and you are good to go.
The interface is also fairly simple. You build the executable yourself, so there are a few boilerplate lines required to set up and run a test in code though.
More on that later! First read about the assertions, the core of LiTest.


## Assertions

There are multiple types of *assertions*, separated into two groups: *weak* and *strong* assertions.
Weak assertions may fail and the test will still continue. Contrary, if a strong assertion fails,
the test will be aborted.

There are also *pseudo-assertions* which share the syntax and behaviour of assertions but have a statically determined outcome. These can be useful for complex tests where the standard assertions aren't expressive enough.

### Weak Assertions

- `LT_CHECK ( bool_expr )`
- `LT_EQUAL ( expr, value_expr )`
- `LT_THROWS ( expr )`
- `LT_EXCEPT ( expr, type )`

### Strong Assertions

- `LT_REQUIRE ( bool_expr )`
- `LT_EQUAL_REQ ( expr, value_expr )`
- `LT_THROWS_REQ ( expr )`
- `LT_EXCEPT_REQ ( expr, type )`

### Weak Pseudo-Assertions

- `LT_MESSAGE ( message )`
- `LT_PRINT_EXPR( expr )`
- `LT_FAIL ( message )`

### Strong Pseudo-Assertions

- `LT_ABORT ( message )`

Each weak assertion type has a counterpart strong assertion type.

Using the `LT_EQUAL` or `LT_EQUAL_REQ` assertion types places some restrictions on the type of the expressions.
If these requirements are fulfilled these assertions are preferred to `LT_CHECK` and `LT_REQUIRE` (when both are applicable) since more information will be available for debug purposes.

- The type of `exprB` is dominant and chosen as the type to use for the assertion.
- `exprA` has to be convertible to the type of `exprB`.
- This type has to implement operator `==`.

## Testing

Performing a test requires a *test suite*, i.e. an instance of class `litest::TestSuite`.
Chunks of code, called *test*s, are defined using the macro `LT_ADD_TEST ( suite, name, block)`. The test suite is the first argument to the macro, followed by a name for this test and a block statement including the individual *assertions*. Any indentifier defined outside the block statement is available to use.

A test is run with the `litest::TestSuite::run()` member function on test suites. This is a templated function, where the template parameter type should be a subclass of `litest::TestResultFormatter`. An instance of this type will be used to format a *test report*. An `std::ostream &` is passed as parameter, to which the report will be written. The report will include information about passed/failed assertions, messages, statistics etc.

In many cases the value of an assertion will be printed. This requires the type of the assertion (say, `T`) to have a `std::ostream& operator<<(std::ostream&, T)` operator defined, otherwise a placeholder value will be displayed instead.



# Example application

~~~cpp
litest::TestSuite suite("A test suite");

LT_ADD_TEST(suite, "My first test",
{
	std::vector<int> vec;

	// Assert that an expression evaluates to true:
	LT_CHECK(vec.empty());

	LT_MESSAGE("Adding an element to the vector");
	vec.push_back(1);

	// Assert equality of two expressions:
	LT_EQUAL(vec.size(), 1);

	// Assert that an expression throws:
	LT_THROWS(throw "Bad code");

	// Assert that an expression throws a certain type:
	LT_EXCEPT(vec.at(5), std::out_of_range);
});

suite.run<litest::TestResultFormatterMarkdown<>>(std::cout);
~~~

Which results in the following output:

~~~
Test 1: *Tests that pass* in file *mytest.cpp*
------------------------------------------------
- Line 28:	Passed check:  in `vec.empty()`
- Line 30:	Message: Adding an element to the vector.
- Line 34:	Passed equals: `vec.size()` == `1`
- Line 37:	Passed throw:  in `throw "Bad code"`
- Line 40:	Passed throw:  in `vec.at(5)`

**Total passed / failed assertions: 4 / 0**

Summary
------------------------------------------------
**Total passed / failed assertions: 4 / 0**
~~~

# Test Output Formatting

Clear output from the tests are necessary for a seamless workflow. LiTest has two built-in output formats:

- Markdown
- HTML

More can be added in your application by subclassing the `litest::TestResultFormatter` class, editing the
LiTest implementation is not necessary. The base class is initialized with a `std::ostream` reference
which is available in overridden member functions as the member variable `s`.

The following member functions on `litest::TestResultFormatter` are possible (none are required) to override:

- `litest::TestResultFormatter::formatTestHeader()`
...

# Implementation

LiTest is implemented as a C++11 runtime based on template programming and lambda expressions.
On top of the C++11 implementation is a layer of C macros to provide a simpler interface to the user.
The macro implementations uses C++ template metaprogramming techiniques to make the macros
as type-safe as possible. It also employs some compiler macros (`__FILE__` and `__LINE__`)
to supply debugging information.