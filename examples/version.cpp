/*
 * version.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 * 
 * This demo simply displays the version of the bento library.
 */

#include <bento/version.h>
#include <iostream>

using namespace std;

int main()
{
	cout << "Version: " << bento::version() << endl;
}


