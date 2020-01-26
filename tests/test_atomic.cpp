/**
 * @file test_safe.cpp
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2019-03-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */


TEST_CASE("Atomic<std::shared_ptr<int>>")
{
	safe::Atomic<std::shared_ptr<int>> atomic(42);

	SUBCASE("readAccess gives access to the int")
	{
		const auto value = **atomic.readAccess();
		CHECK(value == 42);
	}

	SUBCASE("copy then assign does not modify the copy")
	{
		const auto value = atomic.copy();
		atomic.assign(43);
		const auto otherValue = atomic.copy();

		CHECK(*value == 42);
		CHECK(*otherValue == 43);
	}

	SUBCASE("copy then writeAccess does not modify the copy")
 	{
		const auto value = atomic.copy();
		**atomic.writeAccess() = 43;
		const auto otherValue = atomic.copy();

		CHECK(*value == 42);
		CHECK(*otherValue == 43);
	}

	SUBCASE("assign does not reallocate if unique")
	{
		const auto ptr = atomic.copy().get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!
		atomic.assign(43);
		const auto samePtr = atomic.copy().get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!

		CHECK(ptr == samePtr);
	}

	SUBCASE("writeAccess does not reallocate if unique")
	{
		const auto ptr = atomic.copy().get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!
		**atomic.writeAccess() = 43;
		const auto samePtr = atomic.copy().get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!

		CHECK(ptr == samePtr);
	}

	SUBCASE("assign reallocates if not unique")
	{
		const auto value = atomic.copy();
		const auto ptr = value.get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!
		atomic.assign(43);
		const auto otherPtr = atomic.copy().get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!

		CHECK(*value == 42);
		CHECK(ptr != otherPtr);
		CHECK(*otherPtr == 43);
	}

	SUBCASE("writeAccess reallocates if not unique")
	{
		const auto value = atomic.copy();
		const auto ptr = value.get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!
		**atomic.writeAccess() = 43;
		const auto otherPtr = atomic.copy().get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!

		CHECK(*value == 42);
		CHECK(ptr != otherPtr);
		CHECK(*otherPtr == 43);
	}

	SUBCASE("Several copies are the same")
	{
		const auto ptr = atomic.copy().get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!
		const auto value = atomic.copy();
		const auto samePtr = value.get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!
		const auto alsoSamePtr = atomic.copy().get(); // do not do this! never store a raw pointer to the content of a safe::Atomic!

		CHECK(ptr == samePtr);
		CHECK(ptr == alsoSamePtr);
	}
}