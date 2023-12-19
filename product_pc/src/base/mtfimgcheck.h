#ifndef MTFIMGCHECK_H
#define MTFIMGCHECK_H


class mtfImgCheck
{
public:
    mtfImgCheck();
    void Get_Y16_hist(unsigned short* input_data, unsigned short&max_value, unsigned short&min_value, int width, int height);

    void Get_target_count(unsigned short* input_data, int* target_count, unsigned short max_value, unsigned short min_value, int width, int height);

};

#endif // MTFIMGCHECK_H
