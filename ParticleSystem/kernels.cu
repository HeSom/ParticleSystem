#include "kernels.h"
#include "cuda.h"
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_functions.h>
#include <device_launch_parameters.h>
#include <device_atomic_functions.h>
#include "math_constants.h"
#include "math_functions.h"
#include "stdio.h"

#define THREADS_PER_BLOCK_DIM 256
#define UNIFORM_GRID_MIN 0.0f
#define UNIFORM_GRID_MAX 4.0f
#define PARTICLES_PER_CELL 32
#define CELL_SIZE 0.2f
#define SMOOTHING_KERNEL_RADIUS 0.2f
#define GAS_CONSTANT 0.1
#define RHO_0 1000
#define EPSILON 0.005
#define MU 0.01f

bool first = true;
cudaGraphicsResource* vbo_resource;
int* uniformGrid;
int* particlesInCell;
int gridSize;
float gridWidth;
int cellsPerDim;
float* density;
float* pressure;
float3* velocity;

__device__ float smoothing_kernel(float r_sqr, float d)
{
	if (r_sqr < 0 || r_sqr > d*d) return 0;
	float coeff = 315.0f / (64.0f * CUDART_PI * pow(d, 9.0f));
	float dist = d*d - r_sqr;
	return coeff * (dist * dist * dist);
}

__device__ float smoothing_kernel_grad(float r_sqr, float d)
{
	if (r_sqr < 0 || r_sqr > d*d) return 0;
	float coeff = -945.0f / (32.0f * CUDART_PI * pow(d, 9.0f));
	float dist = d*d - r_sqr;
	return coeff * dist * dist;
}

__device__ float smoothing_kernel_laplacian(float r_sqr, float d)
{
	if (r_sqr < 0 || r_sqr > d*d) return 0;
	float coeff = -945.0f / (32.0f * CUDART_PI * pow(d, 9.0f));
	float dist = d*d - r_sqr;
	float weight = 3.0f*d*d - 7.0f*r_sqr;
	return coeff * dist * weight;
}

__device__ float viscosity_kernel_laplacian(float r_len, float d)
{
	float coeff = 45.0f / (CUDART_PI * pow(d, 6.0f));
	float dist = d - r_len;
	return coeff*dist;
}
__device__ float3 pressure_kernel_grad(float3 r, float r_len, float d)
{
	float dist = d - r_len;
	if (r_len == 0) return make_float3(0,0,0);
	float coeff = -45.0f / (CUDART_PI * pow(d, 6.0f));
	float3 grad = make_float3(r.x*(1.0f / r_len)*(dist*dist), r.y*(1.0f / r_len)*(dist*dist), r.z*(1.0f / r_len)*(dist*dist));
	return make_float3(coeff*grad.x, coeff*grad.y, coeff*grad.z);
}

__global__ void setInitialVelocity_kernel(float3* velocity, int numberParticles)
{
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < numberParticles) {
		velocity[i].x = 0.0f;
		velocity[i].y = 0.0f;
		velocity[i].z = 0.0f;
	}
}

__global__ void calculateDensity_kernel(float* density, float* pressure, const float3* position, int* grid, int* particlesInCell, int numberParticles, int cellsPerDim)
{
	int i = blockIdx.x*blockDim.x + threadIdx.x;

	if (i < numberParticles) {
		float x = position[i].x;
		float y = position[i].y;
		float z = position[i].z;

		int cellX = floor(x / CELL_SIZE);
		int cellY = floor(y / CELL_SIZE);
		int cellZ = floor(z / CELL_SIZE);

		float rho = 0;
		for (int j = -1; j <= 1; ++j) {
			for (int k = -1; k <= 1; ++k) {
				for (int l = -1; l <= 1; ++l) {
					int cellXX = cellX + j;
					int cellYY = cellY + k;
					int cellZZ = cellZ + l;
					if (cellXX >= 0 && cellXX < cellsPerDim && 
						cellYY >= 0 && cellYY < cellsPerDim && 
						cellZZ >= 0 && cellZZ < cellsPerDim) {
						int index = cellZZ*cellsPerDim*cellsPerDim*PARTICLES_PER_CELL +
							cellYY*cellsPerDim*PARTICLES_PER_CELL
							+ cellXX*PARTICLES_PER_CELL;
						int cellIndex = cellZZ*cellsPerDim*cellsPerDim + cellYY*cellsPerDim + cellXX;
						for (int particle = 0; particle < particlesInCell[cellIndex]; ++particle) {
							int particleIndex = grid[index + particle];
							if (particleIndex == i) continue;
							float xx = position[particleIndex].x;
							float yy = position[particleIndex].y;
							float zz = position[particleIndex].z;
							float r_sqr = (x - xx)*(x - xx) + (y - yy)*(y - yy) + (z - zz)*(z - zz);
							rho += smoothing_kernel(r_sqr, SMOOTHING_KERNEL_RADIUS);
						}//end of for loop
					}
				}
			}
		}
		density[i] = rho;
		pressure[i] = (pow(rho, 7)-RHO_0)*GAS_CONSTANT;
	}
}

__global__ void integrate_kernel(float3* position, float* density, float* pressure, float3* velocity, int* grid, int* particlesInCell, int numberParticles, int cellsPerDim, float dt)
{
	int i = blockIdx.x*blockDim.x + threadIdx.x;
	if (i < numberParticles) {
		float x = position[i].x;
		float y = position[i].y;
		float z = position[i].z;
		float vx = velocity[i].x;
		float vy = velocity[i].y;
		float vz = velocity[i].z;
		int cellX = floor(x / CELL_SIZE);
		int cellY = floor(y / CELL_SIZE);
		int cellZ = floor(z / CELL_SIZE);
		
		float3 f_pressure;
		f_pressure.x = 0;
		f_pressure.y = 0;
		f_pressure.z = 0;
		float3 f_viscosity;
		f_viscosity.x = 0;
		f_viscosity.y = 0;
		f_viscosity.z = 0;
		float3 f_external;
		f_external.x = 0;
		f_external.y = -9.81;
		f_external.z = 0;
		float curr_p = pressure[i];
		
		for (int j = -1; j <= 1; ++j) {
			for (int k = -1; k <= 1; ++k) {
				for (int l = -1; l <= 1; ++l) {
					int cellXX = cellX + j;
					int cellYY = cellY + k;
					int cellZZ = cellZ + l;
					if (cellXX >= 0 && cellXX < cellsPerDim &&
						cellYY >= 0 && cellYY < cellsPerDim &&
						cellZZ >= 0 && cellZZ < cellsPerDim) {
						int index = cellZZ*cellsPerDim*cellsPerDim*PARTICLES_PER_CELL +
							cellYY*cellsPerDim*PARTICLES_PER_CELL
							+ cellXX*PARTICLES_PER_CELL;
						int cellIndex = cellZZ*cellsPerDim*cellsPerDim + cellYY*cellsPerDim + cellXX;
						for (int particle = 0; particle < particlesInCell[cellIndex]; ++particle) {
							int particleIndex = grid[index + particle];
							if (particleIndex == i) continue;
							float xx = position[particleIndex].x;
							float yy = position[particleIndex].y;
							float zz = position[particleIndex].z;
							//calculate f_pressure = -nabla*p(x_i)
							float3 r = make_float3(xx - x, yy - y, zz - z);
							float distance_sqr = (x - xx)*(x - xx) + (y - yy)*(y - yy) + (z - zz)*(z - zz);
							float p = pressure[particleIndex];
							float rho = density[particleIndex];
							//float kernel = smoothing_kernel_grad(distance_sqr, SMOOTHING_KERNEL_RADIUS);
							float3 grad = pressure_kernel_grad(r, sqrt(distance_sqr), SMOOTHING_KERNEL_RADIUS);
							float coeff = ((p + curr_p) / (2 * rho));
							if (p + curr_p == 0 || rho == 0) coeff = 0;
							f_pressure.x -= coeff * grad.x;
							f_pressure.y -= coeff * grad.y;
							f_pressure.z -= coeff * grad.z;
							//calculate f_viscosity = mu*laplace(v(x_i))
							float v_x = velocity[particleIndex].x;
							float v_y = velocity[particleIndex].y;
							float v_z = velocity[particleIndex].z;
							float laplacian = viscosity_kernel_laplacian(sqrt(distance_sqr), SMOOTHING_KERNEL_RADIUS);
							f_viscosity.x += rho==0 ? 0 : ((v_x - vx) / rho * laplacian);
							f_viscosity.y += rho==0 ? 0 : ((v_y - vy) / rho * laplacian);
							f_viscosity.z += rho==0 ? 0 : ((v_z - vz) / rho * laplacian);
						}//end of for loop
					}
				}
			}
		}

		float mu = MU;
		f_viscosity.x *= mu;
		f_viscosity.y *= mu;
		f_viscosity.z *= mu;
		__syncthreads();
		float3 F;
		F.x = f_pressure.x + f_viscosity.x + f_external.x;
		F.y = f_pressure.y + f_viscosity.y + f_external.y;
		F.z = f_pressure.z + f_viscosity.z + f_external.z;
		velocity[i].x = vx + (dt*F.x);
		velocity[i].y = vy + (dt*F.y);
		velocity[i].z = vz + (dt*F.z);

		/*if (position[i].x + dt*velocity[i].x > UNIFORM_GRID_MAX || position[i].x + dt*velocity[i].x < UNIFORM_GRID_MIN) {
			velocity[i].x = 0;
		}
		if (position[i].y + dt*velocity[i].y > UNIFORM_GRID_MAX || position[i].y + dt*velocity[i].y < UNIFORM_GRID_MIN) {
			velocity[i].y = 0;
		}
		if (position[i].z + dt*velocity[i].z > UNIFORM_GRID_MAX || position[i].z + dt*velocity[i].z < UNIFORM_GRID_MIN) {
			velocity[i].z = 0;
		}*/


		position[i].x = position[i].x + dt*velocity[i].x;
		position[i].y = position[i].y + dt*velocity[i].y;
		position[i].z = position[i].z + dt*velocity[i].z;

		if (position[i].x < UNIFORM_GRID_MIN + EPSILON) {
			velocity[i].x *= -0.5f;;
			position[i].x = UNIFORM_GRID_MIN + EPSILON;
		}
		if (position[i].x > UNIFORM_GRID_MAX - EPSILON) {
			velocity[i].x *= -0.5f;
			position[i].x = UNIFORM_GRID_MAX - EPSILON;
		}
		if (position[i].y < UNIFORM_GRID_MIN + EPSILON) {
			velocity[i].y *= -0.5f;
			position[i].y = UNIFORM_GRID_MIN + EPSILON;
		}
		if (position[i].y > UNIFORM_GRID_MAX - EPSILON) {
			velocity[i].y *= -0.5f;
			position[i].y = UNIFORM_GRID_MAX - EPSILON;
		}
		if (position[i].z < UNIFORM_GRID_MIN + EPSILON) {
			velocity[i].z *= -0.5f;
			position[i].z = UNIFORM_GRID_MIN + EPSILON;
		}
		if (position[i].z > UNIFORM_GRID_MAX - EPSILON) {
			velocity[i].z *= -0.5f;
			position[i].z = UNIFORM_GRID_MAX - EPSILON;
		}
	}
}

__global__ void clearGrid_kernel(int* grid, int* particlesInCell, int gridSize)
{
	int x = blockIdx.x*blockDim.x + threadIdx.x;
	if (x < gridSize) {
		particlesInCell[x] = 0;
		for(int i = 0; i < PARTICLES_PER_CELL; ++i)
			grid[x*PARTICLES_PER_CELL +i] = -1;
	}
}

__global__ void updateGrid_kernel(int* grid, int* particlesInCell, const float3* position, size_t numParticles, size_t cellsPerDim)
{
	int i = blockIdx.x*blockDim.x + threadIdx.x;	//index of particle
	if (i < numParticles) {
		float x = position[i].x;
		float y = position[i].y;
		float z = position[i].z;

		int cellX = floor(x / CELL_SIZE);
		int cellY = floor(y / CELL_SIZE);
		int cellZ = floor(z / CELL_SIZE);
		int index = cellZ*cellsPerDim*cellsPerDim*PARTICLES_PER_CELL +
			cellY*cellsPerDim*PARTICLES_PER_CELL +
			cellX*PARTICLES_PER_CELL;
		int cellIndex = cellZ*cellsPerDim*cellsPerDim + cellY*cellsPerDim + cellX;
		int offset = atomicAdd(&(particlesInCell[cellIndex]), 1);
		if(offset < PARTICLES_PER_CELL)
			grid[index+offset] = i;
		//printf("Thread:%i, Offset:%i\n", i, offset);
	}

}

// just for calculation of blocks in grid
int iDivUp(int x, int y)
{
	return x / y + (x % y != 0);
}

void integrate(GLuint vbo, size_t numParticles, float dt)
{
	if (first) {
		cudaGraphicsGLRegisterBuffer(&vbo_resource, vbo, cudaGraphicsRegisterFlagsNone);
		gridWidth = UNIFORM_GRID_MAX - UNIFORM_GRID_MIN;
		gridSize = (gridWidth/ CELL_SIZE)*(gridWidth / CELL_SIZE)*(gridWidth / CELL_SIZE);
		cellsPerDim = gridWidth / CELL_SIZE;
		cudaMalloc(&uniformGrid, gridSize * sizeof(int)*PARTICLES_PER_CELL);
		cudaMalloc(&particlesInCell, gridSize * sizeof(int));
		cudaMalloc(&density, numParticles * sizeof(float));
		cudaMalloc(&pressure, numParticles * sizeof(float));
		
		cudaMalloc(&velocity, numParticles * sizeof(float3));
		dim3 threads_in_block(THREADS_PER_BLOCK_DIM, 1, 1);
		dim3 blocks_in_grid = dim3(iDivUp(numParticles, threads_in_block.x), 1, 1);
		setInitialVelocity_kernel << <blocks_in_grid, threads_in_block >> > (velocity, numParticles);
		first = false;
	}
	float3* positions;
	cudaGraphicsMapResources(1, &vbo_resource, 0);

	size_t N;	//Number of bytes in the position buffer

	cudaGraphicsResourceGetMappedPointer((void**)&positions, &N, vbo_resource);

	dim3 threads_in_block(min(numParticles,THREADS_PER_BLOCK_DIM), 1, 1);

	dim3 blocks_in_grid(iDivUp(gridSize, threads_in_block.x), 1, 1);
	clearGrid_kernel<<<blocks_in_grid, threads_in_block>>>(uniformGrid, particlesInCell, gridSize);


	blocks_in_grid = dim3(iDivUp(numParticles, threads_in_block.x), 1, 1);

	updateGrid_kernel << <blocks_in_grid, threads_in_block >> > (uniformGrid, particlesInCell, positions, numParticles, cellsPerDim);
	calculateDensity_kernel << <blocks_in_grid, threads_in_block >> >(density, pressure, positions, uniformGrid, particlesInCell, numParticles, cellsPerDim);
	integrate_kernel<<<blocks_in_grid, threads_in_block>>>(positions,density, pressure, velocity, uniformGrid, particlesInCell, numParticles, cellsPerDim, 0.01f);
	cudaGraphicsUnmapResources(1, &vbo_resource, 0);
}

