// Training.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "Btree.h"

using std::string;
using std::cout;
using std::vector;
using std::endl;
using std::shared_ptr;
using std::make_shared;
using std::ostream;

int main() 
{
	Node root;
	// now is 3-order Btree
	auto b = Btree({ 1, 2, 3, 4, 5, 6, 7, });
	cout << b << endl;
	return 0;
}
