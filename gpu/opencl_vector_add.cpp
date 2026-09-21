#include <CL/cl.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

static void check(cl_int code, const char* where) {
    if (code != CL_SUCCESS) throw std::runtime_error(std::string(where) + " failed: " + std::to_string(code));
}

int main() {
    cl_uint platform_count = 0;
    check(clGetPlatformIDs(0, nullptr, &platform_count), "clGetPlatformIDs(count)");
    if (!platform_count) throw std::runtime_error("no OpenCL platforms");

    std::vector<cl_platform_id> platforms(platform_count);
    check(clGetPlatformIDs(platform_count, platforms.data(), nullptr), "clGetPlatformIDs");

    cl_device_id device = nullptr;
    for (auto p : platforms) {
        cl_uint n = 0;
        if (clGetDeviceIDs(p, CL_DEVICE_TYPE_GPU, 1, &device, &n) == CL_SUCCESS && n) break;
    }
    if (!device) throw std::runtime_error("no OpenCL GPU device");

    char name[256]{};
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, nullptr);
    std::cout << "OpenCL device: " << name << "\n";

    cl_int err = CL_SUCCESS;
    cl_context ctx = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    check(err, "clCreateContext");
    cl_command_queue q = clCreateCommandQueueWithProperties(ctx, device, nullptr, &err);
    check(err, "clCreateCommandQueueWithProperties");

    const char* src = R"CLC(
      __kernel void vadd(__global const float* a, __global const float* b, __global float* c) {
        size_t i = get_global_id(0);
        c[i] = a[i] + b[i];
      }
    )CLC";

    cl_program program = clCreateProgramWithSource(ctx, 1, &src, nullptr, &err);
    check(err, "clCreateProgramWithSource");
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t n = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &n);
        std::vector<char> log(n);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, n, log.data(), nullptr);
        throw std::runtime_error("OpenCL build failed:\n" + std::string(log.data()));
    }

    cl_kernel kernel = clCreateKernel(program, "vadd", &err);
    check(err, "clCreateKernel");

    constexpr size_t N = 1 << 20;
    std::vector<float> a(N), b(N), c(N);
    for (size_t i=0;i<N;++i) { a[i]=float(i%101); b[i]=float(i%37); }

    cl_mem da = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, N*sizeof(float), a.data(), &err);
    check(err, "clCreateBuffer(a)");
    cl_mem db = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, N*sizeof(float), b.data(), &err);
    check(err, "clCreateBuffer(b)");
    cl_mem dc = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, N*sizeof(float), nullptr, &err);
    check(err, "clCreateBuffer(c)");

    check(clSetKernelArg(kernel,0,sizeof(da),&da), "arg0");
    check(clSetKernelArg(kernel,1,sizeof(db),&db), "arg1");
    check(clSetKernelArg(kernel,2,sizeof(dc),&dc), "arg2");

    size_t global = N;
    check(clEnqueueNDRangeKernel(q,kernel,1,nullptr,&global,nullptr,0,nullptr,nullptr), "enqueue");
    check(clEnqueueReadBuffer(q,dc,CL_TRUE,0,N*sizeof(float),c.data(),0,nullptr,nullptr), "read");

    float max_error=0.0f;
    for(size_t i=0;i<N;++i) max_error=std::max(max_error,std::abs(c[i]-(a[i]+b[i])));
    std::cout << "elements=" << N << " max_abs_error=" << max_error << "\n";

    clReleaseMemObject(da); clReleaseMemObject(db); clReleaseMemObject(dc);
    clReleaseKernel(kernel); clReleaseProgram(program); clReleaseCommandQueue(q); clReleaseContext(ctx);
    return max_error == 0.0f ? 0 : 1;
}
