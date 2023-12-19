#ifndef MtfUtils_H
#define MtfUtils_H
class MtfUtils
{
public:
    MtfUtils();
    ~MtfUtils();

	static void Gethanmingwin(float *hamming, int n_len, float mid);
	static void Getloc(float *loc, float* src_grad, int n_width, int n_height);
	static void findedge(float* Cen_Shifts, int n_len, float& a, float& b);
	static void cent(float* puc_dst, float* puc_src, int n_len, float center);
};

#endif
