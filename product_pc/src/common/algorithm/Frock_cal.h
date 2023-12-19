#ifndef FROCK_CAL_H
#define FROCK_CAL_H

#define UNIFORM_THRESHOLD 80
#define DROP_RATE 0.005
#define BAD_POINT_THRESHOLD 50
#define SFFC_DIFF 20
#define SFFC_DIFF_MAX 500
#define NOISE_THRESHOLD 20
#include <vector>
struct Points
{
	int x;
	int y;
};

class Frock_cal
{
public:
	Frock_cal();
	~Frock_cal();
    bool Check_uniform(unsigned char* src_data, int width, int height, int&high_value, int&low_value);
    bool Check_uniform(unsigned short* src_data, int width, int height, int&high_value, int&low_value);

	int  Check_badpoint(unsigned char* src_data, std::vector<Points>& badPoints, int width, int height);
    int  Check_badpoint(unsigned short* src_data, std::vector<Points>& badPoints, int width, int height);

    bool Check_pic_grad(unsigned char* src_data, int width, int height, float&avg_grad);
    bool Check_pic_grad(unsigned short* src_data, int width, int height, float&avg_grad);

    bool Check_SFFC(unsigned char* first_src, unsigned char* second_src, int width, int height, int &avg_value);
    bool Check_SFFCEx(unsigned short* first_src, unsigned short* second_src, int width, int height, int &avg_value);

private:
	void Get_hist(unsigned char* src_data, float* hist_data, int width, int height);
    void Get_hist(unsigned short* src_data, float* hist_data, int width, int height);
};

#endif
