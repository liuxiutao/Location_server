#include<fcgi_stdio.h>
#include<string.h>
#include<string>
#include<sstream>
#include<cstdlib>
#include<ctime>
#include<iostream>
using namespace std;

double x = 0.0, y = 0.0;

string doubleToString(double d)
{
	stringstream stream;
	stream << d;
	return stream.str();
}

string getNewPoint()
{
    return "{\"point\":["+doubleToString(x)+","+doubleToString(y)+"]}";
}


int main()
{
    string str;
    char s[100];
    srand((unsigned) time(NULL));
	while(FCGI_Accept() >= 0)
    {
		x = rand()%10;
		y = rand()%10;
		str = getNewPoint();
		strcpy(s, str.c_str());
		printf("Content-Type: text/html\n\n");
		printf("%s\n", s);
    }
    return 0;
}
