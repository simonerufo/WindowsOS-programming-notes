#include <math.h>

/* 4×4 matrix in column-major order */
typedef float mat4[4][4];

/* Set M to the identity matrix */
static void mat4_identity(mat4 M) {
    memset(M, 0, sizeof(mat4));
    M[0][0] = M[1][1] = M[2][2] = M[3][3] = 1.0f;
}

/* Multiply A * B → out (can alias) */
static void mat4_mul(const mat4 A, const mat4 B, mat4 out) 
{
    mat4 tmp;

    for (int r = 0; r < 4; ++r) 
    {
        for (int c = 0; c < 4; ++c) 
        {
            tmp[r][c] = 0.0f;
            for (int k = 0; k < 4; ++k) 
            {
                tmp[r][c] += A[r][k] * B[k][c];
            }
        }
    }
    memcpy(out, tmp, sizeof(tmp));
}

/* Build a translation matrix T(tx,ty,tz) */
static void mat4_translate(mat4 M, float tx, float ty, float tz) 
{
    mat4_identity(M);

    M[3][0] = tx;
    M[3][1] = ty;
    M[3][2] = tz;
}


/* Build a rotation matrix around axis (x,y,z) by angle radians */
static void mat4_rotate(mat4 M, float angle, float x, float y, float z) 
{
    float c = cosf(angle), s = sinf(angle);
    float len = sqrtf(x*x + y*y + z*z);
    if (len == 0.0f) { mat4_identity(M); return; }
    x/=len; y/=len; z/=len;
    float nc = 1.0f - c;


    M[0][0] = x*x*nc + c;
    M[0][1] = y*x*nc + z*s;
    M[0][2] = x*z*nc - y*s;
    M[0][3] = 0.0f;

    M[1][0] = x*y*nc - z*s;
    M[1][1] = y*y*nc + c;
    M[1][2] = y*z*nc + x*s;
    M[1][3] = 0.0f;

    M[2][0]  = x*z*nc + y*s;
    M[2][1]  = y*z*nc - x*s;
    M[2][2] = z*z*nc + c;
    M[2][3] = 0.0f;

    M[3][0]= M[3][1]= M[3][2]= 0.0f;
    M[3][3]= 1.0f;
}


static void 
mat4_rotate_inplace(mat4 M, float angle, float x, float y, float z) 
{
    float c = cosf(angle);
    float s = sinf(angle);
    float len = sqrtf(x*x + y*y + z*z);
    if (len == 0.0f) return;
    x /= len; y /= len; z /= len;
    float nc = 1.0f - c;

    mat4 R;
    
    R[0][0] = x*x*nc + c;     
    R[0][1] = y*x*nc + z*s; 
    R[0][2] = x*z*nc - y*s; 
    R[0][3] = 0.0f;

    R[1][0] = x*y*nc - z*s;   
    R[1][1] = y*y*nc + c;   
    R[1][2] = y*z*nc + x*s; 
    R[1][3] = 0.0f;

    R[2][0] = x*z*nc + y*s;   
    R[2][1] = y*z*nc - x*s; 
    R[2][2] = z*z*nc + c;   
    R[2][3] = 0.0f;

    R[3][0] = 0.0f;           
    R[3][1] = 0.0f;         
    R[3][2] = 0.0f;         
    R[3][3] = 1.0f;

    mat4 result;
    mat4_mul(M, R, result);

    memcpy(M, result, sizeof(mat4));
}

static void 
mat4_translate_inplace(mat4 M, float tx, float ty, float tz) 
{
    mat4 R;
    mat4_identity(R);

    R[3][0] = tx;
    R[3][1] = ty;
    R[3][2] = tz;

    mat4 result;
    mat4_mul(M, R, result);

    memcpy(M, result, sizeof(mat4));
}


/* Build a perspective projection matrix:
   fovY in radians, aspect = width/height, near>0, far>near */
static void
mat4_perspective(mat4 M,float fovY,float aspect,float near_,float far_)
{
    float f = 1.0f / tanf(fovY * 0.5f);
    mat4_identity(M);

    M[0][0] = f / aspect;
    M[1][1] = f;
    M[2][2] = (far_+near_)/(near_-far_);
    M[2][3] = -1.0f;
    M[3][2] = (2.0f*far_*near_)/(near_-far_);
    M[3][3] = 0.0f;
}


void normalize(float* x, float* y, float* z) 
{
    float len = sqrtf((*x)*(*x) + (*y)*(*y) + (*z)*(*z));
    if (len > 0.0f) 
    {
        *x /= len;
        *y /= len;
        *z /= len;
    }
}


void cross(float ax, float ay, float az,
           float bx, float by, float bz,
           float* rx, float* ry, float* rz) 
{
    *rx = ay * bz - az * by;
    *ry = az * bx - ax * bz;
    *rz = ax * by - ay * bx;
}


float dot(float ax, float ay, float az, float bx, float by, float bz) 
{
    return ax * bx + ay * by + az * bz;
}

/*
    Camera lookAt
*/
void mat4_lookAt(mat4 out,
                 float eyeX, float eyeY, float eyeZ,
                 float centerX, float centerY, float centerZ,
                 float upX, float upY, float upZ) 
{
    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;
    normalize(&fx, &fy, &fz);

    float sx, sy, sz;
    cross(fx, fy, fz, upX, upY, upZ, &sx, &sy, &sz);
    normalize(&sx, &sy, &sz);

    float ux, uy, uz;
    cross(sx, sy, sz, fx, fy, fz, &ux, &uy, &uz);

    memset(out, 0, sizeof(mat4));
    out[0][0] = sx;
    out[1][0] = sy;
    out[2][0] = sz;

    out[0][1] = ux;
    out[1][1] = uy;
    out[2][1] = uz;

    out[0][2] = -fx;
    out[1][2] = -fy;
    out[2][2] = -fz;

    out[3][0] = -dot(sx, sy, sz, eyeX, eyeY, eyeZ);
    out[3][1] = -dot(ux, uy, uz, eyeX, eyeY, eyeZ);
    out[3][2] =  dot(fx, fy, fz, eyeX, eyeY, eyeZ);
    out[3][3] = 1.0f;
}
