#include <cmath>
#include <unistd.h>
#include "location.h"

#define PI 3.14159265
using namespace std;

double Microphone_Cita[NODE_NUMBER] = {0, 270, 0, 270, 0, 270, 0, 270};
//double Microphone_Center_Location[NODE_NUMBER][2] = {3, 3, 3, 3, 7, 3, 7, 3, 3, 7, 3, 7, 7, 7, 7, 7};
double Microphone_Center_Location[NODE_NUMBER][2] = {2, 2, 2, 2, 4, 2, 4, 2, 2, 4, 2, 4, 4, 4, 4, 4};

double Microphone_1_Location[NODE_NUMBER][2];
double Microphone_2_Location[NODE_NUMBER][2];

void getMicLocation()
{
	for (int i = 0; i < NODE_NUMBER; i++)
	{
		Microphone_1_Location[i][0] = Microphone_Center_Location[i][0] + 0.5 * MIC_DISTANCE * (cos(Microphone_Cita[i] * PI / 180));
		Microphone_1_Location[i][1] = Microphone_Center_Location[i][1] + 0.5 * MIC_DISTANCE * (-sin(Microphone_Cita[i] * PI / 180));
		Microphone_2_Location[i][0] = Microphone_Center_Location[i][0] - 0.5 * MIC_DISTANCE * (cos(Microphone_Cita[i] * PI / 180));
		Microphone_2_Location[i][1] = Microphone_Center_Location[i][1] - 0.5 * MIC_DISTANCE * (-sin(Microphone_Cita[i] * PI / 180));
        //cout << Microphone_1_Location[i][0] << " " << Microphone_1_Location[i][1] << endl;
	}
}

void tdoa()
{
	int Length, Width;
	Length = (int)AREA_LENGTH;
	Width = (int)AREA_WIDTH;
	double xx = 0, yy = 0;
	double min = 1000;


    //for (int i = 0; i < NODE_NUMBER; i++)
    //{
    //    cout << (measure_data[i] ) << " ";
    //}
    //cout << endl;

	for (int k = 0; k < SCALE * Width; k++)
	{
		for (int j = 0; j < SCALE * Length; j++)
		{
			double x = k / (SCALE * 1.0);
			double y = j / (SCALE * 1.0);
			double data_temp[NODE_NUMBER];
			for (int i = 0; i < NODE_NUMBER; i++)
			{
				data_temp[i] = (sqrt((x - Microphone_1_Location[i][0]) * (x - Microphone_1_Location[i][0]) + (y - Microphone_1_Location[i][1]) * (y - Microphone_1_Location[i][1])) - sqrt((x - Microphone_2_Location[i][0]) * (x - Microphone_2_Location[i][0]) + (y - Microphone_2_Location[i][1]) * (y - Microphone_2_Location[i][1]))) / 340;
			}
			double error = 0;
			for (int i = 0; i < NODE_NUMBER; i++)
			{
                //cout << data_temp[i] << " " << measure_data[i] << endl;
                //cout << data_temp[i] -  measure_data[i] << endl;
                
				long double temp = abs(data_temp[i] - measure_data[i]);
				if (data_flag[i] == 1)
					error = error + temp;
			}
            //cout << endl;
            //cout << "error值:" << error << endl;
			if (error < min)
			{
				min = error;
				xx = x;
				yy = y;
			}
		}
	}
    //cout << "x=" << xx << " y=" << yy << endl;
	result[0] = xx;
	result[1] = yy;
	isAvailable = true;
}

void *calculation(void *ptr)
{
	getMicLocation();

    while (1)
    {
        sleep(1);
        int flag_count = 0;
        for(int i = 0; i < NODE_NUMBER; i++)
        {
            if(data_flag[i] == 1)
                flag_count ++;
        }
        if( flag_count > 3)
            tdoa();
        else
            continue;

		cout << "接收的数据为：" << endl;
        for (int i = 0; i < NODE_NUMBER; i++)
        {
                cout << (int)(measure_data[i] * 44100) << " ";
        }
        cout << endl;

        cout << "结果为：X=" << result[0] << "  Y=" << result[1] << endl << endl;

        for (int i = 0; i < NODE_NUMBER; i++)
        {
            measure_data[i] = 99; 
            data_flag[i] = 0;
        }
    }
    return 0;
}
