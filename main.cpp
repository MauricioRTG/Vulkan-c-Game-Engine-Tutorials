#include "first_app.h"

//std 
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	lve::FirstApp app{};

	try
	{
		app.run();
	}//end try
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}//end catch
}