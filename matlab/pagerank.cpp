#include "../prpack.h"
#include "mex.h"
#include <algorithm>
#include <string>
using namespace std;
using namespace prpack;

bool is_int_scalar(const mxArray* a) {
    return (mxIsInt32(a) || mxIsInt64(a))
            && mxGetNumberOfElements(a) == 1;
}

bool is_real_scalar(const mxArray* a) {
    return mxIsDouble(a)
            && !mxIsComplex(a)
            && mxGetNumberOfElements(a) == 1;
}

bool is_vector(const mxArray* a) {
    const mwSize* dims = mxGetDimensions(a);
    return mxGetNumberOfDimensions(a) == 2
            && min(dims[0], dims[1]) <= 1;
}

bool is_int_vector(const mxArray* a) {
    return (mxIsInt32(a) || mxIsInt64(a))
            && is_vector(a);
}

bool is_real_vector(const mxArray* a) {
    return mxIsDouble(a)
            && !mxIsComplex(a)
            && is_vector(a);
}

bool is_string(const mxArray* a) {
    return mxIsChar(a)
            && is_vector(a);
}

void mexFunction(
		int nlhs,
		mxArray *plhs[],
		int nrhs,
		const mxArray *prhs[]) {
	// validate number of inputs and outputs
    if (nrhs != 8)
        mexErrMsgTxt("Not enough input arguments.");
	if (nlhs > 1)
		mexErrMsgTxt("Too many output arguments.");
    // set up raw variables
    const mxArray* raw_num_vs = prhs[0];
    const mxArray* raw_heads = prhs[1];
    const mxArray* raw_tails = prhs[2];
    const mxArray* raw_alpha = prhs[3];
    const mxArray* raw_tol = prhs[4];
    const mxArray* raw_u = prhs[5];
    const mxArray* raw_v = prhs[6];
    const mxArray* raw_method = prhs[7];
    // parse num_vs
    if (!is_int_scalar(raw_num_vs))
        mexErrMsgTxt("num_vs must be an int.");
    int num_vs = *((int*) mxGetData(raw_num_vs));
    if (num_vs <= 0)
        mexErrMsgTxt("num_vs must be > 0.");
    // parse heads and tails
    if (!is_int_vector(raw_heads) || !is_int_vector(raw_tails))
        mexErrMsgTxt("heads and tails must be int vectors.");
    if (mxGetNumberOfElements(raw_heads) != mxGetNumberOfElements(raw_tails))
        mexErrMsgTxt("heads and tails must be of the same size.");
    int num_es = (int) mxGetNumberOfElements(raw_heads);
    int* heads = (int*) mxGetData(raw_heads);
    int* tails = (int*) mxGetData(raw_tails);
    // parse alpha and tol
    if (!is_real_scalar(raw_alpha) || !is_real_scalar(raw_tol))
        mexErrMsgTxt("alpha and tol must be real scalars.");
    double alpha = *((double*) mxGetData(raw_alpha));
    double tol = *((double*) mxGetData(raw_tol));
    if (alpha <= 0 || 1 <= alpha)
        mexErrMsgTxt("alpha must be in (0, 1).");
    if (tol <= 0)
        mexErrMsgTxt("tol must be > 0.");
    // parse u and v
    if (!is_real_vector(raw_u) || !is_real_vector(raw_v))
        mexErrMsgTxt("u and v must be real vectors.");
    mwSize u_size = mxGetNumberOfElements(raw_u);
    mwSize v_size = mxGetNumberOfElements(raw_v);
    if ((u_size != 0 && u_size != num_vs) || (v_size != 0 && v_size != num_vs))
        mexErrMsgTxt("u and v must be the same size as the matrix, or empty.");
    double* u = (u_size == 0) ? NULL : (double*) mxGetPr(raw_u);
    double* v = (v_size == 0) ? NULL : (double*) mxGetPr(raw_v);
    // parse method
    if (!is_string(raw_method))
        mexErrMsgTxt("method must be a string");
    mwSize method_length = mxGetNumberOfElements(raw_method);
    char* s = new char[method_length + 1];
    mxGetString(raw_method, s, method_length + 1);
    string method(s);
    delete[] s;
    // compute pagerank
    prpack_edge_list g;
    g.num_vs = num_vs;
    g.num_es = num_es;
    g.heads = heads;
    g.tails = tails;
    prpack_solver solver(&g);
    prpack_result* res = solver.solve(alpha, tol, u, v, method);
    // return vector
    plhs[0] = mxCreateDoubleMatrix(num_vs, 1, mxREAL);
    double* ret = mxGetPr(plhs[0]);
    for (int i = 0; i < num_vs; ++i)
        ret[i] = res->x[i];
    delete res;
}