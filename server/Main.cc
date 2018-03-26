#include <fcgi_stdio.h>
#include <string.h>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include <pthread.h>
#include "location.h"

using namespace std;

double result[2] = {0, 0};
bool isAvailable = false;

//字符串拼接
string doubleToString(double d)
{
	stringstream stream;
	stream << d;
	return stream.str();
}

bool getNewPoint(string& str)
{
	if(isAvailable)
	{
		str = "{\"point\":[" + doubleToString(result[0]) + "," + doubleToString(result[1]) + "]}";
		isAvailable = false;
		return true;
	}		
	else
		return false;
}

void * interfaceTest(void *)
{
    srand((unsigned) time(NULL));
    while(1)
    {
        result[0] = rand()%10;
        result[1] = rand()%10;
        isAvailable = true;
        sleep(1);
    }
}

int main()
{
	string str;
	int err;
	char s[100];
	pthread_t communication_pid;
	pthread_t calculation_pid;

	err = pthread_create(&communication_pid, NULL, communication, NULL);
	if (err)
	{
		cout << "ERROR: 创建通信线程失败，返回值为" << err << endl;
		return EXIT_FAILURE;
	}

	err = pthread_create(&calculation_pid, NULL, calculation, NULL);
	if (err)
	{
		cout << "ERROR: 创建定位线程失败，返回值为" << err << endl;
		return EXIT_FAILURE;
	}

	while (FCGI_Accept() >= 0)
	{
		if(getNewPoint(str))
		{
		    printf("Content-Type: text/html\n\n");
			strcpy(s, str.c_str());
			printf("%s\n", s);
            cout << s << endl;
		}
        else
        {
		    printf("Content-Type: text/html\n\n");
            printf("empty\n");
            cout << "empty" <<endl;
        }
	}

	pthread_join(communication_pid, NULL);
	pthread_join(calculation_pid, NULL);

	return 0;
}
